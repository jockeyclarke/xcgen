/*******************************************************************
**    File: XCD3DParticle.cpp
**      By: Paul Rowan    
** Created: 971231
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
D3DTLVERTEX		gPVerts[4];

/************************ MEMBER FUNCTIONS *************************/
Void D3DDrawDust( Void )
{
	ENTER;

	Vec_3			vp, wp;
	Float			scale, zInv;
	Int				sx, sy, i, j;
	Short			color;
	ShortPtr		pScr, pZB;
	Float			hrep;

	gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_TEXTUREHANDLE, NULL );

	if (gRenderSurf.b555)
		color = Short((10<<10) + (10<<5) + 8);
	else
		color = Short((10<<11) + (10<<6) + 8);
	
	scale = 1.0f/SPACING;
			hrep = SPACING/2;

	for (i=0;i<MAX_DUST;i++)
	{
		for (j=0;j<3;j++)
		{
			Float	c, a, p;

			a = gCamera.pos[j]*scale;
			c = (a - floor(a))*SPACING - hrep;
			p = gDust[i][j];
			if (p<c-hrep)
				p += SPACING;
			else if (p>c+hrep)
				p -= SPACING;
			wp[j] = p - c;
		}

		MatVecMul( vp, gCamera.mat, wp );
		if (vp[2]>0.0f)
		{
			Vec_3	ss;

			XCGENProjectPoint( ss, vp );
			sx = Int(ss[0])+gViewport.xoff;
			sy = Int(ss[1])+gViewport.yoff;

			if (sx>gViewport.xoff && sx<gViewport.xoff+gViewport.width-1 && 
				sy>gViewport.yoff && sy<gViewport.yoff+gViewport.height-1)
			{
				// D3D Render Code	
				gPVerts[0].rhw = 1.0f/ss[2];
				gPVerts[0].sz = 1.0f - min(max(gPVerts[0].rhw,0.0f),1.0f);
				gPVerts[0].color = (Int(255)<<24) + (Int(40)<<16) + (Int(40)<<8) + Int(32);
				gPVerts[0].specular = (255<<24);
				gPVerts[0].sx = Float(sx);
				gPVerts[0].sy = Float(sy);
				gRenderSurf.pD3DDev2->DrawPrimitive( D3DPT_POINTLIST, D3DVT_TLVERTEX, gPVerts, 1,  D3DDP_DONOTCLIP );
			}
		}	
	}

	EXIT( "D3DDrawDust" );
}

Void D3DDrawParticleHalo( ParticlePtr pP, Int sx, Int sy, Float zInv )
{
	ENTER;

	EXIT( "D3DDrawParticleHalo" );
}


Void D3DDrawParticle( ParticlePtr pP, Int sx, Int sy, Int radius, Float zInv )
{
	ENTER;

	HRESULT	res;
	Int		i, r, g, b;
	
	r = (((*pP->pColor)>>10)&31)<<3;
	g = (((*pP->pColor)>>5)&31)<<3;
	b = (((*pP->pColor))&31)<<3;

	if (radius==0)
	{
		gPVerts[0].rhw = zInv;
		gPVerts[0].sz = 1.0f - min(max(gPVerts[0].rhw,0.0f),1.0f);
		gPVerts[0].color = (128<<24) + (r<<16) + (g<<8) + b;
		gPVerts[0].sx = Float(sx);
		gPVerts[0].sy = Float(sy);
		res = gRenderSurf.pD3DDev2->DrawPrimitive( D3DPT_POINTLIST, D3DVT_TLVERTEX, gPVerts, 1,  D3DDP_DONOTCLIP );
	}
	else
	{
		for (i=0;i<4;i++)
		{
			gPVerts[i].rhw = zInv;
			gPVerts[i].sz = 1.0f - min(max(gPVerts[i].rhw,0.0f),1.0f);

			gPVerts[i].color = (128<<24) + (r<<16) + (g<<8) + b;
		}

		gPVerts[0].sx = Float(sx)-radius;
		gPVerts[1].sx = Float(sx)+radius+1;
		gPVerts[2].sx = Float(sx)+radius+1;
		gPVerts[3].sx = Float(sx)-radius;

		gPVerts[0].sy = Float(sy)-radius;
		gPVerts[1].sy = Float(sy)-radius;
		gPVerts[2].sy = Float(sy)+radius+1;
		gPVerts[3].sy = Float(sy)+radius+1;

		res = gRenderSurf.pD3DDev2->DrawPrimitive( D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, gPVerts, 4,  D3DDP_DONOTCLIP );
	}

	EXIT( "D3DDrawParticle" );
}




Void D3DDrawParticles( Void )
{
	ENTER;

	Int				sx, sy;
	Vec_3			ss;
	Float			zInv;
	ParticlePtr		pP;
	Short			color;
	ShortPtr		pScr, pSC, pZB;

	if (!gpActiveParts)
		return;

	gRenderSurf.pD3DDev2->SetRenderState( D3DRENDERSTATE_TEXTUREHANDLE, NULL );

	pP = gpActiveParts;
	while( pP )
	{
		NonObjProjectVector( ss, pP->pos, &zInv );
		sx = Int(ss[0])+gViewport.xoff;
		sy = Int(ss[1])+gViewport.yoff;

		if ( ss[2]>0.0f && 
			 sx>=gViewport.xoff+3 && sx<gViewport.xoff+gViewport.width-3 && 
			 sy>=gViewport.yoff+3 && sy<gViewport.yoff+gViewport.height-3 )
		{

			color = *pP->pColor;
			if (!gRenderSurf.b555)
			{
				// PLR - TODO: This may need to be a precalculated ramp
				color = ((color&0x7fe0)<<1) + (color&0x1f);
			}

			Int			radius, h, w, pitch;
			//CharPtr		pPat;

			//pitch = gRenderSurf.surfpitch;
			radius = min(max(Int(zInv * gViewport.maxscale * 0.1f),0),3);

			//pPat = gPartPat[radius];

			//pSC = pScr + (*pPat++);
			//pSC += pitch * (*pPat++);

			//h = *pPat++;

			if (pP->flags & XCGEN_PART_LIGHT)
			{
				// Draw Alpha halo
				D3DDrawParticleHalo( pP, sx, sy, zInv );
			}
			else
			{
				// D3D Render base particle
				D3DDrawParticle( pP, sx, sy, radius, zInv );
			}
		}
		pP = pP->pNext;
	}

	EXIT( "D3DDrawParticles" );
}
