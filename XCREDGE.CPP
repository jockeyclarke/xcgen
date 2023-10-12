/*******************************************************************
**    File: XCREdge.cpp
**      By: Paul L. Rowan     
** Created: 970409
**
** Description:
**
********************************************************************/

/*--- INCLUDES ----------------------------------------------------*/
#include "xcgenint.h"
#include <math.h>

/*--- CONSTANTS ---------------------------------------------------*/


/************************ IMPORT SECTION ***************************/
/*-------- FUNCTION PROTOTYPES ------------------------------------*/

/*-------- GLOBAL VARIABLES ---------------------------------------*/


/************************ PRIVATE SECTION **************************/
/*-------- FUNCTION PROTOTYPES ------------------------------------*/
/*-------- GLOBAL VARIABLES ---------------------------------------*/
#define EDGE_TOPDOWN		0x00000004
#define EDGE_BOTTOMUP		0x00000008

Bool		gbClipped;
Bool		gbRightClipped, gbMakeRight;
MeshVertex	gvRight[2];
Bool		gbLeftClipped, gbMakeLeft;
MeshVertex	gvLeft[2];
Float		gNearZInv;
MeshFacePtr gpCurFace;
SurfPtr		gpCurSurf;
Int			gnGradVerts;
Vec_3		gTP[3], gTT[3];	
Float		gDet;				 

//********************************************************************
//	-- RESET EDGE LISTS
//********************************************************************
Void ClearEdgeLists( Void )
{
	ENTER;

    Int i;

    for (i=0 ; i<gViewport.height ; i++)
    {
        gNewEdges[i].pNext = &gMaxEdge;
        gpRemoveEdges[i] = NULL;
    }

	gnEdges = 0;
	gnVertices = 0;
	gnSurfaces = 1;		// Surface zero is sentinnel background surface

	EXIT( "ClearEdgeLists" );
}

//********************************************************************
//	-- SET UP THE NEW EDGE
//********************************************************************
Bool InitializeEdge( EdgePtr pCEdge )
{
	ENTER;
	// Calculate screen space edge coordinates
	Int			topy, bottomy, height, tempy;
	Float		slope, deltax, deltay;
	FloatPtr	pV0, pV1;

	pV0 = &gXFormVert[pCEdge->v[0]].p[0];
	pV1 = &gXFormVert[pCEdge->v[1]].p[0];

	// Clamp vertices
	if (pV0[0] < -0.5f) pV0[0] = -0.5f;
	if (pV0[1] < -0.5f) pV0[1] = -0.5f;
	if (pV0[0] > Float(gViewport.width)-0.5f ) pV0[0] = Float(gViewport.width)-0.5f;
	if (pV0[1] > Float(gViewport.height)-0.5f ) pV0[1] = Float(gViewport.height)-0.5f;
	if (pV1[0] < -0.5f) pV1[0] = -0.5f;
	if (pV1[1] < -0.5f) pV1[1] = -0.5f;
	if (pV1[0] > Float(gViewport.width)-0.5f ) pV1[0] = Float(gViewport.width)-0.5f;
	if (pV1[1] > Float(gViewport.height)-0.5f ) pV1[1] = Float(gViewport.height)-0.5f;

	topy = Int(ceil(pV0[1]));
	bottomy = Int(ceil(pV1[1]));
	height = bottomy - topy;

	if (height==0)
	{
		return( FALSE );
	}

	pCEdge->flags &= ~(EDGE_BOTTOMUP|EDGE_TOPDOWN);
	if (height<0)
	{
		pCEdge->flags |= EDGE_BOTTOMUP;

		tempy = topy;
		topy = bottomy;
		bottomy = tempy;

		deltay = pV0[1] - pV1[1];
		deltax = pV0[0] - pV1[0];
		slope = deltax / deltay;

		pCEdge->xstep = Int(slope * Float(0x10000));
		pCEdge->x = Int((pV1[0] + (Float(topy) - pV1[1])*slope) * Float(0x10000) ) + 0xffff;

		pCEdge->s = gXFormVert[pCEdge->v[1]].s[0];
		pCEdge->sstep = Int(Float(gXFormVert[pCEdge->v[0]].s[0] - gXFormVert[pCEdge->v[1]].s[0])/deltay);
	}
	else
	{
		pCEdge->flags |= EDGE_TOPDOWN;

		deltay = pV1[1] - pV0[1];
		deltax = pV1[0] - pV0[0];
		slope = deltax / deltay;

		pCEdge->xstep = Int(slope * Float(0x10000));
		pCEdge->x = Int((pV0[0] + (Float(topy) - pV0[1])*slope) * Float(0x10000) ) + 0xffff;

		pCEdge->s = gXFormVert[pCEdge->v[0]].s[0];
		pCEdge->sstep = Int(Float(gXFormVert[pCEdge->v[1]].s[0] - gXFormVert[pCEdge->v[0]].s[0])/deltay);
	}
	
	DEBUGAssert( topy >= 0 );
	DEBUGAssert( bottomy >= 0 );
	DEBUGAssert( topy < gViewport.height );
	DEBUGAssert( bottomy-1 < gViewport.height );
	DEBUGAssert( pCEdge->x >= 0 );
	DEBUGAssert( pCEdge->x + (bottomy-topy-1)*pCEdge->xstep < ((gViewport.width<<16)+0xFFFF) );

	// Fixup, don't know if it's necessary
	//if (pCEdge->x < (1<<19)-1 )
	//	pCEdge->x = (1<<19)-1;
	//if (pCEdge->x > (gViewport.width<<20) + (1<<19) - 1 )
	//	pCEdge->x = (gViewport.width<<20) + (1<<19) - 1;

	// Incorporate edge into edge database
	EdgePtr		pE;
	Int			newx;

	newx = pCEdge->x;
	//if ( pCEdge->flags & EDGE_BOTTOMUP )
	//	newx++;	// sort trailers before leaders

    // Put the edge on the list to be added on top scan
	pE = &gNewEdges[topy];
	while (pE->pNext->x < newx)
		pE = pE->pNext;
	pCEdge->pNext = pE->pNext;
	pE->pNext = pCEdge;

	// Put the edge on the list to be removed after final scan
	pCEdge->pNextRemove = gpRemoveEdges[bottomy - 1];
	gpRemoveEdges[bottomy - 1] = pCEdge;

	// Remove any lingering surface references
	pCEdge->pSurf[0] = NULL;	
	pCEdge->pSurf[1] = NULL;

	return( TRUE );

	EXIT( "InitializeEdge" );
}


Void ExtractLights( Void )
{
	ENTER;

	ObjectInstancePtr	pObj;
	Vec_3				p, v;
	Int					i, which;
	Float				dist, maxdist;

	// Extract all illuminating objects and transform
	// their coordinates into the current objects space
	gpStack->nLights = 0;

	MatVecMulInv( gpStack->XLight, gpStack->pObj->framemat, gLight );

	pObj = gpObjects->pNext;
	while( pObj )
	{
		if (pObj->obj.light>0.0f)
		{
			// Transform light into object's space
			VectorSubtract( v, pObj->framepos, gpStack->pObj->framepos );
			MatVecMulInv( p, gpStack->pObj->framemat, v );

			dist = QuickLength(p);

			if (gpStack->nLights<MAX_DYNAMIC_LIGHTS)
			{
				VectorCopy( gpStack->Lights[gpStack->nLights], p );
				if (pObj->obj.flags & XCGEN_OBJ_SPRITE)
					gpStack->LightRad[gpStack->nLights] = pObj->obj.light * (Float(pObj->obj.tsKill-pObj->obj.tsBegin) - Float(gCamera.tsCurrent-pObj->obj.tsBegin)) / Float(pObj->obj.tsKill-pObj->obj.tsBegin);
				else
					gpStack->LightRad[gpStack->nLights] = pObj->obj.light;
				gpStack->LightDist[gpStack->nLights] = dist;
				for (i=0;i<4;i++)
					gpStack->LightColor[gpStack->nLights][i] = pObj->obj.lcolor[i];
				gpStack->nLights++;
			}
			else
			{
				maxdist = 0.0f;
				for (i=0;i<MAX_DYNAMIC_LIGHTS;i++)
				{
					if (gpStack->LightDist[i]>maxdist)
					{
						which = i;
						maxdist = gpStack->LightDist[i];
					}
				}
				if (maxdist>0.0f)
				{
					VectorCopy( gpStack->Lights[which], p );
				if (pObj->obj.flags & XCGEN_OBJ_SPRITE)
					gpStack->LightRad[which] = pObj->obj.light * (Float(pObj->obj.tsKill-pObj->obj.tsBegin) - Float(gCamera.tsCurrent-pObj->obj.tsBegin)) / Float(pObj->obj.tsKill-pObj->obj.tsBegin);
				else
					gpStack->LightRad[which] = pObj->obj.light;
					gpStack->LightDist[which] = dist;
				for (i=0;i<4;i++)
					gpStack->LightColor[which][i] = pObj->obj.lcolor[i];
				}
			}
		}

		pObj = pObj->pNext;
	}
	

	EXIT( "ExtractLights" );
}

Void GetVertexShade( Vec_3 norm, Vec_3 light, Float ldist, Bool selfIllum, FloatPtr pDiff )
{
	ENTER;

	Float	d, diff;

	d = DotProduct( norm, light );
		
	if (d>0.0f)
	{
		if ( selfIllum )
			diff = 1.0f;
		else
			diff = d;
	}
	else
	{
		if ( selfIllum )
			diff = 1.0f;
		else
			diff = 0.0f;
	}

	if ( selfIllum )
	{
		*pDiff += diff;
	}
	else
	{
		*pDiff += diff*ldist;
	}

	*pDiff = max(min(*pDiff,1.0f),0.0f);

	return;

	EXIT( "GetVertexShade" );
}

Void LightVertices( EdgePtr pEdge, Vec_3 pP0, Vec_3 pP1 )
{
	ENTER;

	Int					i, j;
	XFormedVertexPtr	pVert;
	Bool				selfIllum;
	Float				diff, d;
	Vec_3				v;


	selfIllum = (gpCurSurf->pObj->obj.flags & XCGEN_OBJ_SELFILLUM);// ||
				//(gpCurSurf->pFace->flags & XCGEN_FACE_SELFILLUM);   

	for (i=0;i<2;i++)
	{
		pVert = &gXFormVert[pEdge->v[i]];
		diff = 0.0f;

		if (gpStack->pObj->obj.flags & XCGEN_OBJ_BACKGROUND)
		{
			diff = 1.0f;
		}
		else
		{
			GetVertexShade( pVert->n, gpStack->XLight, 0.7f, selfIllum, &diff );
			
			for (j=0;j<gpStack->nLights;j++)
			{
				if (i==0)
					VectorSubtract( v, gpStack->Lights[j], pP0 );
				else
					VectorSubtract( v, gpStack->Lights[j], pP1 );
				
				NormalizeVector( v, &d );

				if (gpStack->LightRad[j]>d)
				{
					GetVertexShade( pVert->n, v, (gpStack->LightRad[j]-d)/gpStack->LightRad[j], selfIllum, &diff );
				}
			}
		}

		diff = max(gpStack->pObj->obj.ambient,diff);
		diff = max(gpCurSurf->pFace->emit,diff);

		pVert->s[0] = Int((16.0f-(diff*16.0f))*0x10000)+0x8000;
		pVert->s[0] = max(min(pVert->s[0],0xF7FFF),0x8000);
	}

	EXIT( "LightVertices" );
}

Void AddGradientVerts( EdgePtr pEdge )
{
	XFormedVertexPtr	pXV;
	Int					i, j;

	if (gpCurSurf->pTex/* && gnGradVerts<3*/)
	{
		i = 0;
		while( i<2 /*&& gnGradVerts<3*/)
		{
			pXV = &gXFormVert[pEdge->v[i++]];
			j = gnGradVerts-1;
			while( j>=0 )
			{
				if ( (fabs(pXV->p[0]-gTP[j][0])<0.1f &&
					  fabs(pXV->p[1]-gTP[j][1])<0.1f)  )
				{
					break;			
				}
				j--;
			}
			if (j==-1)
			{
				if (gnGradVerts>=2)
				{
					// Make sure vertices are not colinear
					// Choose the third coordinate to yield the greatest
					//	spread on the plane.  (Largest |determinant| )
					Float	det;

					det =  Float(fabs(((gTP[1][0] - pXV->p[0]) *
							(gTP[0][1] - pXV->p[1])) -
							((gTP[0][0] - pXV->p[0]) *
							(gTP[1][1] - pXV->p[1]))));
					
					if (det<=gDet)
						continue;
					gDet = det;
				}
				if (gnGradVerts<=2)
					gnGradVerts++;
				VectorCopy( gTP[gnGradVerts-1], pXV->p );
				gTT[gnGradVerts-1][0] = pXV->t[0];
				gTT[gnGradVerts-1][1] = pXV->t[1];
			}
		}
	}
}

//********************************************************************
//	-- CLIP AND ADD AN EDGE TO THE BUFFER
//		Returns	either:		index to newly cached edge
//							-2  denotes fully clipped
//							-3	denotes horizontal
//********************************************************************
Int ClipEdge( MeshVertexPtr pV0, MeshVertexPtr pV1, ClipPlanePtr pPlane, Int dir )
{
	ENTER;

	MeshVertex		clipVert;
	Int				index;
	EdgePtr			pCEdge;
	Float			d0, d1, d;

	if (pPlane!=NULL)
	{
		do
		{
			d0 = DotProduct( pV0->p, pPlane->plane ) - pPlane->plane[3];
			d1 = DotProduct( pV1->p, pPlane->plane ) - pPlane->plane[3];

			if (d0>=0.0f)
			{
				// Point 0 is valid
				if (d1>=0.0f)
					continue;			// Both verts are valid

				d = d0 / (d0 - d1);
				clipVert.p[0] = pV0->p[0] + d * (pV1->p[0] - pV0->p[0]);
				clipVert.p[1] = pV0->p[1] + d * (pV1->p[1] - pV0->p[1]);
				clipVert.p[2] = pV0->p[2] + d * (pV1->p[2] - pV0->p[2]);

				clipVert.n[0] = pV0->n[0] + d * (pV1->n[0] - pV0->n[0]);
				clipVert.n[1] = pV0->n[1] + d * (pV1->n[1] - pV0->n[1]);
				clipVert.n[2] = pV0->n[2] + d * (pV1->n[2] - pV0->n[2]);

				clipVert.t[0] = pV0->t[0] + d * (pV1->t[0] - pV0->t[0]);
				clipVert.t[1] = pV0->t[1] + d * (pV1->t[1] - pV0->t[1]);
				//DEBUGAssert( fabs(DotProduct(clipVert.p,pPlane->plane) - pPlane->plane[3])<0.0001f );

				if ( pPlane->flags & LEFT_CLIP_PLANE ) // Left frustum plane
				{
					gbLeftClipped = TRUE;
					gvLeft[!dir] = clipVert;
				}
				else if ( pPlane->flags & RIGHT_CLIP_PLANE ) // Right frustum plane
				{
					gbRightClipped = TRUE;
					gvRight[!dir] = clipVert;
				}

				gbClipped = TRUE;

				return( ClipEdge( pV0, &clipVert, pPlane->pNext, dir ) );
			}
			else
			{
				// Point 0 is invalid
				if (d1<0.0f)
					return( -2 );		// Tag edge as fully clipped

				d = d1 / (d1 - d0);
				clipVert.p[0] = pV1->p[0] + d * (pV0->p[0] - pV1->p[0]);
				clipVert.p[1] = pV1->p[1] + d * (pV0->p[1] - pV1->p[1]);
				clipVert.p[2] = pV1->p[2] + d * (pV0->p[2] - pV1->p[2]);

				clipVert.n[0] = pV1->n[0] + d * (pV0->n[0] - pV1->n[0]);
				clipVert.n[1] = pV1->n[1] + d * (pV0->n[1] - pV1->n[1]);
				clipVert.n[2] = pV1->n[2] + d * (pV0->n[2] - pV1->n[2]);

				clipVert.t[0] = pV1->t[0] + d * (pV0->t[0] - pV1->t[0]);
				clipVert.t[1] = pV1->t[1] + d * (pV0->t[1] - pV1->t[1]);
				//DEBUGAssert( fabs(DotProduct(clipVert.p,pPlane->plane) - pPlane->plane[3])<0.0001f );

				if ( pPlane->flags & LEFT_CLIP_PLANE ) // Left frustum plane
				{
					gbLeftClipped = TRUE;
					gvLeft[dir] = clipVert;
				}
				else if ( pPlane->flags & RIGHT_CLIP_PLANE ) // Right frustum plane
				{
					gbRightClipped = TRUE;
					gvRight[dir] = clipVert;
				}

				gbClipped = TRUE;

				return( ClipEdge( &clipVert, pV1, pPlane->pNext, dir ) );
			}
		} while ( (pPlane=pPlane->pNext) );  // This is an assignment
	}

	index = gnEdges;
	pCEdge = &gEdges[gnEdges++];

	if (gnVertices>MAX_TOTAL_VERTICES-2)
		FATALError( "Ran out of vertices" );

	// Transform vertex and normal and add to list
	TransformProjectVector( gXFormVert[gnVertices].p, pV0->p );
	//TransformWorldVector( gXFormVert[gnVertices].n, pV0->n );
	VectorCopy( gXFormVert[gnVertices].n, pV0->n ); 
	gXFormVert[gnVertices].t[0] = pV0->t[0];
	gXFormVert[gnVertices].t[1] = pV0->t[1];
	pCEdge->v[0] = gnVertices++;

	// Transform vertex and normal and add to list
	TransformProjectVector( gXFormVert[gnVertices].p, pV1->p );
	//TransformWorldVector( gXFormVert[gnVertices].n, pV1->n );
	VectorCopy( gXFormVert[gnVertices].n, pV1->n ); 
	gXFormVert[gnVertices].t[0] = pV1->t[0];
	gXFormVert[gnVertices].t[1] = pV1->t[1];
	pCEdge->v[1] = gnVertices++;

	LightVertices( pCEdge, pV0->p, pV1->p );

	if (!InitializeEdge( pCEdge ))
	{
		// Tag edge as fully clipped (horizontal)
		gnEdges--;		// Return space to buffer
		gnVertices-=2;
		return( -3 );
	}

	return( index );

	EXIT( "ClipEdge" );
}

//********************************************************************
//	-- ADD A FACE TO THE BUFFER
//********************************************************************
Void AddFace( MeshFacePtr pFace, Int clipflags )
{
	ENTER;

	Vec_3				v, norm;
	Int					i, edge, mask, dir;
	Float				d, distinv; //, vscale, dfact;
	SurfPtr				pSurf;
	MeshEdgeIndexPtr	pEI;
	MeshEdgePtr			pEdge;
	EdgePtr				pCEdge;
	ClipPlanePtr		pClip;

	// Test facing and cull
	d = DotProduct( pFace->plane, gpStack->campos ) - pFace->plane[3];

	if (d<0.01f)
		return;

	// Create a new surface and assign it to this edge
	if ( gnSurfaces>=MAX_SURFS )
		FATALError( "Ran out of surfaces" );		// Not enough surfaces

	if ( gnEdges+pFace->nEdges>=MAX_EDGES )
		FATALError( "Ran out of edges" );		// Not enough edges

	// Setup this surface for gradient calculation
	pSurf = &gSurfs[gnSurfaces++];
	pSurf->pFace = pFace;

	if (pFace->flags&XCGEN_FACE_TEXTURE)
	{
		XCGENTexturePtr pT;

		pT = gpStack->pObj->obj.pManips[pFace->flags&0xff].pTexture;

		if (pT)
			pSurf->pTex = pT;
		else
			pSurf->pTex = pFace->pTexture;
	}
	else
	{
		pSurf->pTex = pFace->pTexture;
	}

	pSurf->pObj = gpStack->pObj;
	pSurf->pSpans = NULL;
	pSurf->state = 0;

	MatVecMul( v, gpStack->pObj->framemat, pSurf->pFace->plane );
	MatVecMul( norm, gCamera.mat, v );

	distinv = 1.0f / (pSurf->pFace->plane[3] - DotProduct( gpStack->campos, pSurf->pFace->plane ));

	pSurf->zinvstepx = norm[0] * gViewport.maxscaleinv * distinv;
	pSurf->zinvstepy = -norm[1] * gViewport.maxscaleinv * distinv;
	pSurf->zinv00 = norm[2] * distinv -
					gViewport.xcenter * pSurf->zinvstepx -
					gViewport.ycenter * pSurf->zinvstepy;

	gpCurSurf = pSurf;
	gpCurFace = pFace;

	pEI = MeshEdgeIndexPtr(UCharPtr(pFace) + sizeof(MeshFace) );

	// Set up clip plane pointers
	pClip = NULL;
	for ( i=3,mask=0x08; i>=0; i--,mask>>=1 )
	{
		if ( clipflags & mask )
		{
			gpStack->frustum[i].pNext = pClip;
			pClip = &gpStack->frustum[i];
		}
	}

	gbMakeRight = gbMakeLeft = FALSE;
	gNearZInv = 0.0f;

	gnGradVerts = 0;
	gDet = 0.0f;

	for (i=0;i<pFace->nEdges;i++)
	{
		// Negative edge indexes denote reverse use of edge
		if (pEI[i]<0)
		{
			edge = -pEI[i];
			dir = 1;
		}
		else
		{
			edge = pEI[i];
			dir = 0;
		}
		pEdge = &gpStack->pEdges[edge];

		if (pEdge->iCache<-1)	// If this edge has been fully clipped before
			continue;

		// See if this edge is new and clip it if so
		if (pEdge->iCache==-1)
		{
			gbRightClipped = gbLeftClipped = gbClipped = FALSE;
			pEdge->iCache = ClipEdge( &gpStack->pVerts[pEdge->v[0]], &gpStack->pVerts[pEdge->v[1]], pClip, dir );

			// If we passed over a left or right plane,
			//	mark a reminder to create an edge to fill the
			//	hole.  The vertexes will be recorded in gRightEnter and gRightExit, etc.
			if (gbRightClipped)
				gbMakeRight = TRUE;
			if (gbLeftClipped)
				gbMakeLeft = TRUE;

			// Check if edge is completely out of view
			if (pEdge->iCache<-1)
			{
				// Remove cache hint if we've clipped this edge
				//  and need it to be reprocessed.  PLR - Horizontal
				//	edges were not being reprocessed to create the 
				//	left and right end points
				if (gbClipped)
					pEdge->iCache=-1;
				continue;				
			}
		}

		pCEdge = &gEdges[pEdge->iCache];

		// Get nearest zInv for mip mapping
		Float	zinv;
		zinv	 = gpCurSurf->zinv00 + gXFormVert[pCEdge->v[0]].p[0]*gpCurSurf->zinvstepx  + gXFormVert[pCEdge->v[0]].p[1]*gpCurSurf->zinvstepy;
		if (zinv>gNearZInv)
			gNearZInv = zinv;

		zinv	 = gpCurSurf->zinv00 + gXFormVert[pCEdge->v[1]].p[0]*gpCurSurf->zinvstepx  + gXFormVert[pCEdge->v[1]].p[1]*gpCurSurf->zinvstepy;
		if (zinv>gNearZInv)
			gNearZInv = zinv;

		AddGradientVerts( pCEdge );

		// Remove cache hint for clipped edges
		//	We need to re-process clipped edges in order to correctly
		//	Generate the left and right closure edges
		if (gbClipped)
			pEdge->iCache = -1;

		if ( (pCEdge->flags & EDGE_BOTTOMUP) )
			pCEdge->pSurf[dir] = pSurf;
		else
			pCEdge->pSurf[!dir] = pSurf;
	}

	// Check and see if we need to close the polygon by adding
	//	an edge on either side along the frustum
	if (gbMakeLeft)
	{			
												// Omit left plane
		i = ClipEdge( &gvLeft[1], &gvLeft[0], gpStack->frustum[0].pNext, 0 );

		if (i>-1)
		{
			Float	zinv;
			zinv	 = gpCurSurf->zinv00 + gXFormVert[gEdges[i].v[0]].p[0]*gpCurSurf->zinvstepx  + gXFormVert[gEdges[i].v[0]].p[1]*gpCurSurf->zinvstepy;
			if (zinv>gNearZInv)
				gNearZInv = zinv;

			zinv	 = gpCurSurf->zinv00 + gXFormVert[gEdges[i].v[1]].p[0]*gpCurSurf->zinvstepx  + gXFormVert[gEdges[i].v[1]].p[1]*gpCurSurf->zinvstepy;
			if (zinv>gNearZInv)
				gNearZInv = zinv;

			gEdges[i].pSurf[0] = pSurf;
			AddGradientVerts( &gEdges[i] );
		}
	}

	if (gbMakeRight)
	{	
												// Omit left and right planes
		i = ClipEdge( &gvRight[1], &gvRight[0], gpStack->frustum[1].pNext, 0 );

		if (i>-1)
		{
			Float	zinv;
			zinv	 = gpCurSurf->zinv00 + gXFormVert[gEdges[i].v[0]].p[0]*gpCurSurf->zinvstepx  + gXFormVert[gEdges[i].v[0]].p[1]*gpCurSurf->zinvstepy;
			if (zinv>gNearZInv)
				gNearZInv = zinv;

			zinv	 = gpCurSurf->zinv00 + gXFormVert[gEdges[i].v[1]].p[0]*gpCurSurf->zinvstepx  + gXFormVert[gEdges[i].v[1]].p[1]*gpCurSurf->zinvstepy;
			if (zinv>gNearZInv)
				gNearZInv = zinv;

			gEdges[i].pSurf[1] = pSurf;
			AddGradientVerts( &gEdges[i] );
		}
	} 

	/*vscale = max(gViewport.width,gViewport.height)/gViewport.fov;
	dfact = vscale*min(pFace->uaxis[4],pFace->vaxis[4])*gNearZInv;*/

	Float	tsize;

	tsize = gNearZInv*pFace->texelscale;

	if (tsize<0.00025f)
		pSurf->miplevel = 3;
	else if (tsize<0.0005f)
		pSurf->miplevel = 2;
	else if (tsize<0.001f)
		pSurf->miplevel = 1;
	else
		pSurf->miplevel = 0;

	//pSurf->miplevel = 0;

	// Prepare to calculate gradients
	if (pSurf->pTex)
	{
		for (i=0;i<gnGradVerts;i++)
		{
			gTT[i][0] *= ( 1 << (pSurf->pTex->width - pSurf->miplevel) );
			gTT[i][1] *= ( 1 << (pSurf->pTex->height - pSurf->miplevel) );
		}

		CalculateGradients( pSurf, gTP, gTT );
	}

	gStats.nPolygons++;

	EXIT( "AddFace" );
}


