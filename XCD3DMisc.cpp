/*******************************************************************
**    File: XCD3DMisc.cpp
**      By: Paul L. Rowan 
** Created: 980106
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
Void D3DTestCaps( Void )
{
	ENTER;

	HRESULT			res;
	D3DDEVICEDESC	d3dhal, d3dhel;

	memset( &d3dhal, 0, sizeof(D3DDEVICEDESC) );
	d3dhal.dwSize = sizeof(D3DDEVICEDESC);
	d3dhal.dwFlags = D3DDD_COLORMODEL + D3DDD_DEVCAPS + 
						D3DDD_DEVICERENDERBITDEPTH + D3DDD_DEVICEZBUFFERBITDEPTH +
						D3DDD_LINECAPS + D3DDD_TRICAPS;


	memset( &d3dhel, 0, sizeof(D3DDEVICEDESC) );
	d3dhel.dwFlags = D3DDD_COLORMODEL + D3DDD_DEVCAPS + 
						D3DDD_DEVICERENDERBITDEPTH + D3DDD_DEVICEZBUFFERBITDEPTH +
						D3DDD_LINECAPS + D3DDD_TRICAPS;
	d3dhel.dwSize = sizeof(D3DDEVICEDESC);

	res = gRenderSurf.pD3DDev2->GetCaps( &d3dhal, &d3dhel );
	if (res!=DD_OK)
	{
		FATALError( "Cannot get DIRECT3DDEVICE2 caps" );
	}

	if (d3dhal.dwDevCaps & D3DDEVCAPS_DRAWPRIMTLVERTEX)
		gStats.bDrawPrim = TRUE;
	else
		gStats.bDrawPrim = FALSE;
	
	if ( (d3dhal.dwDeviceRenderBitDepth = DDBD_16) && (d3dhal.dwDeviceZBufferBitDepth = DDBD_16) )
		gStats.b16Bit = TRUE;
	else
		gStats.b16Bit = FALSE;

	if ( d3dhal.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR )
		gStats.bFilterTex = TRUE;
	else
		gStats.bFilterTex = FALSE;

	if (d3dhal.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_TRANSPARENCY )
		gStats.bColorKey = TRUE;
	else
		gStats.bColorKey = FALSE;

	if (d3dhal.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDBLEND )
		gStats.bAlpha = TRUE;
	else
		gStats.bAlpha = FALSE;

	if (d3dhal.dwDevCaps & D3DDEVCAPS_TLVERTEXSYSTEMMEMORY)
		gStats.bVertSysMem = TRUE;
	else
		gStats.bVertSysMem = FALSE;

	if (d3dhal.dwDevCaps & D3DDEVCAPS_TEXTURESYSTEMMEMORY)
		gStats.bTexSysMem = TRUE;
	else
		gStats.bTexSysMem = FALSE;

	if (d3dhal.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGVERTEX)
		gStats.bFogVert = TRUE;
	else
		gStats.bFogVert = FALSE;

	if (d3dhal.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE)
		gStats.bFogTable = TRUE;
	else
		gStats.bFogTable = FALSE;

	EXIT( "D3DTestCaps" );
}
