/*******************************************************************
**    File: XCD3D.cpp
**      By: Paul L. Rowan     
** Created: 970527
**
** Description:
**
********************************************************************/

/*--- INCLUDES ----------------------------------------------------*/
#define INITGUID

#include "xcgenint.h"

/*--- CONSTANTS ---------------------------------------------------*/


/************************ IMPORT SECTION ***************************/
/*-------- FUNCTION PROTOTYPES ------------------------------------*/

/*-------- GLOBAL VARIABLES ---------------------------------------*/


/************************ PRIVATE SECTION **************************/
/*-------- FUNCTION PROTOTYPES ------------------------------------*/

/*-------- GLOBAL VARIABLES ---------------------------------------*/

/************************ MEMBER FUNCTIONS *************************/
/*HRESULT WINAPI EnumD3DFunc( LPGUID pGuid, LPSTR pDesc, LPSTR pName, 
							LPD3DDEVICEDESC pHWDesc, LPD3DDEVICEDESC pHELDesc,
							LPVOID pContext )
{
	HRESULT	res;

	DEBUGLog( "\nD3DName: " ); DEBUGLog( pName ); DEBUGLog( "\n" );
	DEBUGLog( "D3DDesc: " ); DEBUGLog( pDesc ); DEBUGLog( "\n" );

	if (!strcmp(pName,"Direct3D HAL"))
	{
		// We are hardware accelerated
		gRenderSurf.bD3D = TRUE;

		res = gRenderSurf.pDDS->QueryInterface( *pGuid, (LPVOID*)&gRenderSurf.pD3DDev );
		if (res != D3D_OK )
		{
			DEBUGLog( "ERROR: Can't get Direct3D Device\n" );
			return( D3DENUMRET_OK );
		}

		res = gRenderSurf.pD3D2->CreateDevice( *pGuid, gRenderSurf.pDDS, &gRenderSurf.pD3DDev2 );
		if (res != D3D_OK )
		{
			DEBUGLog( "ERROR: Can't get Direct3D Device2\n" );
			return( D3DENUMRET_OK );
		}
	}

	return( D3DENUMRET_OK );
}

Bool EnumD3DDevices( Void )
{
	HRESULT		res;
	
	res = gRenderSurf.pD3D->EnumDevices( EnumD3DFunc, NULL );
	if (res != DD_OK )
	{
		DEBUGLog( "ERROR: Cannot enumerate devices\n" );
		return( FALSE );
	}
	return( TRUE );
}


BOOL WINAPI DDEnumCallback( GUID FAR * pGUID, LPSTR pDesc, LPSTR pName, LPVOID pContext )
{
	LPDIRECTDRAW		pDD;
	LPDIRECTDRAWSURFACE	pDDS, pDDSF;
	DDSURFACEDESC		ddsd;
	DDSCAPS				ddscaps;
	HRESULT				res;

	DEBUGLog( "\nDD Device:"); DEBUGLog( pName ); DEBUGLog( "\n" );
	DEBUGLog( "DD Desc: " ); DEBUGLog( pDesc ); DEBUGLog( "\n" );

	if (!strcmp(pName,"dd3dfx"))
	{
		// Switch to the newly found 3D Device
		res = DirectDrawCreate( pGUID, &pDD, NULL );
		if (res!=DD_OK)
		{
			DEBUGLog( "3DFXERROR: Cannot get pointer to 3D Device.\n" );
			return( TRUE );
		}

		// Reset coopertive level on old DirectDraw device
		res = gRenderSurf.pDD->SetCooperativeLevel( gRenderSurf.hWnd, DDSCL_NORMAL );
		if (res!=DD_OK)
		{
			DEBUGLog( "3DFXERROR: Cannot reset cooperative level to normal.\n" );
			return( TRUE );
		}

		res = pDD->SetCooperativeLevel( gRenderSurf.hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
		if (res!=DD_OK)
		{
			DEBUGLog( "3DFXERROR: Cannot set cooperative level.\n" );
			return( TRUE );
		}

		res = pDD->SetDisplayMode( 640, 480, 16 );
		if( res != DD_OK )
		{
			DEBUGLog( "3DFXERROR: Can't set 640x480x16\n" );
			return( TRUE );
		}

		// CREATE THE PRIMARY SURFACE AND BACK BUFFER
		memset( &ddsd, 0, sizeof(DDSURFACEDESC) );
		ddsd.dwSize	= sizeof( DDSURFACEDESC );
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT ;
		ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_PRIMARYSURFACE; 
		ddsd.dwBackBufferCount = 1;
		res = pDD->CreateSurface( &ddsd, &pDDSF, NULL );
		if( res != DD_OK )
		{
			DEBUGLog( "3DFXERROR: Can't create primary surface\n" );
			return( TRUE );
		}
		memset( &ddsd, 0, sizeof(DDSURFACEDESC) );
		ddsd.dwSize	= sizeof( DDSURFACEDESC );
		pDDSF->GetSurfaceDesc( &ddsd );
		if (ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
			DEBUGLog( "YES: Primary surface in video memory\n" );
		else
			DEBUGLog( "3DFXERROR: Primary surface in system memory - IMPOSSIBLE\n" );

		// GRAB THE BACK BUFFER
		ddscaps.dwCaps =  DDSCAPS_3DDEVICE | DDSCAPS_BACKBUFFER;
		res = pDDSF->GetAttachedSurface( &ddscaps, &pDDS );
		if (res != DD_OK )
		{
			DEBUGLog( "3DFXERROR: Can't get back buffer\n" );
			return( TRUE );
		}
		memset( &ddsd, 0, sizeof(DDSURFACEDESC) );
		ddsd.dwSize	= sizeof( DDSURFACEDESC );
		pDDS->GetSurfaceDesc( &ddsd );
		if (ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
			DEBUGLog( "YES: Back buffer in video memory\n" );
		else
			DEBUGLog( "WARNING: Back buffer in system memory\n" );


		DEBUGLog( "3DFX: Initialized\n" );

		gRenderSurf.pDD = pDD;
		gRenderSurf.pDDS = pDDS;
		gRenderSurf.pDDSF = pDDSF;
	}

	return( TRUE );
}

Void CheckAndInitDirect3D( Void )
{
	HRESULT	res;

	/// ENUMERATE DIRECTDRAW DEVICES, and reinitialize to the 3d device
	res = DirectDrawEnumerate( DDEnumCallback, NULL );

	// Assumes gRenderSurf is already initialized
	res = gRenderSurf.pDD->QueryInterface( IID_IDirect3D, (LPVOID*)&gRenderSurf.pD3D );
	if (res != DD_OK )
	{
		DEBUGLog( "ERROR: Direct3D not installed.\n" );
		return;
	}
	res = gRenderSurf.pD3D->QueryInterface( IID_IDirect3D2, (LPVOID*)&gRenderSurf.pD3D2 );
	if (res != DD_OK )
	{
		DEBUGLog( "ERROR: Direct3D2 not installed.\n" );
		return;
	}

	// ENUMERATE DIRECT3D DEVICES
	if (EnumD3DDevices())
	{
		
	}
	else
	{
		DEBUGLog( "ERROR: Can't get a Direct3D device\n" );
		return;
	}
}
*/