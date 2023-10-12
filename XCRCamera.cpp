/*******************************************************************
**    File: XCRCamera.cpp
**      By: Paul L. Rowan     
** Created: 970416
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
Void Swap( Int a, Int b )
{
	Double				t;
	ObjectInstancePtr	lt;

	t = gObjDist[b];
	lt = gObjZOrder[b];
	gObjDist[b] = gObjDist[a];
	gObjZOrder[b] = gObjZOrder[a];
	gObjDist[a] = t;
	gObjZOrder[a] = lt;
}

Void QSort( Int i, Int j )
{
	Double	k;
	Long	ni, nj;

 	if (i>=j)
		return;
	else if (j==i+1)
	{
		if (gObjDist[i]>gObjDist[j])
			Swap(i,j);
		return;
	}
	else
	{
		k = gObjDist[i];	
		ni = i+1;
		nj = j;
		do
		{
			while(gObjDist[nj]>k && ni<nj)
				nj--;
			while(gObjDist[ni]<=k && ni<nj )
				ni++;
			if (ni<nj)
				Swap(ni,nj);
		}while(ni<nj);
		if (gObjDist[i]>gObjDist[nj])
			Swap(i,nj);
		QSort(ni,j);
		QSort(i,nj-1);
	}
}

#define DIST3D(x,y,z)	Double((Double(x)*Double(x))+(Double(y)*Double(y))+(Double(z)*Double(z)))

Bool CameraCanSee( ObjectInstancePtr pObj )
{
	ENTER;

	if (pObj->obj.flags & XCGEN_OBJ_BEAM)
	{
		Vec_3	vt, vh;

		VectorCopy( vt, pObj->pBeam->tail );
		VectorCopy( vh, pObj->pBeam->head );
		return( ClipLineToWorldFrustum( vt, vh, pObj->pBeam->shead, pObj->pBeam->stail ) );
	}
	else
	{
		Vec_3	ss, v;
		Int		sx, sy;
		Float	zInv, sradius, dist;

		if (pObj->obj.flags & XCGEN_OBJ_BACKGROUND)
		{
			VectorAdd( v, pObj->framepos, gCamera.pos );	
			dist = QuickDist( gCamera.pos, v );
			NonObjProjectVector( ss, v, &zInv );
		}
		else
		{
			dist = QuickDist( gCamera.pos, pObj->framepos );
			NonObjProjectVector( ss, pObj->framepos, &zInv );
		}

		VectorCopy( pObj->curproj, ss );

		sradius = pObj->objradius * zInv * gViewport.maxscale;

		pObj->curradius = sradius;

		if (pObj->objradius>dist)
			return( TRUE );

		sx = Int(ss[0]);
		sy = Int(ss[1]);

		if ( ss[2]<0.0f || 
			 sx<-sradius || sx>gViewport.width+sradius || 
			 sy<-sradius || sy>gViewport.height+sradius )
			return( FALSE );
		else 
			return( TRUE );
	}
	
	EXIT( "CameraCanSee" );
}

Void Sort( Void )
{
	ENTER;

	ObjectInstancePtr	pObj;

	gnObjects = 0;
	pObj = gpObjects->pNext;
	while( pObj )
	{
		if ( (gnObjects < MAX_OBJECTS) &&
			 (pObj->obj.flags & XCGEN_OBJ_VISIBLE) &&
			 !(pObj->obj.flags & XCGEN_OBJ_ATTACHED) &&	// Attached objects will be taken care of
			 CameraCanSee( pObj ) )
		{
			gObjZOrder[gnObjects] = pObj;

			if (pObj->obj.flags & XCGEN_OBJ_BACKGROUND)
			{
				gObjDist[gnObjects] = 9999999999.0f;
			}
			else
			{
				gObjDist[gnObjects] = DIST3D(	(pObj->framepos[0]-gCamera.pos[0]),
												(pObj->framepos[1]-gCamera.pos[1]),
												(pObj->framepos[2]-gCamera.pos[2]) );
			}
			gnObjects++;
		}
		pObj = pObj->pNext;
	}

	if (gnObjects>1)// && !gRenderSurf.bD3D)
	{
		QSort( 0,gnObjects-1 );
	}

	EXIT( "Sort" );
}

Bool ObjectJustAPixel( ObjectInstancePtr pObj )
{
	ENTER;

	Int	     sx, sy;
	ShortPtr pScr;

	if (fabs(pObj->curradius)<2.0f)
	{
		if (!gRenderSurf.bD3D)
		{
			sx = Int(min(max(pObj->curproj[0],0.5f),Float(gViewport.width)-0.5f));
			sy = Int(min(max(pObj->curproj[1],0.5f),Float(gViewport.height)-0.5f));

			sx += gViewport.xoff;
			sy += gViewport.yoff;

			pScr = ShortPtr(gRenderSurf.pSurf+sy*gRenderSurf.surfpitch+sx);

			*pScr = Short(0xFFFF);
		}
		
		return( TRUE );
	}

	return( FALSE );

	EXIT( "ObjectJustAPixel" );
}

inline Void DrawIt( ObjectInstancePtr pObj )
{
	if (pObj->obj.flags & XCGEN_OBJ_SPRITE)
	{
		DrawSprite( pObj );
	}
	else if (pObj->obj.flags & XCGEN_OBJ_BEAM)
	{
		RenderBeam( pObj );
	}
	else
	{
		if (ObjectJustAPixel(pObj))
			return;
		ClearEdgeLists();
		gnCurObject = 0;	// Reset object render stack
		RenderObject( pObj );
		if (!gRenderSurf.bD3D)
		{
			ScanEdges();
			DrawSurfaces();
		}
	}
}

Void ZOrderAndRender( Void )
{
	ENTER;

	Int					i;

	CalculateFramePosition();
	
	Sort();

	if (gRenderSurf.bD3D)
	{
		gRenderSurf.pD3DDev2->BeginScene();
		gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_SPECULARENABLE, FALSE );
		gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE );
		if (gRenderSurf.bAGP)
		{
			gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_MIPLINEAR );
			gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_MIPLINEAR );
		}
		else
		{
			gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR );
			gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR );
		}
		gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_ZENABLE, FALSE );
		gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );

		gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_BOTHSRCALPHA );
		//gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );

		gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, TRUE );

		/*gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_FOGENABLE, TRUE );

		gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_LINEAR );
		gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_FOGTABLESTART, 0.0f );
		gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_FOGTABLEEND, 1.0f );
		gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_FOGTABLEDENSITY, 0.3f );
		gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_FOGCOLOR, 0xff800000 );
*/
		gStats.nFullBuffs = 0;
		gStats.nTexLoads = 0;
	}
	
	// Background objects do not affect the Z-buffer
	gRenderSurf.bZBWrite = FALSE;
	for (i=gnObjects-1;i>=0;i--)
	{
		if (gObjZOrder[i]->obj.flags & XCGEN_OBJ_BACKGROUND)
			DrawIt( gObjZOrder[i] );
	}

	gRenderSurf.bZBWrite = TRUE;
	if (gRenderSurf.bD3D && gRenderSurf.bZBuff)
	{
		FlushD3DPolyBuffers();
		gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_ZENABLE, TRUE );
		gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
	}

	for (i=gnObjects-1;i>=0;i--)
	{
		if (!(gObjZOrder[i]->obj.flags & XCGEN_OBJ_BACKGROUND))
			DrawIt( gObjZOrder[i] );
	}

	gStats.tsParticles = 0;
	TIMERBEGIN( gStats.tsParticles );

	if (gRenderSurf.bD3D)
	{
		FlushD3DPolyBuffers();
		if (!(gViewport.flags & XCGEN_VPVLAG_NOPARTICLES))
			D3DDrawDust();

		D3DDrawParticles();
	}
	else
	{
		if (!(gViewport.flags & XCGEN_VPVLAG_NOPARTICLES))
			DrawDust();

		DrawParticles();
	}

	TIMEREND( gStats.tsParticles );

	if (gRenderSurf.bD3D)
	{
		gRenderSurf.pD3DDev2->EndScene();
	}
 
	EXIT( "ZOrderAndRender" );
}
/************************ MEMBER FUNCTIONS *************************/
Void RenderCamera( Void )
{
	ENTER;

	Long	tick;
	Short	OldFPUCW, FPUCW;

	_asm
	{
		fstcw	[OldFPUCW]						; Store copy of CW
		mov		ax, [OldFPUCW]					
		and		ax, NOT 0x300				; 24 bit precis.
		mov		[FPUCW], ax
		fldcw	[FPUCW]
	}

	tick = GetTickCount();

	gStats.tsEdgeSort = 0;
	gStats.tsSurfDraw = 0;
	gStats.nPolygons = 0;

	gStats.tsFrame = 0;
	TIMERBEGIN( gStats.tsFrame );

	// Build and transform the viewing frustum for this frame
	BuildFrustum();

	if (!gRenderSurf.bD3D)
	{
		LockRenderSurface();

	#ifdef _DEBUG
		UnlockRenderSurface();
	#endif
	}
	
	if (gRenderSurf.bD3D)
		UnlockRenderSurface();

	// Draw regions
	ObjectInstancePtr	pO, pO2, pO3;
	OccupiedRegionPtr	pR, pR2, pR3;
	Vec_3	v0, v1;
	
	if (gViewport.flags & XCGEN_VIEW_BBOXREGIONS)
	{
		pR = gRegionHead[0].pNext;
		while( pR )
		{
			pR2 = gRegionHead[1].pNext;
			while( pR2 )
			{
				pR3 = gRegionHead[2].pNext;
				while( pR3 )
				{
					Bool	bFound = FALSE;

					pO = pR->pObj;
					while( pO )
					{
						pO2 = pR2->pObj;
						while( pO2 && Int(pO2)<=Int(pO) )
						{
							pO3 = pR3->pObj;
							while( pO3 && Int(pO3)<=Int(pO) )
							{
								if (pO==pO2 && pO2==pO3)
									bFound = TRUE;
								pO3 = pO3->pGroup[2];
							}
							pO2 = pO2->pGroup[1];
						}
						pO = pO->pGroup[0];
					}

					if (bFound)
					{
						v0[0] = pR->begin;
						v1[0] = pR->end;
						v0[1] = pR2->begin;
						v1[1] = pR2->end;
						v0[2] = pR3->begin;
						v1[2] = pR3->end;

						DrawBox( v0, v1, 64, 64, 64, FALSE );
					}

					pR3 = pR3->pNext;
				}
				pR2 = pR2->pNext;
			}
			pR = pR->pNext;
		}
	}

	ZOrderAndRender();

	tick = GetTickCount()-tick;

	if (tick>MAX_FRAME_TIME)
	{
		DisplayIcon( ICON_SLOW );
	}

#ifndef _DEBUG
	if (!gRenderSurf.bD3D)
		UnlockRenderSurface();
#endif

	TIMEREND( gStats.tsFrame );
	TIMEREND( gStats.tsLastFrame );

	if (gViewport.flags & XCGEN_VIEW_SHOWSTATS)
		DisplayStats();

	gStats.tsLastFrame = 0;
	TIMERBEGIN( gStats.tsLastFrame );
	_asm
	{
		fldcw	[OldFPUCW]
	}

	EXIT( "RenderCamera" );
}

