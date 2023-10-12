/*******************************************************************
**    File: XCMath.h
**      By: Paul L. Rowan     
** Created: 970409
**
** Description: General Math routines  
**
********************************************************************/
#ifndef	_XCMATH_H_
#define _XCMATH_H_

#include <math.h>

#define UNITANGLE 	65536L
#define ANGLEMASK	0x0000FFFFL
#define QUADRANT	16384L
#define QUADMASK	0x00003FFFL

extern Float	gTrigTable[QUADRANT];

inline Float DotProduct( Vec_3 v0, Vec_3 v1 )
{
	return( v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2] );	
}

inline Void VectorAdd( Vec_3 dst, Vec_3 a, Vec_3 b )
{
	dst[0] = a[0] + b[0];
	dst[1] = a[1] + b[1];
	dst[2] = a[2] + b[2];
}

inline Void VectorSubtract( Vec_3 dst, Vec_3 a, Vec_3 b )
{
	dst[0] = a[0] - b[0];
	dst[1] = a[1] - b[1];
	dst[2] = a[2] - b[2];
}

inline Void VectorScale( Vec_3 dst, Vec_3 a, Float s )
{
	dst[0] = a[0]*s;
	dst[1] = a[1]*s;
	dst[2] = a[2]*s;
}

inline Void VectorCopy( Vec_3 dst, Vec_3 a )
{
	dst[0] = a[0];
	dst[1] = a[1];
	dst[2] = a[2];
}

inline Void PlaneCopy( Plane_3 dst, Plane_3 a )
{
	dst[0] = a[0];
	dst[1] = a[1];
	dst[2] = a[2];
	dst[3] = a[3];
}

inline Void CrossProd( Vec_3 dst, Vec_3 a, Vec_3 b )
{
	dst[0] = a[1]*b[2] - a[2]*b[1];
	dst[1] = a[2]*b[0] - a[0]*b[2];
	dst[2] = a[0]*b[1] - a[1]*b[0];
}

inline Void MakeIdentity( Mat_3 dst )
{
	dst[0][0] = 1.0f;  dst[0][1] = 0.0f;  dst[0][2] = 0.0f;
	dst[1][0] = 0.0f;  dst[1][1] = 1.0f;  dst[1][2] = 0.0f;
	dst[2][0] = 0.0f;  dst[2][1] = 0.0f;  dst[2][2] = 1.0f;
}

inline Void NormalizeVector( Vec_3 vec )
{
	Float	dist;

	dist = Float(sqrt(DotProduct(vec,vec)));
	VectorScale( vec, vec, 1.0f/dist );
}

inline Void NormalizeVector( Vec_3 vec, FloatPtr pDist )
{
	Float	dist;

	dist = Float(sqrt(DotProduct(vec,vec)));
	VectorScale( vec, vec, 1.0f/dist );
	*pDist = dist;
}

inline Void MatTrans( Mat_3 dst, Mat_3 a )
{
	dst[0][0] = a[0][0];
	dst[0][1] = a[1][0];
	dst[0][2] = a[2][0];
	dst[1][0] = a[0][1];
	dst[1][1] = a[1][1];
	dst[1][2] = a[2][1];
	dst[2][0] = a[0][2];
	dst[2][1] = a[1][2];
	dst[2][2] = a[2][2];
}

Void	MatMatMul( Mat_3 dst, Mat_3 a, Mat_3 b );
Void	MatVecMul( Vec_3 dst, Mat_3 a, Vec_3 b );
Void	MatVecMulInv( Vec_3 dst, Mat_3 a, Vec_3 b );
Void	RotMatHPR( Mat_3 dst, Long h, Long p, Long r );
Void	RotMatRPH( Mat_3 pEval, Long r, Long p, Long h );

Long	Arcsine( Float val );
Long	Arccosine( Float val );
Float	Sine( Long theta );
Float	Cosine( Long theta );
Void	InitTrigTable( Void );

// Quaternions
inline Void QuaternionAdd( Quat dst, Quat a, Quat b )
{
	dst[0] = a[0] + b[0];
	dst[1] = a[1] + b[1];
	dst[2] = a[2] + b[2];
	dst[3] = a[3] + b[3];
}

inline Void QuaternionSubtract( Quat dst, Quat a, Quat b )
{
	dst[0] = a[0] - b[0];
	dst[1] = a[1] - b[1];
	dst[2] = a[2] - b[2];
	dst[3] = a[3] - b[3];
}

inline Void QuaternionMultiply( Quat dst, Quat a, Quat b )
{
	Vec_3	v0, v1;

	CrossProd( v0, a, b );
	VectorScale( v1, a, b[3] );
	VectorAdd( v0, v0, v1 );
	VectorScale( v1, b, a[3] );
	VectorAdd( v0, v0, v1 );
	VectorCopy( dst, v0 );
	dst[3] = a[3]*b[3] - DotProduct(a,b);
}

inline Void QuaternionCopy( Quat dst, Quat a )
{
	dst[0] = a[0];
	dst[1] = a[1];
	dst[2] = a[2];
	dst[3] = a[3];
}

inline Float QuaternionNorm( Quat a )
{
	return( Float(sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]+a[3]*a[3])) );
}

inline Void NormalizeQuaternion( Quat a )
{
	Float	norm;

	norm = QuaternionNorm(a);

	a[0] /= norm;
	a[1] /= norm;
	a[2] /= norm;
	a[3] /= norm;
}

inline Void QuaternionToMatrix( Mat_3 m, Quat a )
{
   m[0][0] = a[3]*a[3] - a[0]*a[0] - a[1]*a[1] + a[2]*a[2];
   m[1][0] = 2.0F*(a[1]*a[2] + a[3]*a[0]);
   m[2][0] = 2.0F*(a[0]*a[2] - a[3]*a[1]);

   m[0][1] = 2.0F*(a[1]*a[2] - a[3]*a[0]);
   m[1][1] = a[3]*a[3] - a[0]*a[0] + a[1]*a[1] - a[2]*a[2];
   m[2][1] = 2.0F*(a[0]*a[1] + a[2]*a[3]);

   m[0][2] = 2.0F*(a[3]*a[1] + a[0]*a[2]);
   m[1][2] = 2.0F*(a[0]*a[1] - a[3]*a[2]);
   m[2][2] = a[3]*a[3] + a[0]*a[0] - a[1]*a[1] - a[2]*a[2];
}

inline Void RotateQuaternion( Quat q, Float roll, Float pitch, Float yaw )
{
	Float		e1dot, e2dot, e3dot, e4dot;

	Float	p = -pitch;
	Float	y = yaw;
	Float	r = -roll;

   /*-----------------------------------*/
   /* quaternion differential equations */
   /*-----------------------------------*/
   e1dot = (-q[1]*p + q[2]*y + q[3]*r)*0.5F;
   e2dot = ( q[0]*p + q[3]*y - q[2]*r)*0.5F;
   e3dot = ( q[3]*p - q[0]*y + q[1]*r)*0.5F;
   e4dot = (-q[2]*p - q[1]*y - q[0]*r)*0.5F;

   /*-----------------------*/
   /* integrate quaternions */
   /*-----------------------*/
   q[0] += e1dot;	// Could scale by SIM_TIME_STEP
   q[1] += e2dot;
   q[2] += e3dot;
   q[3] += e4dot;

   NormalizeQuaternion( q );
}

Void MatrixToQuaternion( Mat_3 inmat, Quat qu );

#endif

