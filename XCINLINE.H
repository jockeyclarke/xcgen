/*******************************************************************
**    File: XCInline.h
**      By: Paul L. Rowan    
** Created: 970409
**
** Description: General Inline functions
**
********************************************************************/
#ifndef	_XCINLINE_H_
#define _XCINLINE_H_

#include "xcgen.h"
#include <math.h>

// Inline math routines are in XCMath.h
inline Void Swap( FloatPtr a, FloatPtr b )
{
	Float	t;

	t = *a;
	*a = *b;
	*b = t;
}

inline Float QuickDist( Vec_3 p0, Vec_3 p1 )
{
	Float	dx, dy, dz;

	dx = Float(fabs( p1[0] - p0[0] ));  // min
	dy = Float(fabs( p1[1] - p0[1] ));	 // med
	dz = Float(fabs( p1[2] - p0[2] ));  // max

	if (dx>dy)
		Swap( &dx, &dy );
	if (dy>dz)
		Swap( &dy, &dz );
	if (dx>dz)
		Swap( &dx, &dz );

	return( dz + dy*0.34375f + dx*0.25f );
}

inline Float QuickLength( Vec_3 p0 )
{
	Float	dx, dy, dz;

	dx = Float(fabs( p0[0] ));  // min
	dy = Float(fabs( p0[1] ));	 // med
	dz = Float(fabs( p0[2] ));  // max

	if (dx>dy)
		Swap( &dx, &dy );
	if (dy>dz)
		Swap( &dy, &dz );
	if (dx>dz)
		Swap( &dx, &dz );

	return( dz + dy*0.34375f + dx*0.25f );
}

#endif

