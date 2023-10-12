/*******************************************************************
**    File: XCBeam.cpp
**      By: Paul L. Rowan      
** Created: 971122
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

/*-------- GLOBAL VARIABLES ---------------------------------------*/
// Faux beam object

typedef struct 
{
	MeshVertex		verts[7];
	MeshEdge		edges[8];
	struct
	{
		MeshFace		face;
		MeshEdgeIndex	edge[4];
	} faces[2];
} BeamFace;

typedef struct
{
	MeshVertex		verts[7];
	MeshEdge		edges[13];
	struct
	{
		MeshFace		face;
		MeshEdgeIndex	edge[3];
	} faces[6];
} CapFace;

ObjectInstance	gBeamObj;	// A cheap shot, but effective
BeamFace		gBeamFace;
CapFace			gCapFace;

/************************ MEMBER FUNCTIONS *************************/
Void InitBeamObject( Void )
{
	ENTER;

	Int		i, j;

	gBeamObj.obj.ambient = 1.0f;
	
	// SIMPLE BEAM FACE
	for (i=0;i<7;i++)
		gBeamFace.verts[i].n[0] = 1.0f;

	gBeamFace.edges[0].v[0] = 0;
	gBeamFace.edges[0].v[1] = 1;
	gBeamFace.edges[1].v[0] = 1;
	gBeamFace.edges[1].v[1] = 2;
	gBeamFace.edges[2].v[0] = 3;
	gBeamFace.edges[2].v[1] = 4;
	gBeamFace.edges[3].v[0] = 4;
	gBeamFace.edges[3].v[1] = 5;
	gBeamFace.edges[4].v[0] = 5;
	gBeamFace.edges[4].v[1] = 6;
	gBeamFace.edges[5].v[0] = 6;
	gBeamFace.edges[5].v[1] = 0;
	gBeamFace.edges[6].v[0] = 2;
	gBeamFace.edges[6].v[1] = 6;
	gBeamFace.edges[7].v[0] = 6;
	gBeamFace.edges[7].v[1] = 3;

	for (i=0;i<2;i++)
	{
		gBeamFace.faces[i].face.nEdges = 4;
		gBeamFace.faces[i].face.plane[0] = 1.0f;
		gBeamFace.faces[i].face.texelscale = 1.0f;
	}

	gBeamFace.faces[0].edge[0] = 0;
	gBeamFace.faces[0].edge[1] = 1;
	gBeamFace.faces[0].edge[2] = 6;
	gBeamFace.faces[0].edge[3] = 5;
	gBeamFace.faces[1].edge[0] = 7;
	gBeamFace.faces[1].edge[1] = 2;
	gBeamFace.faces[1].edge[2] = 3;
	gBeamFace.faces[1].edge[3] = 4;

	// CAP FACE
	// Dummy edge zero
	gCapFace.edges[1].v[0] = 0;
	gCapFace.edges[1].v[1] = 1;
	gCapFace.edges[2].v[0] = 1;
	gCapFace.edges[2].v[1] = 2;
	gCapFace.edges[3].v[0] = 2;
	gCapFace.edges[3].v[1] = 3;
	gCapFace.edges[4].v[0] = 3;
	gCapFace.edges[4].v[1] = 4;
	gCapFace.edges[5].v[0] = 4;
	gCapFace.edges[5].v[1] = 5;
	gCapFace.edges[6].v[0] = 5;
	gCapFace.edges[6].v[1] = 0;
	gCapFace.edges[7].v[0] = 0;
	gCapFace.edges[7].v[1] = 6;
	gCapFace.edges[8].v[0] = 1;
	gCapFace.edges[8].v[1] = 6;
	gCapFace.edges[9].v[0] = 2;
	gCapFace.edges[9].v[1] = 6;
	gCapFace.edges[10].v[0] = 3;
	gCapFace.edges[10].v[1] = 6;
	gCapFace.edges[11].v[0] = 4;
	gCapFace.edges[11].v[1] = 6;
	gCapFace.edges[12].v[0] = 5;
	gCapFace.edges[12].v[1] = 6;

	gCapFace.verts[0].t[0] = 1.0f;
	gCapFace.verts[2].t[0] = 1.0f;
	gCapFace.verts[4].t[0] = 1.0f;
	gCapFace.verts[6].t[0] = 0.5f;
	gCapFace.verts[6].t[1] = 0.5f;
	
	for (i=0;i<6;i++)
	{
		gCapFace.faces[i].face.nEdges = 3;
		gCapFace.faces[i].face.plane[2] = -1.0f;
		gCapFace.faces[i].face.texelscale = 1.0f;
	}

	gCapFace.faces[0].edge[0] = -1;
	gCapFace.faces[0].edge[1] = 7;
	gCapFace.faces[0].edge[2] = -8;

	gCapFace.faces[1].edge[0] = -2;
	gCapFace.faces[1].edge[1] = 8;
	gCapFace.faces[1].edge[2] = -9;

	gCapFace.faces[2].edge[0] = -3;
	gCapFace.faces[2].edge[1] = 9;
	gCapFace.faces[2].edge[2] = -10;

	gCapFace.faces[3].edge[0] = -4;
	gCapFace.faces[3].edge[1] = 10;
	gCapFace.faces[3].edge[2] = -11;

	gCapFace.faces[4].edge[0] = -5;
	gCapFace.faces[4].edge[1] = 11;
	gCapFace.faces[4].edge[2] = -12;

	gCapFace.faces[5].edge[0] = -6;
	gCapFace.faces[5].edge[1] = 12;
	gCapFace.faces[5].edge[2] = -7;

	EXIT( "InitBeamObject" );
}

ObjectInstancePtr CreateBeam( XCGENBeamPtr pTemplate )
{
	ENTER;

	ObjectInstancePtr	pObj;

	pObj = CreateNewObject( NULL, XCGEN_TYPEBEAM );
	memcpy( pObj->pBeam, pTemplate, sizeof(XCGENBeam) );

	return( pObj );

	EXIT( "CreateBeam" );
}

Void MakeCapVerts( Float width )
{
	ENTER;

	gCapFace.verts[0].p[1] = width;
	gCapFace.verts[3].p[1] = -width;
	gCapFace.verts[1].p[0] = -0.86f * width;
	gCapFace.verts[1].p[1] = 0.5f * width;
	gCapFace.verts[2].p[0] = -0.86f * width;
	gCapFace.verts[2].p[1] = -0.5f * width;
	gCapFace.verts[4].p[0] = 0.86f * width;
	gCapFace.verts[4].p[1] = -0.5f * width;
	gCapFace.verts[5].p[0] = 0.86f * width;
	gCapFace.verts[5].p[1] = 0.5f * width;

	EXIT( "MakeCapVerts" );
}

Void RenderBeamObject( Vec_3 tail, Vec_3 head, XCGENTexturePtr pTex, Float hwidth, Float twidth )
{
	ENTER;

	Int		i;
	Float	length;
	Vec_3	v;
	Mat_3	m, bmat;

	VectorSubtract( v, head, tail );

	length = Float(sqrt(DotProduct(v,v)));
	
	// Setup vertices
	gBeamFace.verts[0].p[1] = hwidth;
	gBeamFace.verts[5].p[1] = -hwidth;
	gBeamFace.verts[1].p[1] = twidth;
	gBeamFace.verts[4].p[1] = -twidth;
	gBeamFace.verts[1].p[2] = length;
	gBeamFace.verts[2].p[2] = length;
	gBeamFace.verts[3].p[2] = length;
	gBeamFace.verts[4].p[2] = length;

	Float	tscale, scale;
	Float	tw, th;

	tw = Float(1<<pTex->width);
	th = Float(1<<pTex->height);

	tscale = (hwidth*2.0f) / th;
	
	//if (hwidth==twidth)
	//{
		gBeamFace.verts[0].t[0] = 
			gBeamFace.verts[6].t[0] = 
			gBeamFace.verts[5].t[0] = 0.0f;

		gBeamFace.verts[1].t[0] = 
			gBeamFace.verts[2].t[0] =
			gBeamFace.verts[3].t[0] =
			gBeamFace.verts[4].t[0] = length/(tscale*tw);

		gBeamFace.verts[0].t[1] = 
			gBeamFace.verts[1].t[1] = 0.0f;

		gBeamFace.verts[6].t[1] = 0.5f;
		
		gBeamFace.verts[2].t[1] = 0.5f * twidth / hwidth;
		gBeamFace.verts[3].t[1] = 1.0f - gBeamFace.verts[2].t[1];

		gBeamFace.verts[5].t[1] = 
			gBeamFace.verts[4].t[1] = 1.0f;
	/*}
	else if (hwidth>twidth)
	{*/
		/*Float	dwidth;

		dwidth = hwidth - twidth;

		gBeamFace.verts[5].t[0] = 0.0f;

		gBeamFace.verts[1].t[1] = 
			gBeamFace.verts[0].t[1] = 0.0f;
		gBeamFace.verts[4].t[1] = 
			gBeamFace.verts[5].t[1] = 1.0f;

		gBeamFace.verts[6].t[0] = Float(sqrt( hwidth*hwidth - 0.25f ));
		gBeamFace.verts[6].t[1] = 0.5f;

		gBeamFace.verts[1].t[0] =
			gBeamFace.verts[4].t[0] = Float(sqrt( dwidth*dwidth + length*length ));

		gBeamFace.verts[3].t[0] = 
			gBeamFace.verts[2].t[0] = gBeamFace.verts[1].t[0] + (gBeamFace.verts[6].t[0] * dwidth / length);

		gBeamFace.verts[2].t[1] = 0.5f * dwidth / length;
		gBeamFace.verts[3].t[1] = 1.0f - gBeamFace.verts[2].t[1];*/

/*	}
	else
	{
		Float	dwidth;

		dwidth = twidth - hwidth;

		gBeamFace.verts[5].t[0] = 0.0f;

		gBeamFace.verts[1].t[1] = 
			gBeamFace.verts[0].t[1] = 0.0f;
		gBeamFace.verts[4].t[1] = 
			gBeamFace.verts[5].t[1] = 1.0f;

		gBeamFace.verts[6].t[0] = Float(sqrt( twidth*twidth - 0.25f ));
		gBeamFace.verts[6].t[1] = 0.5f;

		gBeamFace.verts[1].t[0] =
			gBeamFace.verts[4].t[0] = Float(sqrt( dwidth*dwidth + length*length ));

		gBeamFace.verts[3].t[0] = 
			gBeamFace.verts[2].t[0] = gBeamFace.verts[1].t[0] + (gBeamFace.verts[6].t[0] * dwidth / length);

		gBeamFace.verts[2].t[1] = 0.5f * dwidth / length;
		gBeamFace.verts[3].t[1] = 1.0f - gBeamFace.verts[2].t[1];

	}*/

	/*scale = (length / tscale) / tw;

	gBeamFace.verts[1].t[0] = scale;
	gBeamFace.verts[2].t[0] = scale;*/

	if (gRenderSurf.bD3D)
		gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );

	scale = 1.0f / (tw/(th*0.5f));
	gCapFace.verts[0].t[0] = scale;
	gCapFace.verts[2].t[0] = scale;
	gCapFace.verts[4].t[0] = scale;
	gCapFace.verts[6].t[0] = scale*0.5f;
	gCapFace.verts[6].t[1] = 0.5f;

	/*if (gRenderSurf.bD3D)
	{
		for (i=0;i<2;i++)
		{
			gBeamFace.faces[i].face.pTexture = NULL;
			gBeamFace.faces[i].face.r = 128;
			gBeamFace.faces[i].face.g = 128;
			gBeamFace.faces[i].face.b = 128;
		}
		for (i=0;i<6;i++)
		{
			gCapFace.faces[i].face.pTexture = NULL;
			gCapFace.faces[i].face.r = 128;
			gCapFace.faces[i].face.g = 128;
			gCapFace.faces[i].face.b = 128;
		}
	}
	else
	{*/
		for (i=0;i<2;i++)
			gBeamFace.faces[i].face.pTexture = pTex;
		for (i=0;i<6;i++)
			gCapFace.faces[i].face.pTexture = pTex;
	/*}*/

	VectorSubtract( v, tail, head );

	VectorScale( v, v, 1.0f/length );	// Normalize
	VectorCopy( m[2], v );

	VectorSubtract( v, gCamera.pos, head );
	NormalizeVector( v );
	CrossProd( m[1], m[2], v );
	NormalizeVector( m[1] );
	CrossProd( m[0], m[1], m[2] );

	VectorCopy( bmat[0], m[0] );
	VectorCopy( bmat[1], m[1] );
	VectorCopy( bmat[2], m[2] );

	//Just in case
	gpStack = &gpObjStack[0];
	gpStack->nLights = 0;
	gpStack->pMesh = NULL;
	gpStack->pLOD = NULL;
	gpStack->pObj = &gBeamObj;
	
	// CAP HEAD
	MakeCapVerts( hwidth );
	
	// Clear edge cache
	for (i=0;i<13;i++)
		gCapFace.edges[i].iCache = -1;

	VectorCopy( gBeamObj.framepos, head );

	VectorSubtract( v, head, gCamera.pos );

	//NormalizeVector( v );
	VectorCopy( m[1], bmat[1] );
	CrossProd( m[0], m[1], v );
	NormalizeVector( m[0] );
	CrossProd( m[2], m[0], m[1] );

	MatTrans( gBeamObj.framemat, m );

	gpStack->pVerts = gCapFace.verts;
	gpStack->pEdges = gCapFace.edges;

	GenerateFrustum();
	ClearEdgeLists();
		
	if (gRenderSurf.bD3D)
	{
		for (i=0;i<6;i++)
			D3DRenderFace( &gCapFace.faces[i].face, 15 );
	}
	else
	{
		for (i=0;i<6;i++)
			AddFace( &gCapFace.faces[i].face, 15 );
	}

	if (!gRenderSurf.bD3D)
	{
		ScanEdges();
		DrawSurfaces();
	}

	//CAP TAIL
	gCapFace.verts[6].t[1] = 0.5f * twidth / hwidth;

	MakeCapVerts( twidth );
	
	// Clear edge cache
	for (i=0;i<13;i++)
		gCapFace.edges[i].iCache = -1;

	VectorCopy( gBeamObj.framepos, tail );

	VectorSubtract( v, tail, gCamera.pos );

	//NormalizeVector( v );
	VectorCopy( m[1], bmat[1] );
	CrossProd( m[0], m[1], v );
	NormalizeVector( m[0] );
	CrossProd( m[2], m[0], m[1] );

	MatTrans( gBeamObj.framemat, m );

	gpStack->pVerts = gCapFace.verts;
	gpStack->pEdges = gCapFace.edges;

	GenerateFrustum();
	ClearEdgeLists();
		
	if (gRenderSurf.bD3D)
	{
		for (i=0;i<6;i++)
			D3DRenderFace( &gCapFace.faces[i].face, 15 );
	}
	else
	{
		for (i=0;i<6;i++)
			AddFace( &gCapFace.faces[i].face, 15 );
	}

	if (!gRenderSurf.bD3D)
	{
		ScanEdges();
		DrawSurfaces();
	}

	// DRAW BEAM
	VectorCopy( gBeamObj.framepos, head );
	MatTrans( gBeamObj.framemat, bmat );

	for (i=0;i<8;i++)
		gBeamFace.edges[i].iCache = -1;

	gpStack->pVerts = gBeamFace.verts;
	gpStack->pEdges = gBeamFace.edges;

	GenerateFrustum();
	ClearEdgeLists();
		
	if (gRenderSurf.bD3D)
	{
		for (i=0;i<2;i++)
			D3DRenderFace( &gBeamFace.faces[i].face, 15 );
	}
	else
	{
		for (i=0;i<2;i++)
			AddFace( &gBeamFace.faces[i].face, 15 );
	}

	if (!gRenderSurf.bD3D)
	{
		ScanEdges();
		DrawSurfaces();
	}

	if (gRenderSurf.bD3D)
		gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );

	EXIT( "RenderBeamObject" );
}

Void RenderBeam( ObjectInstancePtr pBeam )
{
	ENTER;

	XCGENBeamPtr	pB;
	Vec_3			v;

	gBeamObj.obj.alpha = pBeam->obj.alpha;

	pB = pBeam->pBeam;

	VectorSubtract( v, pB->head, pB->tail ); 
	if (DotProduct(v,pB->vhead)<0.0f)
		return;

	if (pB->twidth>pB->hwidth)
	{
		//VectorSubtract( v, pB->tail, pB->head );
		//VectorScale( v, v, 1.0f - pB->hwidth/pB->twidth );
		//VectorAdd( v, v, pB->head );
		//RenderRealWorldLine( v, pB->tail, pB->r, pB->g, pB->b );

		RenderBeamObject( pB->head, pB->tail, pB->pTex, pB->twidth*0.5f, pB->hwidth*0.5f );
	}
	else
	{
		//VectorSubtract( v, pB->head, pB->tail );
		//VectorScale( v, v, 1.0f - pB->twidth/pB->hwidth );
		//VectorAdd( v, v, pB->tail );
		//RenderRealWorldLine( v, pB->head, pB->r, pB->g, pB->b );

		RenderBeamObject( pB->tail, pB->head, pB->pTex, pB->hwidth*0.5f, pB->twidth*0.5f );
	}

	EXIT( "RenderBeam" );
}

