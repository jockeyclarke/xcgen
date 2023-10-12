/*******************************************************************
**    File: XCD3DTexture.cpp
**      By: Paul L. Rowan
** Created: 970612
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
#define MAXTEXTURES		256
#define MAXTEXWIDTH		128
#define MAXTEXSIZE		(MAXTEXWIDTH*MAXTEXWIDTH*sizeof(Short))
/*-------- GLOBAL VARIABLES ---------------------------------------*/
typedef struct
{
	Int						width, height;
	LPDIRECT3DTEXTURE		pSrcTex, pTex;
	LPDIRECTDRAWSURFACE		pSurf;
	D3DTEXTUREHANDLE		hTex;
	Int						lastUsed;
} D3DTexCache, *D3DTexCachePtr; 

Int				gCacheSize;
D3DTexCache		gTexCache[MAXTEXTURES];


/************************ MEMBER FUNCTIONS *************************/
Bool InitD3DTex( Int index, Int width, Int height )
{
	ENTER;

	DDSURFACEDESC		ddsd, ddsdd;
	HRESULT				res;

	memset( &ddsd, 0, sizeof(DDSURFACEDESC) );
	ddsd.dwSize = sizeof(DDSURFACEDESC);
	ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_ALLOCONLOAD|DDSCAPS_TEXTURE|DDSCAPS_VIDEOMEMORY;
	ddsd.dwHeight = height;
	ddsd.dwWidth = width;
	ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
	ddsd.ddpfPixelFormat.dwRBitMask = 0x0000F800;
	ddsd.ddpfPixelFormat.dwGBitMask = 0x000007E0;
	ddsd.ddpfPixelFormat.dwBBitMask = 0x0000001F;
	res = gRenderSurf.pDD->CreateSurface( &ddsd, &gTexCache[index].pSurf, NULL );
	if (res!=DD_OK)
	{
		return( FALSE );
	}

	memset( &ddsdd, 0, sizeof(DDSURFACEDESC) );
	ddsdd.dwSize = sizeof(DDSURFACEDESC);
	ddsdd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT;
	gTexCache[index].pSurf->GetSurfaceDesc( &ddsdd );

	if (!(ddsdd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY))
	{
		FATALError( "Destination texture not in video memory" );
	}

	res = gTexCache[index].pSurf->QueryInterface( IID_IDirect3DTexture, (LPVOID*)&gTexCache[index].pTex );
	if (res!=DD_OK)
	{
		FATALErrorDD( "Cannot get Direct3DTexture interface.", res );
	}
	
	res = gTexCache[index].pTex->GetHandle( gRenderSurf.pD3DDev, &gTexCache[index].hTex );
	if (res!=DD_OK)
	{
		FATALErrorDD( "Cannot get handle to texture.", res );
	}

	gTexCache[index].width = width;
	gTexCache[index].height = height;

	return( TRUE );

	EXIT( "InitD3DTex" );
}

Void DeleteD3DTex( Int index )
{
	ENTER;

	HRESULT		res;

	res = gTexCache[index].pTex->Unload();
	if (res!=DD_OK)
	{
		FATALErrorDD( "Cannot unload texture", res );
	}

	res = gTexCache[index].pSurf->Release();
	//if (res!=DD_OK)
	//{
	//	FATALErrorDD( "Cannot release surface", res );
	//}

	res = gTexCache[index].pTex->Release();
	if (res!=DD_OK)
	{
		FATALErrorDD( "Cannot release texture", res );
	}


	gTexCache[index].pTex = NULL;
	gTexCache[index].pSrcTex = NULL;

	EXIT( "DeleteD3DTex" );
}

Void InitD3DTextureCache( Void )
{
	ENTER;

	Bool	gCacheInited;
	LPDIRECTDRAW2		pDD2;
	HRESULT				res;
	Int					i;
	ULong				totaltexmem, freetexmem;
	ULong				nlvidmem;
	ULong				freenlvidmem;
	DDSCAPS				ddscaps;

	//if (gCacheInited)
	//	return;

	res = gRenderSurf.pDD->QueryInterface( IID_IDirectDraw2, (LPVOID *)&pDD2 );
	if (res!=DD_OK)
	{
		FATALErrorDD( "Cannot get DirectDraw2 interface", res );
	}

	memset( &ddscaps, 0, sizeof(DDSCAPS) );
	ddscaps.dwCaps = DDSCAPS_NONLOCALVIDMEM;
	res = pDD2->GetAvailableVidMem( &ddscaps, &nlvidmem, &freenlvidmem );
	if (res!=DD_OK)
	{
		FATALErrorDD( "Cannot get non-local video memory size", res );
	}
	if (nlvidmem>0)
	{
		gRenderSurf.bAGP = TRUE;
		return;
	}

	gRenderSurf.bAGP = FALSE;

	memset( &ddscaps, 0, sizeof(DDSCAPS) );
	ddscaps.dwCaps = DDSCAPS_TEXTURE;
	res = pDD2->GetAvailableVidMem( &ddscaps, &totaltexmem, &freetexmem );
	if (res!=DD_OK)
	{
		FATALErrorDD( "Cannot get texture memory size", res );
	}

	gCacheSize = min((freetexmem/MAXTEXSIZE),MAXTEXTURES);
	//gCacheSize = 2;

	if (gCacheSize==0)
	{
		FATALError( "No texture memory available" );
	}

	for (i=0;i<gCacheSize;i++)
	{
	}	
	
	gCacheInited = TRUE;

	EXIT( "InitD3DTextureCache" );
}

Void FlushD3DTextureCache( Void )
{
	ENTER;

	Int		i;

	for (i=0;i<gCacheSize;i++)
	{
		if (gTexCache[i].pTex)
			DeleteD3DTex( i );
	}

	EXIT( "FlushD3DTextureCache" );
}

D3DTEXTUREHANDLE GetD3DTexture( XCGENTexturePtr pTex )
{
	ENTER;

	if (gRenderSurf.bAGP)
	{
		D3DTEXTUREHANDLE	hTex;
		HRESULT				res;
		
		res = pTex->d3dinfo.pD3DTex->GetHandle( gRenderSurf.pD3DDev, &hTex );
		if (res!=DD_OK)
		{
			FATALErrorDD( "Cannot get handle to texture.", res );
		}

		return( hTex );
	}
	else
	{

		static	Int				frame;
				Int				i, min, imin;
				HRESULT			res;
				DDSURFACEDESC	ddsd;
				Bool			okay;

	FindIt:
		imin = 0;
		min = 999999999;
		for (i=0;i<gCacheSize;i++)
		{
			if (gTexCache[i].pSrcTex==pTex->d3dinfo.pD3DTex)
				return( gTexCache[i].hTex );

			if (gTexCache[i].lastUsed<min)
			{
				min = gTexCache[i].lastUsed;
				imin = i;
			}
		}

		if (!gTexCache[imin].pTex)
		{
			okay = InitD3DTex( imin, pTex->width, pTex->height );
		}
		else if (gTexCache[imin].width!=pTex->width || gTexCache[imin].height!=pTex->height)
		{
			DeleteD3DTex( imin );
			okay = InitD3DTex( imin, pTex->width, pTex->height );
		}

		if (!okay)
		{
			gTexCache[imin].lastUsed = frame;
			goto FindIt;
		}

		gTexCache[imin].pSrcTex = pTex->d3dinfo.pD3DTex;

		memset( &ddsd, 0, sizeof(DDSURFACEDESC) );
		ddsd.dwSize = sizeof(DDSURFACEDESC);
		ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT;
		gTexCache[imin].pSurf->GetSurfaceDesc( &ddsd );

		res = gTexCache[imin].pTex->Load( pTex->d3dinfo.pD3DTex );
		if (res!=DD_OK)
		{
			FATALErrorDD( "Cannot load texture", res );
		}
		gTexCache[imin].lastUsed = frame++;

		gStats.nTexLoads++;

		return( gTexCache[imin].hTex );
	}

	EXIT( "GetD3DTexture" );
}
