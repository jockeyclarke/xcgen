/*******************************************************************
**    File: XCSprite.cpp
**      By: Paul L. Rowan    
** Created: 970522
**
** Description:
**
********************************************************************/

/*--- INCLUDES ----------------------------------------------------*/
#include "xcgenint.h"

#include <windows.h>

typedef RECT	Rect;
typedef RECT	*RectPtr;

inline Int	RectWidth( RectPtr r ) { return(r->right - r->left); }
inline Int	RectHeight( RectPtr r ) { return(r->bottom - r->top); }
/*--- CONSTANTS ---------------------------------------------------*/


/************************ IMPORT SECTION ***************************/
/*-------- FUNCTION PROTOTYPES ------------------------------------*/

/*-------- GLOBAL VARIABLES ---------------------------------------*/


/************************ PRIVATE SECTION **************************/
/*-------- FUNCTION PROTOTYPES ------------------------------------*/

/*-------- GLOBAL VARIABLES ---------------------------------------*/
extern "C"
{
			Void		DrawSpriteAsm( Void );

	extern Int			HStepUV;
	extern Int			HStepU;
	extern Int			VStepUV;
	extern Int			VStepU;
	extern ShortPtr	pSprSrc;
	extern ShortPtr	pSprDst;
	extern Int			Sprdstskip;
	extern Int			Sprshw, Sprshf, Sprsvw, Sprsvf;	// whole and frac amounts for src step
	extern Int			Sprw, Sprh;	
	extern Int			Sprctv, Sprcth;
};


/************************ MEMBER FUNCTIONS *************************/
Void DrawSprite( ObjectInstancePtr pObj )
{
	Int				frame;
	AnimSpritePtr	pSpr;
	SpriteFramePtr	pFrame;

	//DEBUGAssert( gCamera.tsCurrent >= pObj->obj.tsBegin );
	//return;
	if (gRenderSurf.bD3D)
		return;

	if ( gCamera.tsCurrent < pObj->obj.tsBegin )
		return;

	pSpr = pObj->pSprite;

	frame = gCamera.tsCurrent - pObj->obj.tsBegin;
	frame = frame*pSpr->speed/1000;
	frame %= pSpr->nFrames;

	if (pSpr->offFrames[frame]!=0)
	{
		pFrame = SpriteFramePtr(UCharPtr(pSpr) + pSpr->offFrames[frame]);

		Vec_3	pt, vpt;
		Vec_3	vp[4], sp[4];
		Rect	sr, dr, cdr, vr;
		Float	wl, wt, wr, wb;
		Int		i;

		VectorSubtract( pt, pObj->framepos, gCamera.pos );
		MatVecMul( vpt, gCamera.mat, pt );

		if (vpt[2]>0.0f)
		{
			//ww = pSpr->scale*pFrame->width;
			//wh = pSpr->scale*pFrame->height;
			wl = pSpr->scale*(pSpr->hotx-pFrame->offx);
			wt = pSpr->scale*(pSpr->hoty-pFrame->offy);
			wr = pSpr->scale*(pFrame->offx+pFrame->width-pSpr->hotx);
			wb = pSpr->scale*(pFrame->offy+pFrame->height-pSpr->hoty);

			// Y is reversed to produce correct orientation onscreen
			vp[0][0] = vpt[0] - (wl);	// left, top
			vp[0][1] = vpt[1] + (wt);
			vp[0][2] = vpt[2];
			vp[1][0] = vpt[0] + (wr);	// right, top
			vp[1][1] = vpt[1] + (wt);
			vp[1][2] = vpt[2];
			vp[2][0] = vpt[0] + (wr);	// right, bottom
			vp[2][1] = vpt[1] - (wb);
			vp[2][2] = vpt[2];
			vp[3][0] = vpt[0] - (wl);	// left, bottom
			vp[3][1] = vpt[1] - (wb);
			vp[3][2] = vpt[2];

			for (i=0;i<4;i++)
				XCGENProjectPoint( sp[i], vp[i] );
			
			SetRect( &sr, 0, 0, pFrame->width, pFrame->height );			// Source rect
			SetRect( &dr, Int(sp[0][0])+gViewport.xoff, 
						  Int(sp[0][1])+gViewport.yoff, 
						  Int(sp[2][0])+gViewport.xoff, 
						  Int(sp[2][1])+gViewport.yoff );				// Dest rect
			SetRect( &vr, gViewport.xoff, gViewport.yoff, 
						  gViewport.xoff + gViewport.width,
						  gViewport.yoff + gViewport.height );// Viewport rect
			IntersectRect( &cdr, &vr, &dr );	// Viewport clipped dest rect

			Sprw = RectWidth(&cdr);
			Sprh = RectHeight(&cdr);

			if (Sprw>0 && Sprh>0)
			{
			//	Int				dstskip;
				Int				shstep, svstep;		// 16.16 fixed point source step values
				Int				sleft, stop;	// 16.16 fixed point source sprite pixel begin
				Int				sright, sbottom;
				Float			sprtoscr;

				// These must be static because we can't acces the stack

				sprtoscr = Float(RectWidth(&sr))/Float(RectWidth(&dr));
							
				sleft = Int((Float(sr.left) + Float(cdr.left-dr.left)*sprtoscr)*0x10000);
				stop = Int((Float(sr.top) + Float(cdr.top-dr.top)*sprtoscr)*0x10000);
				sright = Int((Float(sr.right) - Float(dr.right-cdr.right)*sprtoscr)*0x10000);
				sbottom = Int((Float(sr.bottom) - Float(dr.bottom-cdr.bottom)*sprtoscr)*0x10000);

				shstep = Int(( (Float(sright-sleft)/0x10000)/Float(cdr.right-cdr.left) )*0x10000);
				svstep = Int(( (Float(sbottom-stop)/0x10000)/Float(cdr.bottom-cdr.top) )*0x10000);

				pSprDst = gRenderSurf.pSurf + (gRenderSurf.surfpitch*cdr.top) + (cdr.left);
				Sprdstskip = (gRenderSurf.surfpitch*sizeof(Short) - RectWidth(&cdr)*sizeof(Short)); ///sizeof(Short);

				Sprsvf = (svstep&0xffff)<<16;	// vertical frac step
				Sprsvw = svstep>>16;		// vertical whole step
				Sprshf = (shstep&0xffff)<<16;	// horizontal frac step
				Sprshw = shstep>>16;		// horizontal whole step
				
				pSprSrc = pFrame->sprData + ((stop>>16)*pFrame->rowBytes/sizeof(Short)) + (sleft>>16);

				HStepU = Sprshw*sizeof(Short);
				HStepUV = (Sprshw+1)*sizeof(Short);

				VStepU = Sprsvw*pFrame->rowBytes;
				VStepUV = (Sprsvw+1)*pFrame->rowBytes;

				Sprctv = stop<<16;
				Sprcth = sleft<<16;

				DrawSpriteAsm();
			}
		}
	}
}
