/*******************************************************************
**    File: XCD3DRender.cpp
**      By: Paul L. Rowan     
** Created: 970527
**
** Description:
**
********************************************************************/

/*--- INCLUDES ----------------------------------------------------*/
#include "xcgenint.h"

/*--- CONSTANTS ---------------------------------------------------*/


/************************ IMPORT SECTION ***************************/
/*-------- FUNCTION PROTOTYPES ------------------------------------*/

/*-------- GLOBAL VARIABLES ---------------------------------------*/


/************************ PRIVATE SECTION **************************/
/*-------- FUNCTION PROTOTYPES ------------------------------------*/
#define			AMBIENT			0.2f
#define			IAMBIENT		UChar(AMBIENT*255.0f)

#define			BUFFERPOLYS		TRUE
/*-------- GLOBAL VARIABLES ---------------------------------------*/
MeshFacePtr		gpFace;
Int				gVertSet, gnVerts;
XFormedVertex	gVerts[2][MAX_POLYGON_VERTS];
Vec_3			gWldVerts[MAX_POLYGON_VERTS];
D3DTLVERTEX		gTLVerts[MAX_POLYGON_VERTS];

/************************ MEMBER FUNCTIONS *************************/
Void RenderPolygon( Void )
{
	Int			i, alpha;
	Float		d, s;
	//HRESULT		res;
	UCharPtr	pC;

	alpha = min(Int(gpStack->pObj->obj.alpha*255.0f),255);

	for (i=0;i<gnVerts;i++)
	{
		if (gVerts[gVertSet][i].p[2]>32767.0f)
			return;
		gTLVerts[i].sx = min(max(gVerts[gVertSet][i].p[0],0.0f),gViewport.width-1)+gViewport.xoff;
		gTLVerts[i].sy = min(max(gVerts[gVertSet][i].p[1],0.0f),gViewport.height-1)+gViewport.yoff;
		//gTLVerts[i].rhw = 1.0f/Float(Z_BUFFER_MAX);//gVerts[gVertSet][i].p[2];
		//gTLVerts[i].sz = gVerts[gVertSet][i].p[2]/Float(Z_BUFFER_MAX);//1.0f - min(max(gTLVerts[i].rhw,0.0f),1.0f);
		gTLVerts[i].rhw = 1.0f/gVerts[gVertSet][i].p[2];
		gTLVerts[i].sz = 1.0f - min(max(gTLVerts[i].rhw,0.0f),1.0f);
		d = max(gVerts[gVertSet][i].n[0],AMBIENT);
		s = gVerts[gVertSet][i].n[1];

		pC = &gVerts[gVertSet][i].c[0];

		if (gSurfs[0].pTex!=NULL)
		{
			gTLVerts[i].color = (alpha<<24) + (pC[0]<<16) + (pC[1]<<8) + pC[2];
		}
		else 
			gTLVerts[i].color = (alpha<<24) + (pC[0]<<16) + (pC[1]<<8) + pC[2];//D3DRGBA( pC[0]*Float(gpFace->r)/255, pC[1]*Float(gpFace->g)/255, pC[2]*Float(gpFace->b)/255, 0.5f );
		//gTLVerts[i].specular = (255-Int(min((gVerts[gVertSet][i].p[2]/2000)*255,255)))<<24;//D3DRGBA( 0.0f, 0.0f, 0.0f, 0.5f );

		gTLVerts[i].tu = gVerts[gVertSet][i].t[0];
		gTLVerts[i].tv = gVerts[gVertSet][i].t[1];
	}

#if BUFFERPOLYS
	BufferD3DPoly( gSurfs[0].pTex, gTLVerts, gnVerts );
#else
	HRESULT		res;
	if (gSurfs[0].pTex)
		res = gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_TEXTUREHANDLE, GetD3DTexture(gSurfs[0].pTex) );
	else
		res = gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_TEXTUREHANDLE, NULL );
	res = gRenderSurf.pD3DDev2->DrawPrimitive( D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, gTLVerts, gnVerts,  D3DDP_DONOTCLIP );
	if (res!=D3D_OK)
	{
		Char	errt[64];

		sprintf( errt, "Error: DrawPrimitive failed -- %d", res );
		FATALError( errt );
	}
#endif
}

Int ClipPolygon( ClipPlanePtr pClip )
{
	ENTER;

    Int					i, j, nextvert, curin, nextin;
    Float				curdot, nextdot, scale;
    XFormedVertexPtr	pInVert, pOutVert;

	while( pClip )
	{
		pInVert = gVerts[gVertSet];
		pOutVert = gVerts[!gVertSet];

		curdot = DotProduct( pInVert->p, pClip->plane );
		curin = (curdot >= pClip->plane[3]);

		for (i=0 ; i<gnVerts ; i++)
		{
			nextvert = (i + 1) % gnVerts;

			// Keep the current vertex if it's inside the plane
			if (curin)
				*pOutVert++ = *pInVert;

			nextdot = DotProduct( gVerts[gVertSet][nextvert].p, pClip->plane );
			nextin = (nextdot >= pClip->plane[3]);

			// Add a clipped vertex if one end of the current edge is
			// inside the plane and the other is outside
			if (curin != nextin)
			{
				scale = (pClip->plane[3] - curdot) /
						(nextdot - curdot);

				for (j=0;j<3;j++)
				{
					pOutVert->p[j] = pInVert->p[j] +
							((gVerts[gVertSet][nextvert].p[j] - pInVert->p[j]) *
							 scale);

					pOutVert->n[j] = pInVert->n[j] +
							((gVerts[gVertSet][nextvert].n[j] - pInVert->n[j]) *
							 scale);
				}
				pOutVert->t[0] = pInVert->t[0] +
						((gVerts[gVertSet][nextvert].t[0] - pInVert->t[0]) *
						 scale);
				pOutVert->t[1] = pInVert->t[1] +
						((gVerts[gVertSet][nextvert].t[1] - pInVert->t[1]) *
						 scale);

				pOutVert++;
			}

			curdot = nextdot;
			curin = nextin;
			pInVert++;
		}
		gnVerts = pOutVert - gVerts[!gVertSet];
		if (gnVerts < 3)
			return 0;

		gVertSet = !gVertSet;

		pClip = pClip->pNext;
	}

    return 1;

	EXIT( "ClipToPlane" );
}

Void LightVertex( XFormedVertexPtr pVert, Vec_3 pos )
{
	ENTER;

	Int					j;
	Bool				selfIllum;
	Float				spec, diff, d;
	Int					br;
	Int					r, g, b;
	Vec_3				v;


	selfIllum = (gpStack->pObj->obj.flags & XCGEN_OBJ_SELFILLUM);// ||
				//(gpFace->flags & XCGEN_FACE_SELFILLUM);   

	diff = 0.0f;
	spec = 0.0f;
	r = g = b = max(IAMBIENT,Int(gpStack->pObj->obj.ambient*255.0f)) + Int(gpFace->emit*255.0f);

	if (gpStack->pObj->obj.flags & XCGEN_OBJ_BACKGROUND)
	{
		diff = 1.0f;
		spec = 0.0f;
		r = g = b = 255;	
	}
	else
	{
		GetVertexShade( pVert->n, gpStack->XLight, 0.7f, selfIllum, &diff );
		br = Int(diff * 255.0f);
		r += ((br * gLR)>>8);	// Star light
		g += ((br * gLG)>>8);
		b += ((br * gLB)>>8);
		diff = 0.0f;
		
		for (j=0;j<gpStack->nLights;j++)
		{
			VectorSubtract( v, gpStack->Lights[j], pos );
			
			d = QuickLength(v);
			VectorScale( v, v, 1.0f/d );

			if (gpStack->LightRad[j]>d)
			{
				GetVertexShade( pVert->n, v, (gpStack->LightRad[j]-d)/gpStack->LightRad[j], selfIllum, &diff );
				br = Int(diff * 255.0f);
				r += ((br * Int(gpStack->LightColor[j][0]))>>8);	// Dynamic
				g += ((br * Int(gpStack->LightColor[j][1]))>>8);	// Dynamic
				b += ((br * Int(gpStack->LightColor[j][2]))>>8);	// Dynamic
				diff = 0;
			}
		}
	}

	diff = max(gpStack->pObj->obj.ambient,diff);
	diff = max(gpFace->emit,diff);

	if (r>255)	r = 255;
	if (g>255)	g = 255;
	if (b>255)	b = 255;

	pVert->c[0] = r;
	pVert->c[1] = g;
	pVert->c[2] = b;
	//pVert->n[0] = diff;
	//pVert->n[1] = 0.0f;   // Used to be spectral


	EXIT( "LightVertex" );
}


Void D3DRenderFace( MeshFacePtr pFace, Int clipflags )
{
	ENTER;

	Vec_3				v, norm;
	Int					i, edge, mask, dir;
	Float				d, distinv;
	SurfPtr				pSurf;
	MeshEdgeIndexPtr	pEI;
	MeshEdgePtr			pEdge;
	ClipPlanePtr		pClip;

	gpFace = pFace;

	// Test facing and cull
	d = DotProduct( pFace->plane, gpStack->campos ) - pFace->plane[3];

	if (d<0.01f)
		return;

	// Setup this surface for gradient calculation
	pSurf = &gSurfs[0];
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

	pEI = MeshEdgeIndexPtr(UCharPtr(pFace) + sizeof(MeshFace) );

	// Copy vertices
	gnVerts = 0;
	for (i=0;i<pFace->nEdges;i++)
	{
		// Negative edge indexes denote reverse use of edge
		if (pEI[i]<0)
		{
			dir = 1;
			edge = -pEI[i];
		}
		else
		{
			dir = 0;
			edge = pEI[i];
		}

		pEdge = &gpStack->pEdges[edge];
		
		// Use only the first vertex of every edge
		// Since they are sequential in the object file, this should
		//	yield the vertex set.
		VectorCopy( gVerts[gVertSet][gnVerts].p, gpStack->pVerts[pEdge->v[dir]].p );
		VectorCopy( gVerts[gVertSet][gnVerts].n, gpStack->pVerts[pEdge->v[dir]].n );
		gVerts[gVertSet][gnVerts].t[0] = gpStack->pVerts[pEdge->v[dir]].t[0];
		gVerts[gVertSet][gnVerts].t[1] = gpStack->pVerts[pEdge->v[dir]].t[1];
		gnVerts++;
	}

	if (ClipPolygon( pClip ))
	{
		//CalculateGradients( pSurf );

		// Transform and Light Vertices
		for (i=0;i<gnVerts;i++)
		{
			TransformProjectVector( gVerts[!gVertSet][i].p, gVerts[gVertSet][i].p );
			//TransformWorldVector( gVerts[!gVertSet][i].n, gVerts[gVertSet][i].n );
			VectorCopy( gVerts[!gVertSet][i].n, gVerts[gVertSet][i].n );

			gVerts[!gVertSet][i].t[0] = gVerts[gVertSet][i].t[0];
			gVerts[!gVertSet][i].t[1] = gVerts[gVertSet][i].t[1];

			LightVertex( &gVerts[!gVertSet][i], gVerts[gVertSet][i].p );
		}
		gVertSet = !gVertSet;

		RenderPolygon();

		gStats.nPolygons++;
	}

	EXIT( "D3DRenderFace" );
}
