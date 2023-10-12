/*******************************************************************
**    File: XCParticle.cpp
**      By: Paul L. Rowan    
** Created: 970613
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
Void InitDistTable( Void );
/*-------- GLOBAL VARIABLES ---------------------------------------*/


/************************ MEMBER FUNCTIONS *************************/
XCGENTimestamp	gtsLast;

/************************ MEMBER FUNCTIONS *************************/
Void ProcessAttachedEmits( ObjectInstancePtr pObj, XCGENTimestamp stamp )
{
	ENTER;

	AttachedEmitPtr	pE;
	Vec_3			v;

	pE = pObj->pAttach;
	while( pE )
	{
		MatVecMul( v, pObj->framemat, pE->offset );
		VectorAdd( pE->emit.pos, v, pObj->framepos );

		EmitParticles( &pE->emit, stamp );
		pE = pE->pNext;
	}

	EXIT( "ProcessAttachedEmits" );
}

Void AttachEmitter( ObjectInstancePtr pObj, XCGENParticleEmitPtr pEmit, Vec_3 offset, XCGENTimestamp stamp )
{
	ENTER;

	AttachedEmitPtr	pE;
	Vec_3			v;

	// Allocate an attachment and copy the constant data
	pE = new AttachedEmit;

	VectorCopy( pE->offset, offset );

	memcpy( &pE->emit, pEmit, sizeof(XCGENParticleEmit) );

	pE->emit.sstamp = stamp;

	// Create first position relative to source object
	MatVecMul( v, pObj->framemat, pE->offset );
	if (pObj->bPosStored)
		VectorAdd( pE->emit.spos, v, pObj->framepos );
	else
		VectorAdd( pE->emit.spos, v, pObj->obj.pos );

	pE->pNext = pObj->pAttach;
	pObj->pAttach = pE;

	EXIT( "AttachEmitter" );
}

Void RemoveAttachedEmitters( ObjectInstancePtr pObj )
{
	ENTER;

	AttachedEmitPtr	pE, pN;

	pE = pObj->pAttach;
	while( pE )
	{
		pN = pE->pNext;
		delete pE;
		pE = pN;
	}

	EXIT( "RemoveAttachedEmitters" );
}

Void Particles( XCGENParticleDefPtr pDef, XCGENTimestamp stamp )
{
	ENTER;

	Int				i, j;
	ParticlePtr		pP;
	Vec_3			v, nvel;
	Float			s0, s1, dist;

	VectorCopy( nvel, pDef->vel );
	NormalizeVector( nvel );

	for (i=0;i<pDef->count;i++)
	{
		if (!gpFreeParts)
			return;
		pP = gpFreeParts;
		gpFreeParts = pP->pNext;
		pP->pNext = gpActiveParts;
		gpActiveParts = pP;

		// Determine position
		if (pDef->scatter>0.0f)
		{
			s0 = pDef->scatter * 0.5f;
			s1 = pDef->scatter/RAND_MAX;
			pP->pos[0] = (rand()*s1) - s0;
			pP->pos[1] = (rand()*s1) - s0;
			pP->pos[2] = (rand()*s1) - s0;
			// Squash
			if (pDef->squash>0.0f)
			{
				Float	sq;

				sq = pDef->squash;
				for (j=0;j<3;j++)
				{
					pP->pos[j] = (1.0f - pDef->vsquash[j]) * sq * pP->pos[j];
				}
			}
			VectorAdd( pP->pos, pP->pos, pDef->pos );
		}
		else
		{
			VectorCopy( pP->pos, pDef->pos );
		}

		// Get random velocity
		if (pDef->spread>0.0f)
		{
			s0 = pDef->spread * 0.5f;
			s1 = pDef->spread/RAND_MAX;
			v[0] = (rand()*s1) - s0;
			v[1] = (rand()*s1) - s0;
			v[2] = (rand()*s1) - s0;
			VectorCopy( pP->vel, v );
		}
		else
		{
			pP->vel[0] = 0.0f;
			pP->vel[1] = 0.0f;
			pP->vel[2] = 0.0f;
		}

		// Clamp spread 
		NormalizeVector( v, &dist );
		if (pDef->minspread>0.0f && dist<pDef->minspread)
		{
			VectorScale( pP->vel, v, pDef->minspread );
		}
		if (pDef->maxspread>0.0f && dist>pDef->maxspread)
		{
			VectorScale( pP->vel, v, pDef->maxspread );
		}

		// Squash
		if (pDef->squash>0.0f)
		{
			Float	sq;

			sq = pDef->squash;
			for (j=0;j<3;j++)
			{
				pP->vel[j] = (1.0f - pDef->vsquash[j]) * sq * pP->vel[j];
			}
		}

		// Add base velocity
		VectorAdd( pP->vel, pP->vel, pDef->vel );
		if (pDef->drag>0.0f)
		{
			s0 = (rand()*pDef->drag/RAND_MAX) - (pDef->drag*0.5f);	
			VectorScale( v, nvel, s0 );
			VectorSubtract( pP->vel, pP->vel, v );
		}

		pP->flags = pDef->flags;
		pP->tsBegin = stamp;
		pP->tsDie = stamp + Int((pDef->duration + (rand()*pDef->randlife/RAND_MAX) - (pDef->randlife*0.5f) ) * 1000.0f);
		pP->pColor = pDef->pRamp;
		pP->count = 0;
	}

	EXIT( "Particles" );
}

Void	EmitParticles( XCGENParticleEmitPtr pEmit, XCGENTimestamp stamp )
{
	ENTER;

	Int		i, num;
	Long	time;
	Long	st, dst;
	Long	drot;
	Float	scale, secs;
	Vec_3	dp, v, pos;

	time = stamp - pEmit->sstamp;
	secs = Float(time)*0.001f;

	pEmit->count &= 0xffff;	// Zero integer count
	pEmit->count += Int( (pEmit->rate*secs) * 0x10000 );

	num = pEmit->count >> 16;

	drot = Int( pEmit->drotation*secs/Float(num) );

	if (num>0)
	{
		st = pEmit->sstamp;
		dst = time/num;

		VectorSubtract( dp, pEmit->pos, pEmit->spos );
		scale = 1.0f/Float(num);
		VectorScale( dp, dp, scale );

		VectorCopy( pos, pEmit->spos );

		for (i=0;i<num;i++)
		{
			VectorCopy( pEmit->pDef->vel, pEmit->vel );
			VectorCopy( pEmit->pDef->pos, pos );

			if (pEmit->srotation!=0)
			{
				Float	c, s, t, ta, tb, tc;
				Float	sa, sb, sc;
				Mat_3	m;
				
				c = Cosine(pEmit->srotation);
				s = Sine(pEmit->srotation);
				t = 1.0f - c;

				ta = t * pEmit->axis[0] * pEmit->axis[1];
				tb = t * pEmit->axis[0] * pEmit->axis[2];
				tc = t * pEmit->axis[1] * pEmit->axis[2];

				sa = s * pEmit->axis[0];
				sb = s * pEmit->axis[1];
				sc = s * pEmit->axis[2];

				m[0][0] = t * pEmit->axis[0] * pEmit->axis[0] + c;
				m[0][1] = ta + sc;
				m[0][2] = tb - sb;
				m[1][0] = ta - sc;
				m[1][1] = t * pEmit->axis[1] * pEmit->axis[1] + c;
				m[1][2] = tc + sa;
				m[2][0] = tb + sb;
				m[2][1] = tc - sa;
				m[2][2] = t * pEmit->axis[2] * pEmit->axis[2] + c;
				
				MatVecMul( v, m, pEmit->spinvec );
				VectorAdd( pEmit->pDef->vel, pEmit->pDef->vel, v );

				MatVecMul( v, m, pEmit->spinpos );
				VectorAdd( pEmit->pDef->pos, pos, v );
			}

			Particles( pEmit->pDef, st );

			VectorAdd( pos, pos, dp );
			//VectorScale( v, pEmit->vel, (1.0f/(pEmit->rate*secs))*secs );
			//VectorAdd( dp, dp, v );

			st += dst;

			pEmit->srotation += drot;
		}
	}

	VectorCopy( pEmit->spos, pEmit->pos );
	pEmit->sstamp = stamp;

	EXIT( "EmitParticles" );
}

Void InitParticles( Void )
{
	ENTER;

	Int		i;

	gpActiveParts = NULL;

	gpFreeParts = &gParticles[0];

	for (i=0;i<MAX_PARTICLES-1;i++)
		gParticles[i].pNext = &gParticles[i+1];

	gParticles[MAX_PARTICLES-1].pNext = NULL;

	InitDistTable();

	EXIT( "InitParticles" );
}

Void ProcessParticles( XCGENTimestamp stamp )
{
	ENTER;

	ParticlePtr	pP, pL, pN;
	Float		vscale;
	Long		time;

	time = stamp - gtsLast; 
	vscale = Float(stamp - gtsLast) * 0.001f;

	pL = NULL;
	pP = gpActiveParts;
	while( pP )
	{
		pN = pP->pNext;

		pP->pos[0] += pP->vel[0]*vscale;
		pP->pos[1] += pP->vel[1]*vscale;
		pP->pos[2] += pP->vel[2]*vscale;

		pP->count += time;

		if (pP->count>(pP->tsDie-pP->tsBegin)/16)
		{
			pP->pColor++;
			pP->count = 0;
		}

		if (pP->tsDie<stamp)
		{
			if (pL)
				pL->pNext = pP->pNext;
			else
				gpActiveParts = pP->pNext;
			pP->pNext = gpFreeParts;
			gpFreeParts = pP;
		}
		else
		{
			pL = pP;
		}
		pP = pN;
	}

	gtsLast = stamp;

	EXIT( "ProcessParticles" );
}

#define MAXRADIUS	16
#define RADIUSMASK	15

Long		gDistTable[MAXRADIUS][MAXRADIUS];
UShort		gColorRamp[MAXRADIUS];
Long		gSqrtTable[MAXRADIUS][MAXRADIUS];
UChar		gAddTable[256][256];	

Void InitDistTable( Void )
{
	ENTER;
	
	Float	d;
	Int		i, j, dist;

	for (i=0;i<MAXRADIUS;i++)
		for (j=0;j<MAXRADIUS;j++)
		{
			if (i>=j)
			{
				d = sqrt( Float(i*i) - Float(j*j) );
				gSqrtTable[i][j] = Int(d);
			}
			d = sqrt( Float(i*i) + Float(j*j) );
			d /= Float(MAXRADIUS);
			d = 1.0f - d;
			d = d*d*d;
			d = 1.0f - d;
			d = d*MAXRADIUS;
			dist = Int(d);
			gDistTable[i][j] = min(max(dist,0),MAXRADIUS-1);
		}

	for (i=0;i<256;i++)
		for (j=0;j<256;j++)
		{
			gAddTable[j][i] = UChar(min((i&31) + (j&31),31));
		}

	EXIT( "InitDistTable" );
} 

Void CalculateColorRamp( Int pr, Int pg, Int pb )
{
	ENTER;

	Int		d, hr;
	Int		i, r, g, b;

	hr = MAXRADIUS/2;

	for (i=0;i<hr;i++)
	{
		d = i*65536/hr;
		r = min((31*65536-(31-pr)*d)>>16,31);
		g = min((31*65536-(31-pg)*d)>>16,31);
		b = min((31*65536-(31-pb)*d)>>16,31);

		if (gRenderSurf.b555)
			gColorRamp[i] = UShort( Int(r<<10) + Int(g<<5) + Int(b) );
		else
			gColorRamp[i] = UShort( Int(r<<11) + Int(g<<6) + Int(b) );
	}

	for (i=0;i<hr;i++)
	{
		d = i*65536/hr;
		d = 65536-d;
		r = min((pr*d)>>16,31);
		g = min((pg*d)>>16,31);
		b = min((pb*d)>>16,31);

		if (gRenderSurf.b555)
			gColorRamp[i+hr] = UShort( Int(r<<10) + Int(g<<5) + Int(b) );
		else
			gColorRamp[i+hr] = UShort( Int(r<<11) + Int(g<<6) + Int(b) );
	}

	EXIT( "CalculateColorRamp" );
}

Void DrawParticleHalo( ParticlePtr pP, Int sx, Int sy, Float zInv )
{
	ENTER;

	ShortPtr	pScr,  pRow;
	Int			y0, y1, height;
	Int			x0, x1, hwidth, h, x, y;
	Int			radius, rfact;
	Int			astep, count;
	Int			r, g, b, color, color2;

	radius = min(max(Int(zInv * gViewport.maxscale * 5.0f),2),MAXRADIUS-1);
	rfact = MAXRADIUS*65536/radius;

	astep = UNITANGLE/(4*radius);
	y0 = max( sy-radius, gViewport.yoff );
	y1 = min( sy+radius+1, gViewport.yoff+gViewport.height );
	height = y1 - y0;

	pRow = ShortPtr(gRenderSurf.pSurf+y0*gRenderSurf.surfpitch);

	color = Int(*pP->pColor);
	r = (color>>10)&31;
	g = (color>>5)&31;
	b = (color)&31;

	CalculateColorRamp( r, g, b );
	
	while( height-- )
	{
		h = abs(y0-sy);
		hwidth = gSqrtTable[radius][h];
		x0 = max( sx-hwidth, gViewport.xoff );
		x1 = min( sx+hwidth+1, gViewport.xoff+gViewport.width );

		pScr = pRow + x0;
		pRow += gRenderSurf.surfpitch;

		count = max(x1-x0,0);

		y = ((h * rfact)>>16)&RADIUSMASK;
		y0++;

		count /= 2;

		while( count-- )
		{
			color = *IntPtr(pScr);

			x = (x0 - sx) * rfact;
			if (x<0)
				x = -x - rfact;
			x0++;

			color2 = UInt(gColorRamp[gDistTable[(x>>16)&RADIUSMASK][y]]);

			x = (x0 - sx) * rfact;
			if (x<0)
				x = -x - rfact;
			x0++;

			color2 += UInt(gColorRamp[gDistTable[(x>>16)&RADIUSMASK][y]])<<16;

			if (gRenderSurf.b555)
			{
				_asm
				{
					mov		ebx, color
					mov		ecx, color2

					and		ebx, 0x7BDE7BDE
					and		ecx, 0x7BDE7BDE

					add		ebx, ecx

					test	ebx, 0x00000020
					jz		Blue
					or		ebx, 0x0000001F

				Blue:
					test	ebx, 0x00000400
					jz		Green
					or		ebx, 0x000003E0

				Green:
					test	ebx, 0x00008000
					jz		Red
					or		ebx, 0x00007C00

				Red:
					test	ebx, 0x00200000
					jz		Bluea
					or		ebx, 0x001F0000

				Bluea:
					test	ebx, 0x04000000
					jz		Greena
					or		ebx, 0x03E00000

				Greena:
					test	ebx, 0x80000000
					jz		Reda
					or		ebx, 0x7C000000

				Reda:
					mov		color, ebx

				}
			}
			else
			{
				_asm
				{
					mov		ebx, color
					mov		ecx, color2

					and		ebx, 0xF79EF79E
					and		ecx, 0xF79EF79E

					add		ebx, ecx
					rcr		edx, 1

					test	ebx, 0x00000020
					jz		Blue6
					or		ebx, 0x0000001F

				Blue6:
					test	ebx, 0x00000800
					jz		Green6
					or		ebx, 0x000007C0

				Green6:
					test	ebx, 0x00010000
					jz		Red6
					or		ebx, 0x0000F800

				Red6:
					test	ebx, 0x00200000
					jz		Blue6a
					or		ebx, 0x001F0000

				Blue6a:
					test	ebx, 0x08000000
					jz		Green6a
					or		ebx, 0x07C00000

				Green6a:
					test	edx, 0x80000000
					jz		Red6a
					or		ebx, 0xF8000000

				Red6a:
					mov		color, ebx

				}
			}

			*IntPtr(pScr) = color;

			pScr+=2;
		}
	}

	EXIT( "DrawParticleHalo" );
}

// Built tough for no-nonsense traversal
Char	gPartPat[8][17] = 
{
	// ox oy h  w  s  w  s  w  s  w  s  w  s  w  s  w  s 
	{  0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{  0, 0, 2, 2,-2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1,-1, 3, 3,-3, 3,-3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{  0,-1, 4, 2,-3, 4,-4, 4,-3, 2, 0, 0, 0, 0, 0, 0, 0 },
	{ -1,-2, 5, 3,-4, 5,-5, 5,-5, 5,-4, 3, 0, 0, 0, 0, 0 },
	{  0,-3, 7, 1,-3, 5,-5, 5,-6, 7,-6, 5,-5, 5,-3, 1, 0 },
	{ -1,-3, 7, 3,-4, 5,-6, 7,-7, 7,-7, 7,-6, 5,-4, 3, 0 },
	{ -2,-3, 7, 5,-6, 7,-7, 7,-7, 7,-7, 7,-7, 7,-6, 5, 0 }
};

Void DrawParticles( Void )
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
			pScr = ShortPtr(gRenderSurf.pSurf+sy*gRenderSurf.surfpitch+sx);
			pZB = ShortPtr(gRenderSurf.pZBuff+sy*gRenderSurf.zbuffpitch+sx);

			if ((zInv*65536)>(*pZB))
			{
				color = *pP->pColor;
				if (!gRenderSurf.b555)
				{
					// PLR - TODO: This may need to be a precalculated ramp
					color = ((color&0x7fe0)<<1) + (color&0x1f);
				}

				Int			radius, h, w, pitch;
				CharPtr		pPat;

				pitch = gRenderSurf.surfpitch;
				radius = min(max(Int(zInv * gViewport.maxscale * 0.2f),0),7);

				pPat = gPartPat[radius];

				pSC = pScr + (*pPat++);
				pSC += pitch * (*pPat++);

				h = *pPat++;

				if (pP->flags & XCGEN_PART_LIGHT)
				{
					// Draw Alpha halo
					DrawParticleHalo( pP, sx, sy, zInv );
				}
				else
				{
					// Draw base particle
					while( h-- )
					{
						w = *pPat++;
						while( w-- )
						{
							*(pSC++) = color;
						}
						pSC += pitch + *pPat++;
					}
				}
			}
		}
		pP = pP->pNext;
	}

	EXIT( "DrawParticles" );
}

Void InitDust( Void )
{
	Int		i;

	for (i=0;i<MAX_DUST;i++)
	{
		gDust[i][0] = Float(rand()*SPACING/RAND_MAX);
		gDust[i][1] = Float(rand()*SPACING/RAND_MAX);
		gDust[i][2] = Float(rand()*SPACING/RAND_MAX);
	}
}

Void DrawDust( Void )
{
	Vec_3			vp, wp;
	Float			scale, zInv;
	Int				sx, sy, i, j;
	Short			color;
	ShortPtr		pScr, pZB;
	Float			hrep;

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
				zInv = 1.0f/ss[2];

				pScr = gRenderSurf.pSurf + sy*gRenderSurf.surfpitch + sx;
				pZB = ShortPtr(gRenderSurf.pZBuff+sy*gRenderSurf.zbuffpitch+sx);

				if ((zInv*65536)>(*pZB))
				{
					*(pScr)	= color;
					if (vp[2]<SPACING/4)
					{
						*(pScr+1)	= color;
						*(pScr+gRenderSurf.surfpitch)	= color;
						*(pScr+gRenderSurf.surfpitch+1)	= color;
					}
				}
			}
		}	
	}
}

