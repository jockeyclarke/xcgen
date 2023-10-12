/*******************************************************************
**    File: XCRSurf.cpp
**      By: Paul L. Rowan     
** Created: 970409
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
#define DRAWFLAT	FALSE
/*-------- GLOBAL VARIABLES ---------------------------------------*/

/************************ MEMBER FUNCTIONS *************************/
Void CalculateGradients( SurfPtr pSurf, Vec_3 *pP, Vec_3 *pT  )
{
	ENTER;

	Float	mipscale;

	mipscale = 1.0f / Float(1 << pSurf->miplevel);

#if 1
	Int		i;
	Vec_3	verts[3];
	Float	oneoverz[3], uoverz[3], voverz[3];

	for (i=0;i<3;i++)
	{
		verts[i][0] = pP[i][0];
        verts[i][1] = pP[i][1];
		verts[i][2] = pP[i][2];
	}

	Float	oneoverdx = 1 / (((verts[1][0] - verts[2][0]) *
						(verts[0][1] - verts[2][1])) -
						((verts[0][0] - verts[2][0]) *
						(verts[1][1] - verts[2][1])));
	Float	oneoverdy = -oneoverdx;

	for (i=0;i<3;i++)
	{
		float const toneoverz = 1 / verts[i][2];
		oneoverz[i] = toneoverz;
		uoverz[i] = pT[i][0] * toneoverz;
		voverz[i] = pT[i][1] * toneoverz;
	}

	pSurf->zinvstepx = oneoverdx * (((oneoverz[1] - oneoverz[2]) *
					  (verts[0][1] - verts[2][1])) -
					  ((oneoverz[0] - oneoverz[2]) *
					  (verts[1][1] - verts[2][1])));
	pSurf->zinvstepy = oneoverdy * (((oneoverz[1] - oneoverz[2]) *
					  (verts[0][0] - verts[2][0])) -
					  ((oneoverz[0] - oneoverz[2]) *
					  (verts[1][0] - verts[2][0])));
	pSurf->zinv00 = oneoverz[2] -
				   verts[2][0]*pSurf->zinvstepx -
				   verts[2][1]*pSurf->zinvstepy;

	pSurf->uoverzstepx = oneoverdx * (((uoverz[1] - uoverz[2]) *
					  (verts[0][1] - verts[2][1])) -
					  ((uoverz[0] - uoverz[2]) *
					  (verts[1][1] - verts[2][1])));
	pSurf->uoverzstepy = oneoverdy * (((uoverz[1] - uoverz[2]) *
					  (verts[0][0] - verts[2][0])) -
					  ((uoverz[0] - uoverz[2]) *
					  (verts[1][0] - verts[2][0])));
	pSurf->uoverz00 = uoverz[2] -
					verts[2][0]*pSurf->uoverzstepx -
					verts[2][1]*pSurf->uoverzstepy;

	pSurf->voverzstepx = oneoverdx * (((voverz[1] - voverz[2]) *
					  (verts[0][1] - verts[2][1])) -
					  ((voverz[0] - voverz[2]) *
					  (verts[1][1] - verts[2][1])));
	pSurf->voverzstepy = oneoverdy * (((voverz[1] - voverz[2]) *
					  (verts[0][0] - verts[2][0])) -
					  ((voverz[0] - voverz[2]) *
					  (verts[1][0] - verts[2][0])));
	pSurf->voverz00 = voverz[2] -
					verts[2][0]*pSurf->voverzstepx -
					verts[2][1]*pSurf->voverzstepy;

	pSurf->uoff = 0;
	pSurf->voff = 0;

#else
	Float	t, d;
	Vec_3	v, v2, u2, uax, vax, org;

	MatVecMul( v, gpStack->pObj->framemat, pSurf->pFace->uaxis );
	MatVecMul( u2, gCamera.mat, v );

	MatVecMul( v, gpStack->pObj->framemat, pSurf->pFace->vaxis );
	MatVecMul( v2, gCamera.mat, v );

	d = 1.0f - DotProduct( u2, v2 );

	VectorScale( uax, u2, 1.0f/pSurf->pFace->uaxis[4] );
	VectorScale( vax, v2, 1.0f/pSurf->pFace->vaxis[4] );

	t = gViewport.maxscaleinv * mipscale;
	pSurf->uoverzstepx = uax[0] * t;
	pSurf->voverzstepx = vax[0] * t;

	t = gViewport.maxscaleinv * mipscale;
	pSurf->uoverzstepy = -uax[1] * t;
	pSurf->voverzstepy = -vax[1] * t;

	pSurf->uoverz00 = uax[2] * mipscale - gViewport.xcenter * pSurf->uoverzstepx -
			gViewport.ycenter * pSurf->uoverzstepy;
	pSurf->voverz00 = vax[2] * mipscale - gViewport.xcenter * pSurf->voverzstepx -
			gViewport.ycenter * pSurf->voverzstepy;

	/*pSurf->zinv00 = 1.0f;
	pSurf->zinvstepx = 0.0f;
	pSurf->zinvstepy = 0.0f;

	pSurf->uoverz00 = 256.0f;
	pSurf->uoverzstepx = 1.0f;
	pSurf->uoverzstepy = 1.0f;
	
	pSurf->voverz00 = 256.0f;
	pSurf->voverzstepx = 1.0f;
	pSurf->voverzstepy = -1.0f;*/
	
	MatVecMul( v, gpStack->pObj->framemat, gpStack->campos );
	MatVecMul( v2, gCamera.mat, v );
	
	t = 0x10000*mipscale/pSurf->pFace->uaxis[4];
	VectorScale( org, v2, mipscale );
	pSurf->uoff = Int(((DotProduct(org, uax) * 0x10000 + 0.5)) +
			((pSurf->pFace->uaxis[5] * 0x10000*mipscale) )
			+ pSurf->pFace->uaxis[3]*t);

	t = 0x10000*mipscale/pSurf->pFace->vaxis[4];
	VectorScale( org, v2, mipscale );
	pSurf->voff = Int(((DotProduct(org, vax) * 0x10000 + 0.5)) +
			((pSurf->pFace->vaxis[5] *0x10000*mipscale) )
			+ pSurf->pFace->vaxis[3]*t);
#endif

	pSurf->zinvstepx8 = pSurf->zinvstepx * 16;
	pSurf->uoverzstepx8 = pSurf->uoverzstepx * 16;
	pSurf->voverzstepx8 = pSurf->voverzstepx * 16;

	Long const Half = 0x8000;
	Long const PosModifier = Half;
	Long const NegModifier = Half - 1;

	float dUdXIndicator = pSurf->uoverzstepx * pSurf->zinv00 - pSurf->uoverz00 * pSurf->zinvstepx;

	if(dUdXIndicator > 0)
	{
		pSurf->dudxmod = PosModifier;
	}
	else
	if(dUdXIndicator < 0)
	{
		pSurf->dudxmod = NegModifier;
	}
	else
	{
		// dUdX == 0

		float dUdYIndicator = pSurf->uoverzstepy * pSurf->zinv00 - pSurf->uoverz00 * pSurf->zinvstepy;

		if(dUdYIndicator >= 0)
			pSurf->dudxmod = PosModifier;
		else
			pSurf->dudxmod = NegModifier;
	}

	float dVdXIndicator = pSurf->voverzstepx * pSurf->zinv00 - pSurf->voverz00 * pSurf->zinvstepx;

	if(dVdXIndicator > 0)
	{
		pSurf->dvdxmod = PosModifier;
	}
	else
	if(dVdXIndicator < 0)
	{
		pSurf->dvdxmod = NegModifier;
	}
	else
	{
		// dVdX == 0

		float dVdYIndicator = pSurf->voverzstepy * pSurf->zinv00 - pSurf->voverz00 * pSurf->zinvstepy;

		if(dVdYIndicator >= 0)
			pSurf->dvdxmod = PosModifier;
		else
			pSurf->dvdxmod = NegModifier;
	}

	EXIT( "CalculateGradients" );
}

Long	saveESP, saveEBP;

inline Void RenderColoredSpan( UCharPtr pScreen, SurfPtr pSurf, SpanPtr pSpan )
{
	ENTER;

 	Long	i, s, dsn, scrn, shd;

	i = pSpan->count;
	s = pSpan->s;
	dsn = pSpan->ds;	
	scrn = Long(pScreen);
	shd = Long(&pSurf->shades[0]);
	/*while( i-- )
	{
		*ShortPtr(pScreen) = shades[(s&0x0F0000)>>16];
		pScreen += 2;
		s += ds;
	}*/

	_asm{

		mov		saveEBP, ebp
		mov		saveESP, esp

		mov		ecx, i
		mov		ebx, s
		sar		ebx, 16		; ebx = s
		and		ebx, 0xf
		mov		edx, dsn
		sar		edx, 16		; edx = ds
		mov		esi, dsn	
		shl		esi, 16		; esi = dsfrac
		mov		edi, s
		shl		edi, 16		; edi = sfrac
		mov		esp, scrn
		mov		ebp, shd

		mov		ax, WORD PTR [ebp+ebx*2]
LoopIt:
		add		edi, esi
		adc		ebx, edx
		mov		WORD PTR [esp], ax
		add		esp, 2
		mov		ax, WORD PTR [ebp+ebx*2]
		dec		ecx
		jz		StopIt

		add		edi, esi
		adc		ebx, edx
		mov		WORD PTR [esp], ax
		add		esp, 2
		mov		ax, WORD PTR [ebp+ebx*2]
		dec		ecx
		jz		StopIt

		add		edi, esi
		adc		ebx, edx
		mov		WORD PTR [esp], ax
		add		esp, 2
		mov		ax, WORD PTR [ebp+ebx*2]
		dec		ecx
		jz		StopIt

		add		edi, esi
		adc		ebx, edx
		mov		WORD PTR [esp], ax
		add		esp, 2
		mov		ax, WORD PTR [ebp+ebx*2]
		dec		ecx
		jnz		LoopIt

StopIt:
		mov		ebp, saveEBP
		mov		esp, saveESP
	}

	EXIT( "RenderColoredSpan" );
}

Void GenerateSurfShades( SurfPtr pSurf )
{
	ENTER;

	Long	shade;
	Int		ir, ig, ib;
	Float	r, g, b, rr, gg, bb;

	r = Float(pSurf->pFace->r)/256.0f;
	g = Float(pSurf->pFace->g)/256.0f;
	b = Float(pSurf->pFace->b)/256.0f;

	if (gRenderSurf.b555)
	{
#if 0
		for (i=0;i<16;i++)
		{
			shades[i] = Short(	((pSurf->pFace->r*(23-i)/256)<<10) +
									((pSurf->pFace->g*(23-i)/256)<<5)  +
									(pSurf->pFace->b*(23-i)/256)        );
		}
#else
		/*for (shade=0;shade<8;shade++)
		{
			rr = 1.0f - ((1.0f-r) * Float(shade+4)/12.0f);
			gg = 1.0f - ((1.0f-g) * Float(shade+4)/12.0f);
			bb = 1.0f - ((1.0f-b) * Float(shade+4)/12.0f);

			ir = Int( rr * 32.0f );
			ig = Int( gg * 32.0f );
			ib = Int( bb * 32.0f );

			pSurf->shades[shade] = (ir<<10) + (ig<<5) + ib;
		}*/

		// Diffuse ramp
		for (shade=0;shade<16;shade++)
		{
			rr = r * Float(18-shade)/18.0f;
			gg = g * Float(18-shade)/18.0f;
			bb = b * Float(18-shade)/18.0f;

			ir = Int( rr * 32.0f );
			ig = Int( gg * 32.0f );
			ib = Int( bb * 32.0f );

			pSurf->shades[shade] = (ir<<10) + (ig<<5) + ib;
		}
#endif
	}
	else
	{
#if 0
		for (i=0;i<16;i++)
		{
			shades[i] = Short(	((pSurf->pFace->r*(23-i)/256)<<11) +
									((pSurf->pFace->g*(23-i)/256)<<6)  +
									(pSurf->pFace->b*(23-i)/256)        );
		}
#else
		/*for (shade=0;shade<8;shade++)
		{
			rr = 1.0f - ((1.0f-r) * Float(shade+4)/12.0f);
			gg = 1.0f - ((1.0f-g) * Float(shade+4)/12.0f);
			bb = 1.0f - ((1.0f-b) * Float(shade+4)/12.0f);

			ir = Int( rr * 32.0f );
			ig = Int( gg * 32.0f );
			ib = Int( bb * 32.0f );

			pSurf->shades[shade] = (ir<<11) + (ig<<6) + ib;
		}*/

		// Diffuse ramp
		for (shade=0;shade<16;shade++)
		{
			rr = r * Float(18-shade)/18.0f;
			gg = g * Float(18-shade)/18.0f;
			bb = b * Float(18-shade)/18.0f;

			ir = Int( rr * 32.0f );
			ig = Int( gg * 32.0f );
			ib = Int( bb * 32.0f );

			pSurf->shades[shade] = (ir<<11) + (ig<<6) + ib;
		}

#endif
	}

	EXIT( "GenerateSurfShades" );
}

Void WriteToZBuffer( Void )
{
	ENTER;

	SurfPtr			pSurf;
	SpanPtr			pSpan;
	Int				i, c;
	Int				zinv, dzinv;
	ShortPtr		pSrf;

	pSurf = &gSurfs[1];
	for (i=1;i<gnSurfaces;i++)
	{
		pSpan = pSurf->pSpans;
		while( pSpan )
		{
			pSrf = gRenderSurf.pZBuff + gRenderSurf.zbuffpitch*(pSpan->y+gViewport.yoff) + (pSpan->x+gViewport.xoff); 
			c = pSpan->count;
			zinv = Int(pSpan->zinv*0x10000);
			dzinv = Int(pSurf->zinvstepx*0x10000);
			/*_asm
			{
				mov		ebx, zinv
				mov		edx, dzinv
				mov		edi, pSrf
				mov		ecx, c
				test	edi, 2
				jnz		Aligned

				mov		[edi], bx
				add		ebx, edx
				add		edi, 2
				dec		ecx

			Aligned:
				shr		ecx, 1
				jz		EndIt

			LoopIt:
				mov		ax, bx
				add		ebx, edx
				shl		eax, 16
				mov		ax, bx
				add		ebx, edx
				rol		eax, 16
				mov		[edi], eax
				add		edi, 4
				dec		ecx
				jnz		LoopIt

			EndIt:
			}*/
			
			while( c-- )
			{
				*(pSrf++) = zinv;
				zinv += dzinv; 
			}
			pSpan = pSpan->pNext;
		}
		pSurf++;
	}

	EXIT( "WriteToZBuffer" );
}

Void DrawSurfaces( Void )
{
	ENTER;

	Int			i, vBits;
	SpanPtr		pSpan;
	SurfPtr		pSurf;
 
	pSurf = &gSurfs[1];	// Don't include the background surface

	TIMERBEGIN( gStats.tsSurfDraw );

	for (i=1;i<gnSurfaces;i++)	
	{
		if (pSurf->pSpans!=NULL)
		{
			if (pSurf->pTex==NULL)
			{
				GenerateSurfShades( pSurf );

				pSpan = pSurf->pSpans;
				while( pSpan )
				{
				#if DRAWFLAT
					memset (gRenderSurf.pSurf + (gRenderSurf.surfpitch * (pSpan->y+gViewport.yoff)) + (pSpan->x+gViewport.xoff),
						(Long(pSurf->pFace)&0xff0)>>4,
						pSpan->count*2);
				#else	
					RenderColoredSpan( UCharPtr(gRenderSurf.pSurf + (gRenderSurf.surfpitch * (pSpan->y+gViewport.yoff)) + (pSpan->x+gViewport.xoff)), pSurf, pSpan );
				#endif
					pSpan = pSpan->pNext;
				}
			}
			else
			{
				pSpan = pSurf->pSpans;
				while( pSpan )
				{
				#if DRAWFLAT
					memset (gRenderSurf.pSurf + (gRenderSurf.surfpitch * (pSpan->y+gViewport.yoff)) + (pSpan->x+gViewport.xoff),
						(Long(pSurf->pFace)&0xff0)>>4,
						pSpan->count*2);
				#else
 					gpScreen = UCharPtr(gRenderSurf.pSurf + (gRenderSurf.surfpitch * (pSpan->y+gViewport.yoff)) + (pSpan->x+gViewport.xoff));
					gpSpan = pSpan;
					gpSurf = pSurf;
					if (pSurf->pTex->flags & XCGEN_TEX_ANIMATED)
					{
						Int	frame;

						frame = ((gCamera.tsCurrent / pSurf->pTex->animtex.tsFrame)+(Int(pSurf->pObj)/sizeof(pSurf->pObj)))%pSurf->pTex->animtex.nFrames;

						gpTexture = UCharPtr(UCharPtr(pSurf->pTex) + sizeof(XCGENTexture) + frame*(1<<pSurf->pTex->width)*(1<<pSurf->pTex->height));
						pSurf->miplevel = 0;
					}
					else
					{
						gpTexture = UCharPtr(UCharPtr(pSurf->pTex) + pSurf->pTex->offMips[pSurf->miplevel]);
					}
					gpCTable = UCharPtr(&pSurf->pTex->cTable[0][0]);

					uBits = pSurf->pTex->width - pSurf->miplevel;
					vBits = pSurf->pTex->height - pSurf->miplevel;
					uShift = 16 - uBits;
					uvMask = (0xFF<<(32-uBits)) | (0xFF>>(8-vBits));

					SFixed = pSpan->s;
					DeltaS = pSpan->ds;

					if (DeltaS<0)
					{
						DeltaS += 16*0x10000;

						DEBUGAssert( DeltaS>0 );
					}
					
					DeltaS16 = DeltaS*16;

					EmitTexturedSpan();

					//gStats.pixels += pSpan->count;
				#endif

					pSpan = pSpan->pNext;
				}
			}
		}
		pSurf++;
	}

	if (gRenderSurf.bZBWrite)
		WriteToZBuffer();

	TIMEREND( gStats.tsSurfDraw );

	EXIT( "DrawSurfaces" );
}
