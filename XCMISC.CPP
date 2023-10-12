/*******************************************************************
**    File: XCMisc.cpp
**      By: Paul L. Rowan     
** Created: 970415
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


/************************ MEMBER FUNCTIONS *************************/
Void LockRenderSurface( Void )
{
	ENTER;

	DDSURFACEDESC	ddsd;
	HRESULT			res;

	memset( &ddsd, 0, sizeof(DDSURFACEDESC) );
	ddsd.dwSize = sizeof(DDSURFACEDESC);
	ddsd.dwFlags = DDSD_PITCH;

	gRenderSurf.pDDS->Lock( NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL );
	gRenderSurf.pSurf = ShortPtr(ddsd.lpSurface);
	gRenderSurf.surfpitch = ddsd.lPitch/2;

	//if (gRenderSurf.bD3D)
	//	gRenderSurf.pDDS->Unlock( gRenderSurf.pSurf );

	gRenderSurf.pZDDS->Lock( NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL );
	gRenderSurf.pZBuff = ShortPtr(ddsd.lpSurface);
	gRenderSurf.zbuffpitch = ddsd.lPitch/2;

	//if (gRenderSurf.bD3D)
	//	gRenderSurf.pZDDS->Unlock( gRenderSurf.pZBuff );

	EXIT( "LockRenderSurface" );
}

Void UnlockRenderSurface( Void )
{
	ENTER;

	if (gRenderSurf.bD3D)
		return;
	
	gRenderSurf.pDDS->Unlock( gRenderSurf.pSurf );
	gRenderSurf.pZDDS->Unlock( gRenderSurf.pZBuff );

	EXIT( "UnlockRenderSurface" );
}

#define CWIDTH	180
#define CHEIGHT	16

Void DisplayStats( Void )
{
	ENTER;

	Char	num[64];
	RECT	r;
	HDC		hDC;
	Int		outside;

	gRenderSurf.pDDS->GetDC( &hDC );

	SelectObject( hDC, GetStockObject( ANSI_VAR_FONT ) );
	SetTextColor( hDC, RGB(128,128,156) );
	SetBkColor( hDC, RGB(0,0,0) );
	SetBkMode( hDC, TRANSPARENT );

	// MEMORY STATS
	SetRect( &r, 0, 0, CWIDTH, CHEIGHT );
	OffsetRect( &r, gViewport.xoff+2, gViewport.yoff+2 );
	sprintf( num, "Objects: %4d %4dK", gStats.nObjects, gStats.memObjects/1000 );
	DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );

	r.top += CHEIGHT;
	r.bottom += CHEIGHT;
	sprintf( num, "Textures: %4d %4dK", gStats.nTextures, gStats.memTextures/1000 );
	DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );

	r.top += CHEIGHT;
	r.bottom += CHEIGHT;
	sprintf( num, "Sprites: %4d %4dK", gStats.nSprites, gStats.memSprites/1000 );
	DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );

	r.top += CHEIGHT;
	r.bottom += CHEIGHT;
	sprintf( num, "Meshes: %4d %4dK", gStats.nMeshes, gStats.memMeshes/1000 );
	DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );

	// TIMINGS
	r.top += CHEIGHT;
	r.bottom += CHEIGHT;
	sprintf( num, "CPU:  %4d.%d MHz", (gCycleCal/10)/10, (gCycleCal/10)%10 );
	DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );

	r.top += CHEIGHT;
	r.bottom += CHEIGHT;
	if (gRenderSurf.b555)
		sprintf( num, "16Bit:  %4d", 555 );
	else
		sprintf( num, "16Bit:  %4d", 565 );
	DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );

	r.top += CHEIGHT;
	r.bottom += CHEIGHT;
	sprintf( num, "Polys:  %4d", gStats.nPolygons );
	DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );

	r.top += CHEIGHT;
	r.bottom += CHEIGHT;
	sprintf( num, "      Surf Draw: %4d.%d ms.", (gStats.tsSurfDraw/gCycleCal)/10, (gStats.tsSurfDraw/gCycleCal)%10);
	DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );

	r.top += CHEIGHT;
	r.bottom += CHEIGHT;
	sprintf( num, "      Edge Sort: %4d.%d ms.", (gStats.tsEdgeSort/gCycleCal)/10, (gStats.tsEdgeSort/gCycleCal)%10);
	DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );

	r.top += CHEIGHT;
	r.bottom += CHEIGHT;
	sprintf( num, "      Particles: %4d.%d ms.", (gStats.tsParticles/gCycleCal)/10, (gStats.tsParticles/gCycleCal)%10);
	DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );

	r.top += CHEIGHT;
	r.bottom += CHEIGHT;
	if (gStats.tsFrame/gCycleCal<300)
		SetTextColor( hDC, RGB(128,128,156) );
	else
		SetTextColor( hDC, RGB(128,0,0) );
	sprintf( num, "   Render: %4d.%d ms.", (gStats.tsFrame/gCycleCal)/10, (gStats.tsFrame/gCycleCal)%10);
	DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );
	SetTextColor( hDC, RGB(128,128,156) );

	r.top += CHEIGHT;
	r.bottom += CHEIGHT;
	sprintf( num, "      Regions: %4d.%d ms.", (gStats.tsRegions/gCycleCal)/10, (gStats.tsRegions/gCycleCal)%10);
	DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );

	r.top += CHEIGHT;
	r.bottom += CHEIGHT;
	if (gStats.tsMotion/gCycleCal<200)
		SetTextColor( hDC, RGB(128,128,156) );
	else
		SetTextColor( hDC, RGB(128,0,0) );
	sprintf( num, "   Motion: %4d.%d ms.", (gStats.tsMotion/gCycleCal)/10, (gStats.tsMotion/gCycleCal)%10);
	DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );
	SetTextColor( hDC, RGB(128,128,156) );

	r.top += CHEIGHT;
	r.bottom += CHEIGHT;
	outside = gStats.tsLastFrame - gStats.tsFrame - gStats.tsMotion;
	if (outside/gCycleCal<300)
		SetTextColor( hDC, RGB(128,128,156) );
	else
		SetTextColor( hDC, RGB(128,0,0) );
	sprintf( num, "   Outside: %4d.%d ms.", (outside/gCycleCal)/10, (outside/gCycleCal)%10 );
	DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );
	SetTextColor( hDC, RGB(128,128,156) );

	r.top += CHEIGHT;
	r.bottom += CHEIGHT;
	sprintf( num, "Last Frame: %4d.%d ms. (%2d FPS)", (gStats.tsLastFrame/gCycleCal)/10, (gStats.tsLastFrame/gCycleCal)%10, 10000/(gStats.tsLastFrame/gCycleCal) );
	DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );

	if (gRenderSurf.bD3D)
	{
		r.top += CHEIGHT;
		r.bottom += CHEIGHT;
		sprintf( num, "FullBuffs:  %4d", gStats.nFullBuffs );
		DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );

		r.top += CHEIGHT;
		r.bottom += CHEIGHT;
		sprintf( num, "TexLoads:  %4d", gStats.nTexLoads );
		DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );

		SetRect( &r, CWIDTH, 0, CWIDTH*2, CHEIGHT );
		OffsetRect( &r, gViewport.xoff+2, gViewport.yoff+2 );
		if (gStats.bDrawPrim)
			SetTextColor( hDC, RGB(128,128,156) );
		else
			SetTextColor( hDC, RGB(128,0,0) );

		sprintf( num, "DrawPrim" );
		DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );
		r.top += CHEIGHT;
		r.bottom += CHEIGHT;

		if (gStats.bVertSysMem)
			SetTextColor( hDC, RGB(128,128,156) );
		else
			SetTextColor( hDC, RGB(128,0,0) );

		sprintf( num, "VertSysMem" );
		DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );
		r.top += CHEIGHT;
		r.bottom += CHEIGHT;

		if (gStats.bTexSysMem)
			SetTextColor( hDC, RGB(128,128,156) );
		else
			SetTextColor( hDC, RGB(128,0,0) );

		sprintf( num, "TexSysMem" );
		DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );
		r.top += CHEIGHT;
		r.bottom += CHEIGHT;

		if (gStats.b16Bit)
			SetTextColor( hDC, RGB(128,128,156) );
		else
			SetTextColor( hDC, RGB(128,0,0) );

		sprintf( num, "16BitVideo" );
		DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );
		r.top += CHEIGHT;
		r.bottom += CHEIGHT;

		if (gStats.bFilterTex)
			SetTextColor( hDC, RGB(128,128,156) );
		else
			SetTextColor( hDC, RGB(128,0,0) );

		sprintf( num, "FilteredTex" );
		DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );
		r.top += CHEIGHT;
		r.bottom += CHEIGHT;

		if (gStats.bAlpha)
			SetTextColor( hDC, RGB(128,128,156) );
		else
			SetTextColor( hDC, RGB(128,0,0) );

		sprintf( num, "AlphaBlend" );
		DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );
		r.top += CHEIGHT;
		r.bottom += CHEIGHT;

		if (gStats.bColorKey)
			SetTextColor( hDC, RGB(128,128,156) );
		else
			SetTextColor( hDC, RGB(128,0,0) );

		sprintf( num, "TransTex" );
		DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );
		r.top += CHEIGHT;
		r.bottom += CHEIGHT;

		if (gStats.bFogVert)
			SetTextColor( hDC, RGB(128,128,156) );
		else
			SetTextColor( hDC, RGB(128,0,0) );

		sprintf( num, "VertFog" );
		DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );
		r.top += CHEIGHT;
		r.bottom += CHEIGHT;

		if (gStats.bFogTable)
			SetTextColor( hDC, RGB(128,128,156) );
		else
			SetTextColor( hDC, RGB(128,0,0) );

		sprintf( num, "TableFog" );
		DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );
		r.top += CHEIGHT;
		r.bottom += CHEIGHT;

		if (gRenderSurf.bAGP)
			SetTextColor( hDC, RGB(128,128,156) );
		else
			SetTextColor( hDC, RGB(128,0,0) );

		sprintf( num, "AGP" );
		DrawText( hDC, num, strlen(num), &r, DT_TOP|DT_LEFT );
		r.top += CHEIGHT;
		r.bottom += CHEIGHT;
	}

	gRenderSurf.pDDS->ReleaseDC( hDC );

	EXIT( "DisplayStats" );
}

Void DrawBox( Vec_3 v0, Vec_3 v1, UChar r, UChar g, UChar b, Bool bObject )
{
			Int		i;
			Vec_3	s0[8];
	static	Int		lines[12][2] =  {	{0,1},{1,3},{3,2},{2,0},
										{0,4},{1,5},{3,7},{2,6},
										{4,5},{5,7},{7,6},{6,4} };


	for (i=0;i<8;i++)
	{
		s0[i][0] = (i&1) ? v0[0] : v1[0];
		s0[i][1] = (i&2) ? v0[1] : v1[1];
		s0[i][2] = (i&4) ? v0[2] : v1[2];
	}

	if (bObject)
	{
		for (i=0;i<12;i++)
		{
			RenderWorldLine( s0[lines[i][0]], s0[lines[i][1]], r, g, b );
		}
	}
	else
	{
		for (i=0;i<12;i++)
		{
			RenderRealWorldLine( s0[lines[i][0]], s0[lines[i][1]], r, g, b );
		}
	}
}

Void CalculateRotation( ObjectInstancePtr pObj )
{
	ENTER;

	IntPtr				pHPR;
	Mat_3				m, m2;

	if ( !(pObj->obj.flags & XCGEN_OBJ_MATONLY) )
	{
		pHPR = pObj->obj.offHPR;
		if (pHPR[0]==0 && pHPR[1]==0 && pHPR[2]==0)
		{
			QuaternionToMatrix( pObj->obj.mat, pObj->obj.rot );
		}
		else
		{
			QuaternionToMatrix( m, pObj->obj.rot );
			RotMatHPR( m2, pHPR[0], pHPR[1], pHPR[2] );
			MatMatMul( pObj->obj.mat, m, m2 );

			//if (!(pObj->obj.flags & XCGEN_OBJ_NODAMPVEL)) 
			//	VectorScale( pObj->obj.velocity, pObj->obj.velocity, pow(0.5f,time) );
		}
	}

	/*if (pObj->obj.pParent && (pObj->obj.flags & XCGEN_OBJ_ATTACHED))
	{
		MatMatMul( m, ObjectInstancePtr(pObj->obj.pParent)->pMesh->, pObj->obj.mat );
		MatMatMul( pObj->framemat, ObjectInstancePtr(pObj->obj.pParent)->framemat, m );
	}
	else
	{*/
		memcpy( pObj->framemat, pObj->obj.mat, sizeof(Mat_3) );
	/*}*/

	if (pObj->obj.flags & XCGEN_OBJ_CAMFACING)
	{
		Mat_3	m, m2;
		Vec_3	v;

		VectorSubtract( v, gCamera.pos, pObj->obj.pos );
		NormalizeVector( v );

		MatTrans( m, pObj->framemat );

		// Get Y vector
		CrossProd( m2[1], v, m[2] );
		NormalizeVector( m2[1] );
		// Get X vector
		CrossProd( m2[0], m2[1], m[2] );
		// Copy Z vector
		VectorCopy( m2[2], m[2] );

		MatTrans( pObj->framemat, m2 );
		//VectorCopy( pObj->obj.mat[0], m2[0] );
		//VectorCopy( pObj->obj.mat[1], m2[1] );
		//VectorCopy( pObj->obj.mat[2], m2[2] );
	}

	EXIT( "CalculateRotation" );
}

Void CalculateFramePosition( Void )
{
	ENTER;

	ObjectInstancePtr	pObj;

	// OKAY, Kludge #27.4  -- Go to the end of the list and traverse
	//  backward.  This will ensure that attached objects' parents (created
	//  first -- later in list) are initialized before their children.
	pObj = gpObjects->pNext;
	if (pObj)
	{
		while( pObj->pNext )
		{
			pObj = pObj->pNext;
		}

		while( pObj )
		{
			if (pObj->obj.flags & XCGEN_OBJ_CAMRELATIVE)
			{
				Int		i;

				Float	scale, hrep;
				//Vec_3	sc, sc2;

				DEBUGAssert( pObj->obj.repeat>0.0f );

				hrep = pObj->obj.repeat*0.5f;
				scale = 1.0f/pObj->obj.repeat;
				
				for (i=0;i<3;i++)
				{
					Float	c, a, p;

					a = gCamera.pos[i]*scale;
					c = (a - Float(floor(a)))*pObj->obj.repeat - hrep;
					p = pObj->obj.pos[i];
					if (p<c-hrep)
						p += pObj->obj.repeat;
					else if (p>c+hrep)
						p -= pObj->obj.repeat;
					pObj->framepos[i] = p - c + gCamera.pos[i];
				}
			}
			else if (pObj->obj.flags & XCGEN_OBJ_BEAM)
			{
				Vec_3	v;

				VectorCopy( v, pObj->pBeam->head );
				VectorAdd( v, v, pObj->pBeam->tail );
				VectorScale( v, v, 0.5f );
				VectorCopy( pObj->framepos, v );
			}
			else
			{
				VectorCopy( pObj->framepos, pObj->obj.pos );
			}

			CalculateRotation( pObj );

			if ( pObj->obj.pParent && (pObj->obj.flags & XCGEN_OBJ_ATTACHED))
			{
				Vec_3	v, v2;
				Mat_3	m;

				MatMatMul( m, pObj->pAttachPt->mat, pObj->framemat );
				MatMatMul( pObj->framemat, ObjectInstancePtr(pObj->obj.pParent)->framemat, m );

				MatVecMul( v, pObj->pAttachPt->mat, pObj->framepos );
				VectorAdd( v, v, pObj->pAttachPt->pos );

				MatVecMul( v2, ObjectInstancePtr(pObj->obj.pParent)->framemat, v );
 				VectorAdd( pObj->framepos, v2, ObjectInstancePtr(pObj->obj.pParent)->framepos );
			}

			VectorCopy( pObj->storepos, pObj->framepos );
			pObj->bPosStored = TRUE;

			pObj = pObj->pPrev;
		}
	}

	EXIT( "CalculateFramePosition" );
}

Void CalculateRealPosition( Void )
{
	ENTER;

	Vec_3				v;
	ObjectInstancePtr	pObj;

	pObj = gpObjects->pNext;
	while( pObj )
	{
		if (pObj->bPosStored)
		{

			if (pObj->obj.flags & XCGEN_OBJ_CAMRELATIVE)
			{
				Float	hrep;

				VectorSubtract( v, pObj->framepos, pObj->storepos );
				VectorAdd( pObj->obj.pos, pObj->obj.pos, v );

				DEBUGAssert( pObj->obj.repeat>0.0f );

				hrep = pObj->obj.repeat*0.5f;
				for (Int i=0;i<3;i++)
				{
					if (pObj->obj.pos[i]>hrep)
						pObj->obj.pos[i] -= pObj->obj.repeat;
					else if (pObj->obj.pos[i]<-hrep)
						pObj->obj.pos[i] += pObj->obj.repeat; 
				}
			}
			else
			{
				if ( !(pObj->obj.flags & XCGEN_OBJ_ATTACHED) )
				{
					VectorSubtract( v, pObj->framepos, pObj->storepos );
					VectorAdd( pObj->obj.pos, pObj->obj.pos, v );
				}
			}
		}
		pObj->bPosStored = FALSE;
		pObj = pObj->pNext;
	}

	EXIT( "CalculateFramePosition" );
}

Void CalibrateTimer( Void )
{
	ENTER;

	Int		cycles, ms;

	cycles = 0;
	ms = GetTickCount();
	TIMERBEGIN(cycles);
	while( GetTickCount()-ms < 1000 )
		;	
	TIMEREND(cycles);
	ms = GetTickCount() - ms;

	if (ms>0)
		gCycleCal = cycles/(ms*10);
	else
		gCycleCal = 1;

	EXIT( "CalibrateTimer" );
}

Void LoadIcons( Void )
{
	ENTER;

/*	UCharPtr		pData;

	pData = UCharPtr(GetData( "slow.tga" ));

	memcpy( &gIcons[0][0][0], pData+18, 32*32*sizeof(Short) );

	ReleaseData( pData );*/

	EXIT( "LoadIcons" );
}

Void DisplayIcon( Int type )
{
	ENTER;

/*	UCharPtr		pData;
	UCharPtr		pSrc;
	UCharPtr		pDst;
	DDSURFACEDESC	ddsd;
	Int				i;

	memset( &ddsd, 0, sizeof(DDSURFACEDESC) );
	ddsd.dwSize = sizeof(DDSURFACEDESC);

	gRenderSurf.pDDS->Lock( NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL );
	pData = UCharPtr(ddsd.lpSurface);

	pDst = UCharPtr(pData + (gViewport.width-32)*sizeof(Short));
	pSrc = UCharPtr(&gIcons[type][0][0]);

	for (i=0;i<32;i++)
	{
		memcpy( pDst, pSrc, sizeof(Short)*32 );
		pDst += ddsd.lPitch;
		pSrc += 32*sizeof(Short);
	}

	gRenderSurf.pDDS->Unlock( pData );*/

	EXIT( "DisplayIcon" );
}
//********************************************************************
//	-- Transform a vector from world space and project it into screen space
//********************************************************************
Void NonObjProjectVector( Vec_3 dst, Vec_3 v, FloatPtr pzInv )
{
	ENTER;

	Vec_3	v2;
    Float	zrecip;

	VectorSubtract( v2, v, gCamera.pos );
	MatVecMul( dst, gCamera.mat, v2 );

    zrecip = 1.0f / dst[2];
    v2[0] = dst[0] * zrecip * gViewport.maxscale + gViewport.xcenter;
    v2[1] = gViewport.ycenter - (dst[1] * zrecip * gViewport.maxscale);
	v2[2] = dst[2];

	*pzInv = zrecip;

	VectorCopy( dst, v2 );

	EXIT( "TransformProjectVector" );
}

Void TransformProjectVector( Vec_3 dst, Vec_3 v )
{
	ENTER;

	Vec_3	v2;
    Float	zrecip;

	MatVecMul( v2, gpStack->pObj->framemat, v );
	VectorAdd( dst, v2, gpStack->pObj->framepos );
	if (!(gpStack->pObj->obj.flags & XCGEN_OBJ_BACKGROUND))
		VectorSubtract( v2, dst, gCamera.pos );
	else
		VectorCopy( v2, dst );
	MatVecMul( dst, gCamera.mat, v2 );

    zrecip = 1.0f / dst[2];
    v2[0] = dst[0] * zrecip * gViewport.maxscale + gViewport.xcenter;
    v2[1] = gViewport.ycenter - (dst[1] * zrecip * gViewport.maxscale);
	v2[2] = dst[2];

	VectorCopy( dst, v2 );

	EXIT( "TransformProjectVector" );
}

//********************************************************************
//	-- Transform a vector from object space to world space
//********************************************************************
Void TransformWorldVector( Vec_3 dst, Vec_3 v )
{
	ENTER;

	//Vec_3	v2;

	MatVecMul( dst, gpStack->pObj->framemat, v );
	//VectorAdd( dst, v2, gpStack->pObj->framepos );

	EXIT( "TransformWorldVector" );
}

//********************************************************************
//	-- Transform a vector from object space to view space
//********************************************************************
Void TransformViewVector( Vec_3 dst, Vec_3 v )
{
	ENTER;

	Vec_3	v2;

	MatVecMul( v2, gpStack->pObj->framemat, v );
	VectorAdd( dst, v2, gpStack->pObj->framepos );
	if (!(gpStack->pObj->obj.flags & XCGEN_OBJ_BACKGROUND))
		VectorSubtract( v2, dst, gCamera.pos );
	else 
		VectorCopy( v2, dst );
	MatVecMul( dst, gCamera.mat, v2 );

	EXIT( "TransformVector" );
}

inline Void SetClipPlane( Vec_3 normal, FloatPtr plane )
{
	VectorCopy( plane, normal );
	plane[3] = 0.0001f;
}

Void BuildFrustum( Void )
{
	ENTER;

    Float	angle, s, c;
    Vec_3	normal;

    angle = Float(atan(2.0f / gViewport.fov * gViewport.maxscale / gViewport.xscale));
    s = Float(sin(angle));
    c = Float(cos(angle));

    // Left clip plane
    normal[0] = s;
    normal[1] = 0.0f;
    normal[2] = c;
    SetClipPlane(normal, gFrustum[0]);

    // Right clip plane
    normal[0] = -s;
    SetClipPlane(normal, gFrustum[1]);

    angle = Float(atan(2.0f / gViewport.fov * gViewport.maxscale / gViewport.yscale));
    s = Float(sin(angle));
    c = Float(cos(angle));

    // Bottom clip plane
    normal[0] = 0.0f;
    normal[1] = s;
    normal[2] = c;
    SetClipPlane(normal, gFrustum[2]);

    // Top clip plane
    normal[1] = -s;
    SetClipPlane(normal, gFrustum[3]);

	// Transform frustum into the world
	Int		i;
	Vec_3	v2;
	
	for ( i=0; i<4; i++ )
	{
		MatVecMulInv( v2, gCamera.mat, gFrustum[i] );

		VectorCopy( gFrustum[i], v2 );

		gFrustum[i][3] = DotProduct( gCamera.pos, v2 );
	}
	
	EXIT( "BuildFrustum" );
}
