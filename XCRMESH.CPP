/*******************************************************************
**    File: XCRMesh.cpp
**      By: Paul L. Rowan     
** Created: 970404
**
** Description: Render a single mesh to the viewport
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

/*-------- GLOBAL VARIABLES ---------------------------------------*/
#define CLIP_LEFT	0x00000001
#define CLIP_RIGHT	0x00000002
#define CLIP_TOP	0x00000004
#define CLIP_BOTTOM	0x00000008

//********************************************************************
//	-- RENDER A SINGLE NODE
//********************************************************************
Void RecursiveRenderObject( MeshNodePtr pNode, Int clipflags )
{
	ENTER;

	Int				i, c, side;
	Float			rejectpt[3], acceptpt[3], d;
	LongPtr			pindex;
	MeshFacePtr		pFaces;

	if (pNode==NULL)
		return;

	if (clipflags)
	{
		for (i=0 ; i<4 ; i++)
		{
			if (! (clipflags & (1<<i)) )
				continue;	

			pindex = &gpStack->pindexes[i][0];

			rejectpt[0] = pNode->minmaxs[pindex[0]];
			rejectpt[1] = pNode->minmaxs[pindex[1]];
			rejectpt[2] = pNode->minmaxs[pindex[2]];
			
			d = DotProduct( rejectpt, gpStack->frustum[i].plane );
			d -= gpStack->frustum[i].plane[3];

			if (d <= 0)	// Node is completely offscreen
				return;

			acceptpt[0] = pNode->minmaxs[pindex[3+0]];
			acceptpt[1] = pNode->minmaxs[pindex[3+1]];
			acceptpt[2] = pNode->minmaxs[pindex[3+2]];

			d = DotProduct( acceptpt, gpStack->frustum[i].plane );
			d -= gpStack->frustum[i].plane[3];

			if (d >= 0)
				clipflags &= ~(1<<i);	// Node is entirely in front of plane
		}
	}

	d = DotProduct( pNode->plane, gpStack->campos );
	d -= pNode->plane[3];

	if (d>=0.0f)
		side = 1;
	else
		side = 0;

	RecursiveRenderObject( pNode->pChildren[side], clipflags );

	// Draw node's faces
	pFaces = MeshFacePtr(UCharPtr(pNode)+sizeof(MeshNode));
	c = pNode->nFaces;
	while(c--)
	{
		if (gRenderSurf.bD3D)
			D3DRenderFace( pFaces, clipflags );
		else
			AddFace( pFaces, clipflags );
		pFaces = MeshFacePtr(UCharPtr(pFaces)+sizeof(MeshFace)+sizeof(MeshEdgeIndex)*pFaces->nEdges);
	}

	// Draw node's attached objects
	if (pNode->pAttach!=NULL)
	{
		Long	ID;

		ID = pNode->pAttach->manipID;

		if (gpStack->pObj->obj.pManips[ID].type==XCGEN_MANIP_OBJECT)
		{
			ObjectInstancePtr	pAObj;
			
			pAObj = ObjectInstancePtr(gpStack->pObj->obj.pManips[ID].pObj);

			/*if (!gRenderSurf.bD3D)
			{
				ScanEdges();
				DrawSurfaces();
				ClearEdgeLists();
			}*/

			RenderObject( pAObj );
		}
	}

	RecursiveRenderObject( pNode->pChildren[!side], clipflags );
	
	EXIT( "RecursiveRenderObject" );
}

//********************************************************************
//	-- GENERATE FRUSTUM - build gpStack->frustum to be the
//				transformed viewing frustum in object space
//				initialize the indirection array for quick
//				node accept/reject
//********************************************************************
Void GenerateFrustum( Void )
{
	ENTER;

	Vec_3		temp;
	Int			i, j;
	LongPtr		pindex;

	// Transform camera position into object space
	if (gpStack->pObj->obj.flags & XCGEN_OBJ_BACKGROUND)
	{
		temp[0] = -gpStack->pObj->framepos[0];
		temp[1] = -gpStack->pObj->framepos[1];
		temp[2] = -gpStack->pObj->framepos[2];
	}
	else
	{
		VectorSubtract( temp, gCamera.pos, gpStack->pObj->framepos );
	}
	MatVecMulInv( gpStack->campos, gpStack->pObj->framemat, temp );

	// Transform main viewing frustum into object space
	for (i=0;i<4;i++)
	{
		MatVecMulInv( gpStack->frustum[i].plane, gpStack->pObj->framemat, gFrustum[i] );
		gpStack->frustum[i].plane[3] = DotProduct( gpStack->frustum[i].plane, gpStack->campos );
		if (i==0)
			gpStack->frustum[i].flags = LEFT_CLIP_PLANE;
		else if (i==1)
			gpStack->frustum[i].flags = RIGHT_CLIP_PLANE;
	}

	// Initialize minmax indirection
	pindex = &gpStack->pindexes[0][0];

	for (i=0 ; i<4 ; i++)
	{
		for (j=0 ; j<3 ; j++)
		{
			if (gpStack->frustum[i].plane[j] < 0)
			{
				pindex[j] = j;
				pindex[j+3] = j+3;
			}
			else
			{
				pindex[j] = j+3;
				pindex[j+3] = j;
			}
		}
		pindex += 6;
	}

	EXIT( "GenerateFrustum" );
}

//********************************************************************
//	-- RENDER LINESET gpStack->pLOD
//********************************************************************
Void RenderLineset( Void )
{
	ENTER;

	Int				i;
	MeshEdgePtr		pE;

	pE = &gpStack->pEdges[1];  // Skip dummy edge
	for (i=0;i<gpStack->pLOD->nEdges;i++)
	{
		RenderWorldLine( gpStack->pVerts[pE->v[0]].p, gpStack->pVerts[pE->v[1]].p,
						 pE->r, pE->g, pE->b );
	}

	EXIT( "RenderLineset" );
}

//********************************************************************
//	-- RENDER OBJECT gpObjStack[gnCurObject]
//********************************************************************
Void RenderObject( ObjectInstancePtr pObj )
{
	ENTER;

	Long			lod;
	Float			dist;
	Int				i;
	RenderStackPtr	pOldStack;

	DEBUGAssert( gnCurObject>=0 );

	if ( (gViewport.flags & XCGEN_VPFLAG_NOBACKGROUND) &&
		 (pObj->obj.flags & XCGEN_OBJ_BACKGROUND) )
		 return;

	pOldStack = gpStack;

	gpStack = &gpObjStack[gnCurObject++];
	gpStack->pObj = pObj;
	gpStack->pMesh = pObj->pMesh;

	// Determine distance and LOD
	dist = QuickDist( gCamera.pos, gpStack->pObj->framepos );
	
	// Simple cull
	if (dist>999999.0f)
		return;

	lod = 0;
	for (i=0;i<4;i++)
	{
		if ( gpStack->pMesh->pLOD[i]!=NULL && dist>gpStack->pMesh->pLOD[i]->maxdist)
			lod = i+1;
	}
	//if (gpStack->pMesh->pLOD[1]!=NULL)
	//	lod = max(lod,1);

	DEBUGAssert( lod<4 );

	gpStack->pLOD = gpStack->pMesh->pLOD[lod];

	gpStack->pVerts = gpStack->pLOD->pVerts;
	gpStack->pEdges = gpStack->pLOD->pEdges;

	GenerateFrustum();
	ExtractLights();

	//Reset cache hint indexes in edge lists
	for (i=0;i<gpStack->pLOD->nEdges;i++)
	{
		gpStack->pEdges[i].iCache = -1;
	}

	if (gpStack->pLOD->flags & XCGEN_MESH_LINESET)
		RenderLineset();
	else
		RecursiveRenderObject( gpStack->pLOD->pRoot, 15 );

	// Draw object bounding boxes
	Vec_3	v0, v1;

	if (gViewport.flags & XCGEN_VIEW_BBOXOBJECT)
	{
		VectorCopy( v0, &gpStack->pObj->pMesh->minmaxs[0] );
		VectorCopy( v1, &gpStack->pObj->pMesh->minmaxs[3] );

		DrawBox( v0, v1, 64, 64, 0, TRUE );
		
		// Draw object's axis
		VectorAdd( v0, &gpStack->pObj->pMesh->minmaxs[0], &gpStack->pObj->pMesh->minmaxs[3] );
		VectorScale( v0, v0, 0.5f );
		VectorCopy( v1, v0 );

		Float	scale;

		scale = gpStack->pObj->pMesh->minmaxs[3] - gpStack->pObj->pMesh->minmaxs[0];
		v0[0] += scale*0.5f;
		v1[0] += scale;
		RenderWorldLine( v0, v1, 128, 0, 0 );
		v0[0] -= scale*0.5f;
		v1[0] -= scale;
		scale = gpStack->pObj->pMesh->minmaxs[4] - gpStack->pObj->pMesh->minmaxs[1];
		v0[1] += scale*0.5f;
		v1[1] += scale;
		RenderWorldLine( v0, v1, 0, 128, 0 );
		v0[1] -= scale*0.5f;
		v1[1] -= scale;
		scale = gpStack->pObj->pMesh->minmaxs[5] - gpStack->pObj->pMesh->minmaxs[2];
		v0[2] += scale*0.5f;
		v1[2] += scale;
		RenderWorldLine( v0, v1, 0, 0, 128 );
		v0[2] -= scale*0.5f;
		v1[2] -= scale;
	}

	// Draw object movement boxes
	if (gViewport.flags & XCGEN_VIEW_BBOXMOVE)
	{
		if (gpStack->pObj->obj.flags & (XCGEN_OBJ_SOLIDBBOX+XCGEN_OBJ_SOLIDRADIUS+XCGEN_OBJ_SOLIDHULL) )
		{
			if (gpStack->pObj->obj.flags & XCGEN_OBJ_SOLIDHULL)
			{
				v0[0] = gpStack->pObj->framepos[0] - gpStack->pObj->objradius;
				v0[1] = gpStack->pObj->framepos[1] - gpStack->pObj->objradius;
				v0[2] = gpStack->pObj->framepos[2] - gpStack->pObj->objradius;

				v1[0] = gpStack->pObj->framepos[0] + gpStack->pObj->objradius;
				v1[1] = gpStack->pObj->framepos[1] + gpStack->pObj->objradius;
				v1[2] = gpStack->pObj->framepos[2] + gpStack->pObj->objradius;
			}
			else
			{
				v0[0] = gpStack->pObj->framepos[0] - gpStack->pObj->obj.radius;
				v0[1] = gpStack->pObj->framepos[1] - gpStack->pObj->obj.radius;
				v0[2] = gpStack->pObj->framepos[2] - gpStack->pObj->obj.radius;

				v1[0] = gpStack->pObj->framepos[0] + gpStack->pObj->obj.radius;
				v1[1] = gpStack->pObj->framepos[1] + gpStack->pObj->obj.radius;
				v1[2] = gpStack->pObj->framepos[2] + gpStack->pObj->obj.radius;
			}

			if (gpStack->pObj->region!=0)
			{
				DrawBox( v0, v1, (gpStack->pObj->region&0x01)?128:0, (gpStack->pObj->region&0x02)?128:0, 0, FALSE );
			}
			else
			{
				DrawBox( v0, v1, 0, 32, 32, FALSE );
			}
		}
	}

	gnCurObject--;

	gpStack = pOldStack;

	EXIT( "RenderObject" );
}

