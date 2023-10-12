/*******************************************************************
**    File: XCD3DPolyBuff.cpp
**      By: Paul L. Rowan
** Created: 980126
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


/************************ MEMBER FUNCTIONS *************************/
Void RenderBuffer( D3DPolyBuffPtr pBuff )
{
	ENTER;

	HRESULT		res;

	if (!pBuff->pTex)
		res = gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_TEXTUREHANDLE, NULL );
	else
		res = gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_TEXTUREHANDLE, GetD3DTexture(pBuff->pTex) );
	if (res!=D3D_OK)
		FATALErrorDD( "SetRenderState failed", res );

	res = gRenderSurf.pD3DDev2->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, pBuff->verts, pBuff->nVerts, pBuff->indices, pBuff->nIndices, D3DDP_DONOTCLIP );
	if (res!=D3D_OK)
		FATALErrorDD( "DrawIndexedPrimitive failed", res );

	pBuff->nVerts = 0;
	pBuff->nIndices = 0;

	EXIT( "RenderBuffer" );
}

Void AddPolyToBuffer( D3DPolyBuffPtr pBuff, D3DTLVERTEX *pVerts, Int nVerts )
{
	ENTER;

	Int		i, n, svert;

	if (pBuff->nVerts+nVerts > MAX_D3DVERTICES)
	{
		RenderBuffer( pBuff );
		gStats.nFullBuffs++;
	}

	// Copy vertices
	svert = n = pBuff->nVerts;
	for (i=0;i<nVerts;i++)
		pBuff->verts[n++] = pVerts[i];
	pBuff->nVerts = n;

	// Triangulate polys
	n = pBuff->nIndices;
	pBuff->indices[n++] = svert;
	pBuff->indices[n++] = svert+1;
	pBuff->indices[n++] = svert+2;	
	for (i=3;i<nVerts;i++)
	{
		pBuff->indices[n++] = svert;
		pBuff->indices[n++] = svert+i-1;
		pBuff->indices[n++] = svert+i;
	}
	pBuff->nIndices = n;

	EXIT( "AddPolyToBuffer" );
}

Void BufferD3DPoly( XCGENTexturePtr pTex, D3DTLVERTEX *pVerts, Int nVerts )
{
	ENTER;

	Int				i;
	D3DPolyBuffPtr	pBuff;

	pBuff = gBuff.polybuff;

	// Find shared texture in list
	for (i=0;i<gnBuffs;i++)
	{
		if (pTex==pBuff[i].pTex)
		{
			AddPolyToBuffer( &pBuff[i], pVerts, nVerts );
			return;
		}
	}

	if (gnBuffs>=MAX_D3DPOLYBUFFERS)
		FlushD3DPolyBuffers();		

	pBuff[gnBuffs].pTex = pTex;
	AddPolyToBuffer( &pBuff[gnBuffs], pVerts, nVerts );
	gnBuffs++;

	EXIT( "DrawPolyD3D" );
}

Void FlushD3DPolyBuffers( Void )
{
	ENTER;

	Int				i;
	D3DPolyBuffPtr	pBuff;

	pBuff = gBuff.polybuff;

	for (i=0;i<gnBuffs;i++)
	{
		RenderBuffer(&pBuff[i]);
	}

	gnBuffs = 0;

	EXIT( "FlushD3DPolyBuffers" );
}