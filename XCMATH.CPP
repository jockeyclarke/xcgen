/*******************************************************************
**    File: XCMath.cpp
**      By: Paul L. Rowan      
** Created: 970409
**
** Description: general math routines
**
********************************************************************/

/*--- INCLUDES ----------------------------------------------------*/
#include "xcgen.h"
#include "xcmath.h"
#include <math.h>

/*--- CONSTANTS ---------------------------------------------------*/

#define PI			3.1415927F

/************************ IMPORT SECTION ***************************/
/*-------- FUNCTION PROTOTYPES ------------------------------------*/

/*-------- GLOBAL VARIABLES ---------------------------------------*/


/************************ PRIVATE SECTION **************************/
/*-------- FUNCTION PROTOTYPES ------------------------------------*/

/*-------- GLOBAL VARIABLES ---------------------------------------*/



/************************ MEMBER FUNCTIONS *************************/
Void MatMatMul( Mat_3 pEval, Mat_3 pM1, Mat_3 pM2 )
{
	_asm
	{
		mov		eax, pEval
		mov		ebx, pM1
		mov		ecx, pM2

			;pEval[0] = pM1[0]*pM2[0] + pM1[1]*pM2[3] + pM1[2]*pM2[6];
		fld		dword ptr [ebx+0]		;	p0
		fmul	dword ptr [ecx+0]		;	p0*x
		fld		dword ptr [ebx+4]		;	p1				p0*x
		fmul	dword ptr [ecx+12]		;	p1*y			p0*x
		fadd							;   p1*y+p0*x		p0*x
		fld		dword ptr [ebx+8]		;	p2				p1*y+p0*x		p0*x
		fmul	dword ptr [ecx+24]		;	p2*z			p1*y+p0*x		p0*x
		fadd							;	p2*z+p1*y+p0*x	p1*y+p0*x		p0*x	
		fstp	dword ptr [eax+0]		;	p1*y+p0*x		p0*x

			;pEval[1] = pM1[0]*pM2[1] + pM1[1]*pM2[4] + pM1[2]*pM2[7];
		fld		dword ptr [ebx+0]		;	p0
		fmul	dword ptr [ecx+4]		;	p0*x
		fld		dword ptr [ebx+4]		;	p1				p0*x
		fmul	dword ptr [ecx+16]		;	p1*y			p0*x
		fadd							;   p1*y+p0*x		p0*x
		fld		dword ptr [ebx+8]		;	p2				p1*y+p0*x		p0*x
		fmul	dword ptr [ecx+28]		;	p2*z			p1*y+p0*x		p0*x
		fadd							;	p2*z+p1*y+p0*x	p1*y+p0*x		p0*x	
		fstp	dword ptr [eax+4]		;	p1*y+p0*x		p0*x

			;pEval[2] = pM1[0]*pM2[2] + pM1[1]*pM2[5] + pM1[2]*pM2[8];
		fld		dword ptr [ebx+0]		;	p0
		fmul	dword ptr [ecx+8]		;	p0*x
		fld		dword ptr [ebx+4]		;	p1				p0*x
		fmul	dword ptr [ecx+20]		;	p1*y			p0*x
		fadd							;   p1*y+p0*x		p0*x
		fld		dword ptr [ebx+8]		;	p2				p1*y+p0*x		p0*x
		fmul	dword ptr [ecx+32]		;	p2*z			p1*y+p0*x		p0*x
		fadd							;	p2*z+p1*y+p0*x	p1*y+p0*x		p0*x	
		fstp	dword ptr [eax+8]		;	p1*y+p0*x		p0*x

			;pEval[3] = pM1[3]*pM2[0] + pM1[4]*pM2[3] + pM1[5]*pM2[6];
		fld		dword ptr [ebx+12]		;	p0
		fmul	dword ptr [ecx+0]		;	p0*x
		fld		dword ptr [ebx+16]		;	p1				p0*x
		fmul	dword ptr [ecx+12]		;	p1*y			p0*x
		fadd							;   p1*y+p0*x		p0*x
		fld		dword ptr [ebx+20]		;	p2				p1*y+p0*x		p0*x
		fmul	dword ptr [ecx+24]		;	p2*z			p1*y+p0*x		p0*x
		fadd							;	p2*z+p1*y+p0*x	p1*y+p0*x		p0*x	
		fstp	dword ptr [eax+12]		;	p1*y+p0*x		p0*x

			;pEval[4] = pM1[3]*pM2[1] + pM1[4]*pM2[4] + pM1[5]*pM2[7];
		fld		dword ptr [ebx+12]		;	p0
		fmul	dword ptr [ecx+4]		;	p0*x
		fld		dword ptr [ebx+16]		;	p1				p0*x
		fmul	dword ptr [ecx+16]		;	p1*y			p0*x
		fadd							;   p1*y+p0*x		p0*x
		fld		dword ptr [ebx+20]		;	p2				p1*y+p0*x		p0*x
		fmul	dword ptr [ecx+28]		;	p2*z			p1*y+p0*x		p0*x
		fadd							;	p2*z+p1*y+p0*x	p1*y+p0*x		p0*x	
		fstp	dword ptr [eax+16]		;	p1*y+p0*x		p0*x

			;pEval[5] = pM1[3]*pM2[2] + pM1[4]*pM2[5] + pM1[5]*pM2[8];
		fld		dword ptr [ebx+12]		;	p0
		fmul	dword ptr [ecx+8]		;	p0*x
		fld		dword ptr [ebx+16]		;	p1				p0*x
		fmul	dword ptr [ecx+20]		;	p1*y			p0*x
		fadd							;   p1*y+p0*x		p0*x
		fld		dword ptr [ebx+20]		;	p2				p1*y+p0*x		p0*x
		fmul	dword ptr [ecx+32]		;	p2*z			p1*y+p0*x		p0*x
		fadd							;	p2*z+p1*y+p0*x	p1*y+p0*x		p0*x	
		fstp	dword ptr [eax+20]		;	p1*y+p0*x		p0*x

			;pEval[6] = pM1[6]*pM2[0] + pM1[7]*pM2[3] + pM1[8]*pM2[6];
		fld		dword ptr [ebx+24]		;	p0
		fmul	dword ptr [ecx+0]		;	p0*x
		fld		dword ptr [ebx+28]		;	p1				p0*x
		fmul	dword ptr [ecx+12]		;	p1*y			p0*x
		fadd							;   p1*y+p0*x		p0*x
		fld		dword ptr [ebx+32]		;	p2				p1*y+p0*x		p0*x
		fmul	dword ptr [ecx+24]		;	p2*z			p1*y+p0*x		p0*x
		fadd							;	p2*z+p1*y+p0*x	p1*y+p0*x		p0*x	
		fstp	dword ptr [eax+24]		;	p1*y+p0*x		p0*x

			;pEval[7] = pM1[6]*pM2[1] + pM1[7]*pM2[4] + pM1[8]*pM2[7];
		fld		dword ptr [ebx+24]		;	p0
		fmul	dword ptr [ecx+4]		;	p0*x
		fld		dword ptr [ebx+28]		;	p1				p0*x
		fmul	dword ptr [ecx+16]		;	p1*y			p0*x
		fadd							;   p1*y+p0*x		p0*x
		fld		dword ptr [ebx+32]		;	p2				p1*y+p0*x		p0*x
		fmul	dword ptr [ecx+28]		;	p2*z			p1*y+p0*x		p0*x
		fadd							;	p2*z+p1*y+p0*x	p1*y+p0*x		p0*x	
		fstp	dword ptr [eax+28]		;	p1*y+p0*x		p0*x

			;pEval[8] = pM1[6]*pM2[2] + pM1[7]*pM2[5] + pM1[8]*pM2[8];
		fld		dword ptr [ebx+24]		;	p0
		fmul	dword ptr [ecx+8]		;	p0*x
		fld		dword ptr [ebx+28]		;	p1				p0*x
		fmul	dword ptr [ecx+20]		;	p1*y			p0*x
		fadd							;   p1*y+p0*x		p0*x
		fld		dword ptr [ebx+32]		;	p2				p1*y+p0*x		p0*x
		fmul	dword ptr [ecx+32]		;	p2*z			p1*y+p0*x		p0*x
		fadd							;	p2*z+p1*y+p0*x	p1*y+p0*x		p0*x	
		fstp	dword ptr [eax+32]		;	p1*y+p0*x		p0*x
	}

/*	pEval[0] = pM1[0]*pM2[0] + pM1[1]*pM2[3] + pM1[2]*pM2[6];
	pEval[1] = pM1[0]*pM2[1] + pM1[1]*pM2[4] + pM1[2]*pM2[7];
	pEval[2] = pM1[0]*pM2[2] + pM1[1]*pM2[5] + pM1[2]*pM2[8];
	pEval[3] = pM1[3]*pM2[0] + pM1[4]*pM2[3] + pM1[5]*pM2[6];
	pEval[4] = pM1[3]*pM2[1] + pM1[4]*pM2[4] + pM1[5]*pM2[7];
	pEval[5] = pM1[3]*pM2[2] + pM1[4]*pM2[5] + pM1[5]*pM2[8];
	pEval[6] = pM1[6]*pM2[0] + pM1[7]*pM2[3] + pM1[8]*pM2[6];
	pEval[7] = pM1[6]*pM2[1] + pM1[7]*pM2[4] + pM1[8]*pM2[7];
	pEval[8] = pM1[6]*pM2[2] + pM1[7]*pM2[5] + pM1[8]*pM2[8];
*/
}

Void MatVecMul( Vec_3 pEval, Mat_3 pM1, Vec_3 pV1 )
{
	_asm
	{
		mov		eax, pEval
		mov		ebx, pM1
		mov		ecx, pV1

										;	st(0)			st(1)			st(2)
		fld		dword ptr [ebx+0]		;	p0
		fmul	dword ptr [ecx+0]		;	p0*x
		fld		dword ptr [ebx+4]		;	p1				p0*x
		fmul	dword ptr [ecx+4]		;	p1*y			p0*x
		fadd							;   p1*y+p0*x		p0*x
		fld		dword ptr [ebx+8]		;	p2				p1*y+p0*x		p0*x
		fmul	dword ptr [ecx+8]		;	p2*z			p1*y+p0*x		p0*x
		fadd							;	p2*z+p1*y+p0*x	p1*y+p0*x		p0*x	
		fstp	dword ptr [eax+0]		;	p1*y+p0*x		p0*x

										;	st(0)			st(1)			st(2)
		fld		dword ptr [ebx+12]		;	p0
		fmul	dword ptr [ecx+0]		;	p0*x
		fld		dword ptr [ebx+16]		;	p1				p0*x
		fmul	dword ptr [ecx+4]		;	p1*y			p0*x
		fadd							;   p1*y+p0*x		p0*x
		fld		dword ptr [ebx+20]		;	p2				p1*y+p0*x		p0*x
		fmul	dword ptr [ecx+8]		;	p2*z			p1*y+p0*x		p0*x
		fadd							;	p2*z+p1*y+p0*x	p1*y+p0*x		p0*x	
		fstp	dword ptr [eax+4]		;	p1*y+p0*x		p0*x

										;	st(0)			st(1)			st(2)
		fld		dword ptr [ebx+24]		;	p0
		fmul	dword ptr [ecx+0]		;	p0*x
		fld		dword ptr [ebx+28]		;	p1				p0*x
		fmul	dword ptr [ecx+4]		;	p1*y			p0*x
		fadd							;   p1*y+p0*x		p0*x
		fld		dword ptr [ebx+32]		;	p2				p1*y+p0*x		p0*x
		fmul	dword ptr [ecx+8]		;	p2*z			p1*y+p0*x		p0*x
		fadd							;	p2*z+p1*y+p0*x	p1*y+p0*x		p0*x	
		fstp	dword ptr [eax+8]		;	p1*y+p0*x		p0*x

	}

/*	pEval->x = pM1[0]*pV1->x + pM1[1]*pV1->y + pM1[2]*pV1->z;
	pEval->y = pM1[3]*pV1->x + pM1[4]*pV1->y + pM1[5]*pV1->z;
	pEval->z = pM1[6]*pV1->x + pM1[7]*pV1->y + pM1[8]*pV1->z;
*/
}

// Multiply by the transpose of a matrix
Void MatVecMulInv( Vec_3 pEval, Mat_3 pM1, Vec_3 pV1 )
{
	_asm
	{
		mov		eax, pEval
		mov		ebx, pM1
		mov		ecx, pV1

										;	st(0)			st(1)			st(2)
		fld		dword ptr [ebx+0]		;	p0
		fmul	dword ptr [ecx+0]		;	p0*x
		fld		dword ptr [ebx+12]		;	p1				p0*x
		fmul	dword ptr [ecx+4]		;	p1*y			p0*x
		fadd							;   p1*y+p0*x		p0*x
		fld		dword ptr [ebx+24]		;	p2				p1*y+p0*x		p0*x
		fmul	dword ptr [ecx+8]		;	p2*z			p1*y+p0*x		p0*x
		fadd							;	p2*z+p1*y+p0*x	p1*y+p0*x		p0*x	
		fstp	dword ptr [eax+0]		;	p1*y+p0*x		p0*x

										;	st(0)			st(1)			st(2)
		fld		dword ptr [ebx+4]		;	p0
		fmul	dword ptr [ecx+0]		;	p0*x
		fld		dword ptr [ebx+16]		;	p1				p0*x
		fmul	dword ptr [ecx+4]		;	p1*y			p0*x
		fadd							;   p1*y+p0*x		p0*x
		fld		dword ptr [ebx+28]		;	p2				p1*y+p0*x		p0*x
		fmul	dword ptr [ecx+8]		;	p2*z			p1*y+p0*x		p0*x
		fadd							;	p2*z+p1*y+p0*x	p1*y+p0*x		p0*x	
		fstp	dword ptr [eax+4]		;	p1*y+p0*x		p0*x

										;	st(0)			st(1)			st(2)
		fld		dword ptr [ebx+8]		;	p0
		fmul	dword ptr [ecx+0]		;	p0*x
		fld		dword ptr [ebx+20]		;	p1				p0*x
		fmul	dword ptr [ecx+4]		;	p1*y			p0*x
		fadd							;   p1*y+p0*x		p0*x
		fld		dword ptr [ebx+32]		;	p2				p1*y+p0*x		p0*x
		fmul	dword ptr [ecx+8]		;	p2*z			p1*y+p0*x		p0*x
		fadd							;	p2*z+p1*y+p0*x	p1*y+p0*x		p0*x	
		fstp	dword ptr [eax+8]		;	p1*y+p0*x		p0*x

	}

/*	pEval->x = pM1[0]*pV1->x + pM1[1]*pV1->y + pM1[2]*pV1->z;
	pEval->y = pM1[3]*pV1->x + pM1[4]*pV1->y + pM1[5]*pV1->z;
	pEval->z = pM1[6]*pV1->x + pM1[7]*pV1->y + pM1[8]*pV1->z;
*/
}

Void RotMatHPR( Mat_3 pEval, Long h, Long p, Long r )
{
	Mat_3	mH, mP, mR, mE;

	MakeIdentity( mH ); 
	MakeIdentity( mP ); 
	MakeIdentity( mR ); 
	MakeIdentity( mE ); 

	mH[0][0] = Cosine(h);
	mH[0][2] = Sine(h);
	mH[2][0] = -Sine(h);
	mH[2][2] = Cosine(h);

	mP[1][1] = Cosine(p);
	mP[1][2] = Sine(p);
	mP[2][1] = -Sine(p);
	mP[2][2] = Cosine(p);

	mR[0][0] = Cosine(r);
	mR[0][1] = Sine(r);
	mR[1][0] = -Sine(r);
	mR[1][1] = Cosine(r);

	/*mH[0][0] = Cosine(h);
	mH[2][0] = Sine(h);
	mH[0][2] = -Sine(h);
	mH[2][2] = Cosine(h);

	mP[1][1] = Cosine(p);
	mP[2][1] = Sine(p);
	mP[1][2] = -Sine(p);
	mP[2][2] = Cosine(p);

	mR[0][0] = Cosine(r);
	mR[1][0] = Sine(r);
	mR[0][1] = -Sine(r);
	mR[1][1] = Cosine(r);*/

	MatMatMul( pEval, mE, mH );
	MatMatMul( mE, pEval, mP );
	MatMatMul( pEval, mE, mR );
}

Void RotMatRPH( Mat_3 pEval, Long r, Long p, Long h )
{
	Mat_3	mH, mP, mR, mE;

	MakeIdentity( mH ); 
	MakeIdentity( mP ); 
	MakeIdentity( mR ); 
	MakeIdentity( mE ); 

	mH[0][0] = Cosine(h);
	mH[0][2] = Sine(h);
	mH[2][0] = -Sine(h);
	mH[2][2] = Cosine(h);

	mP[1][1] = Cosine(p);
	mP[1][2] = Sine(p);
	mP[2][1] = -Sine(p);
	mP[2][2] = Cosine(p);

	mR[0][0] = Cosine(r);
	mR[0][1] = Sine(r);
	mR[1][0] = -Sine(r);
	mR[1][1] = Cosine(r);


	MatMatMul( pEval, mE, mR );
	MatMatMul( mE, pEval, mP );
	MatMatMul( pEval, mE, mH );
}

Void MatrixToQuaternion( Mat_3 inmat, Quat qu )
{
	Float tr, s;
	Mat_3	mat;

	VectorCopy( mat[0], inmat[0] );
	VectorCopy( mat[1], inmat[1] );
	VectorCopy( mat[2], inmat[2] );

	tr = mat[0][0] + mat[1][1] + mat[2][2];
	if( tr >= 0.0f )
	{
		s = sqrt( tr + 1.0f );
		qu[3] = s * 0.5f;
		s = 0.5f/s;
		qu[0] = (mat[2][1] - mat[1][2])*s;
		qu[1] = (mat[0][2] - mat[2][0])*s;
		qu[2] = (mat[1][0] - mat[0][1])*s;
	}
	else
	{
		Int h = 0;
		if( mat[1][1] > mat[0][0] )
			h = 1;
		if( mat[2][2] > mat[h][h] )
			h = 2;
		switch( h )
		{
#define caseMacro(i,j,k,I,J,K)	\
		case I:\
			s = sqrt( (mat[I][I] - (mat[J][J]+mat[K][K]))+ 1.0f );\
			qu[i] = s*0.5f; \
			s = 0.5f/s;	\
			qu[j] = (mat[I][J]+mat[J][I])*s; \
			qu[k] = (mat[K][I]+mat[I][K])*s;	\
			qu[3] = (mat[K][J]-mat[J][K])*s;	\
			break
		caseMacro(0,1,2,0,1,2);
		caseMacro(1,2,0,1,2,0);
		caseMacro(2,0,1,2,0,1);
#undef caseMacro
		}
	}
	Float	temp;

	temp = qu[0];
	qu[0] = qu[2];
	qu[2] = temp;
}

Long Arcsine( Float val )
{
	return( Long( Float(asin(val)) * UNITANGLE / (2*PI) ) );
}

Long Arccosine( Float val )
{
	return( Long( Float(acos(val)) * UNITANGLE / (2*PI) ) );
}

Float Sine( Long theta )
{
	Long	angle, quad;

	quad = theta & (ANGLEMASK - QUADMASK);
	angle = theta & QUADMASK;

	switch (quad)
	{
		case (0*QUADRANT):	return( gTrigTable[angle] );
		case (1*QUADRANT):	return( gTrigTable[QUADMASK-angle] );
		case (2*QUADRANT):	return( -gTrigTable[angle] );
		case (3*QUADRANT):	return( -gTrigTable[QUADMASK-angle] );
		default:
			//SMEDSAssert(0);
			return(Float(0.0));
	}
}

Float Cosine( Long theta )
{
	Long	angle, quad;

	quad = theta & (ANGLEMASK - QUADMASK);
	angle = theta & QUADMASK;

	switch (quad)
	{
		case (0*QUADRANT):	return( gTrigTable[QUADMASK-angle] );
		case (1*QUADRANT):	return( -gTrigTable[angle] );
		case (2*QUADRANT):	return( -gTrigTable[QUADMASK-angle] );
		case (3*QUADRANT):	return( gTrigTable[angle] );
		default:
			//SMEDSAssert(0);
			return(Float(0.0));
	}
}

Void InitTrigTable( Void )
{
	Long	i;

	for (i=0;i<QUADRANT;i++)
	{
		gTrigTable[i] = Float(sin( Double(i) * 2 * PI / Double(UNITANGLE) ));
	}
}


