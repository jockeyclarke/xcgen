/*******************************************************************
**    File: XCLineDraw.cpp
**      By: Paul L. Rowan    
** Created: 971014
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

// PLR - TODO:  These clip routines are awful.  
//				Fix it sometime.
Int ClipLineToPlane( Vec_3 p0, Vec_3 p1, Plane_3 plane, Vec_3 pp0, Vec_3 pp1 )
{
	ENTER;

	Float		dot0, dot1, scale;
	Bool		in0, in1;
	FloatPtr	pP0, pP1;	// InPt / OutPt

	VectorCopy( pp0, p0 );
	VectorCopy( pp1, p1 );

	dot0 = DotProduct( p0, plane );
	dot1 = DotProduct( p1, plane );

	in0 = (dot0>plane[3]);
	in1 = (dot1>plane[3]);

	// Both points behind plane
	if ( in0==FALSE && in1==FALSE )
		return( FALSE );

	// Both points in front of plane
	if ( in0==TRUE && in1==TRUE )
		return( TRUE );

	// Segment is split
	if ( in0==TRUE )
	{
		pP0 = p0;
		pP1 = p1;
	}
	else
	{
		pP0 = p1;		// Swap these around so p0 is always in
		pP1 = p0;
		VectorCopy( pp0, p1 );
		VectorCopy( pp1, p0 );

		Float t = dot0;
		dot0 = dot1;
		dot1 = t;
	}

    scale = (dot0 - plane[3]) / (dot0 - dot1);
    pp1[0] = pP0[0] + ((pP1[0] - pP0[0]) * scale);
    pp1[1] = pP0[1] + ((pP1[1] - pP0[1]) * scale);
    pp1[2] = pP0[2] + ((pP1[2] - pP0[2]) * scale);

	return( TRUE );

	EXIT( "ClipLineToPlane" );
}

Int ClipLineToFrustum( Vec_3 p0, Vec_3 p1, Vec_3 pp0, Vec_3 pp1 )
{
	ENTER;

    Int         i;

    for (i=0; i<3; i++)
    {
        if (!ClipLineToPlane( p0, p1,  gpStack->frustum[i].plane, pp0, pp1 ))
        {
            return( 0 );
        }
        VectorCopy( p0, pp0 );
		VectorCopy( p1, pp1 );
    }

    return( ClipLineToPlane( p0, p1, gpStack->frustum[i].plane, pp0, pp1 ) );

	EXIT( "ClipLineToFrustum" );
}

Int ClipLineToWorldFrustum( Vec_3 p0, Vec_3 p1, Vec_3 pp0, Vec_3 pp1 )
{
	ENTER;

    Int         i;

    for (i=0; i<3; i++)
    {
        if (!ClipLineToPlane( p0, p1,  gFrustum[i], pp0, pp1 ))
        {
            return( 0 );
        }
        VectorCopy( p0, pp0 );
		VectorCopy( p1, pp1 );
    }

    return( ClipLineToPlane( p0, p1, gFrustum[i], pp0, pp1 ) );

	EXIT( "ClipLineToFrustum" );
}

Void BRESENHAMXLine( Vec_3 s0, Vec_3 s1, Int NX, Int NY, UChar r, UChar g, UChar b ) 
{			
	Int		x, y, end;
	Short	c;
	Float	dx, dy, incrE, incrNE, d;

	s0[0] = Float(floor(s0[0]));
	s1[0] = Float(floor(s1[0]));

	s0[1] = Float(floor(s0[1]));
	s1[1] = Float(floor(s1[1]));
	
	dy = (NY)*(s1[1] - s0[1]);						
	dx = (NX)*(s1[0] - s0[0]);
	
	d = 2.0f * dy - dx;						
	incrE = 2.0f * dy;						
	incrNE = 2.0f * (dy-dx);	

	x = Int(s0[0]);						
	y = Int(s0[1])*gRenderSurf.surfpitch;
	end = Int(s1[0]);

	if (gRenderSurf.b555)
		c = ((Int(r)*32/256)<<10) + ((Int(g)*32/256)<<5) + (Int(b)*32/256);
	else
		c = ((Int(r)*32/256)<<11) + ((Int(g)*32/256)<<6) + (Int(b)*32/256);

	while( ((NX)>0 && x<end) || ((NX)<0 && x>end) )				
	{							
		*(gRenderSurf.pSurf + y + x) = c;	
		if (d<0.0f)							
		{									
			d += incrE;						
			x += (NX);	
		}										
		else								
		{									
			d += incrNE;					
			x += (NX);		
			y += (NY)*gRenderSurf.surfpitch;				
		}									
	}
}										

Void BRESENHAMYLine( Vec_3 s0, Vec_3 s1, Int NX, Int NY, UChar r, UChar g, UChar b ) 
{			
	Int		x, y, end;							
	Short	c;
	Float	dx, dy, incrE, incrNE, d;	
	
	s0[0] = Float(floor(s0[0]));
	s1[0] = Float(floor(s1[0]));

	s0[1] = Float(floor(s0[1]));
	s1[1] = Float(floor(s1[1]));
	
	dy = (NY)*(s1[1] - s0[1]);						
	dx = (NX)*(s1[0] - s0[0]);
	
	d = 2.0f * dx - dy;					
	incrE = 2.0f * dx;						
	incrNE = 2.0f * (dx-dy);
				
	x = Int(s0[0]);
	y = Int(s0[1])*gRenderSurf.surfpitch;
	end = Int(s1[1]);

	if (gRenderSurf.b555)
		c = ((Int(r)*32/256)<<10) + ((Int(g)*32/256)<<5) + (Int(b)*32/256);
	else
		c = ((Int(r)*32/256)<<11) + ((Int(g)*32/256)<<6) + (Int(b)*32/256);

	while( (((NY)>0 && y<end*gRenderSurf.surfpitch) || ((NY)<0 && y>end*gRenderSurf.surfpitch)))						
	{										
		*(gRenderSurf.pSurf + y + x) = c;	
		if (d<0.0f)							
		{									
			d += incrE;
			y += (NY)*gRenderSurf.surfpitch;				
		}										
		else								
		{									
			d += incrNE;					
			y += (NY)*gRenderSurf.surfpitch;				
			x += (NX);							
		}									
	}
}										


Void DrawLine( Vec_3 p0, Vec_3 p1, UChar r, UChar g, UChar b )
{
	ENTER;

	Vec_3		s0, s1;
	Vec_3		P0, P1;
	Vec_3	pP0, pP1;
	Float		dx, dy;
	Int			nx, ny;

	if (!ClipLineToFrustum( p0, p1, P0, P1 ))
		return;
	
	TransformProjectVector( s0, P0 );
	TransformProjectVector( s1, P1 );

	if (s0[0]<0.5f)
		s0[0] = 0.5f;
	if (s0[0]>=gViewport.width-1.5f)
		s0[0] = gViewport.width-1.5f;
	if (s0[1]<0.5f)
		s0[1] = 0.5f;
	if (s0[1]>=gViewport.height-1.5f)
		s0[1] = gViewport.height-1.5f;

	if (s1[0]<0.5f)
		s1[0] = 0.5f;
	if (s1[0]>=gViewport.width-1.5f)
		s1[0] = gViewport.width-1.5f;
	if (s1[1]<0.5f)
		s1[1] = 0.5f;
	if (s1[1]>=gViewport.height-1.5f)
		s1[1] = gViewport.height-1.5f;

	s0[0] += gViewport.xoff;
	s0[1] += gViewport.yoff;
	s1[0] += gViewport.xoff;
	s1[1] += gViewport.yoff;

	VectorCopy( pP0, s0 );
	VectorCopy( pP1, s1 );

	dx = s1[0] - s0[0];
	dy = s1[1] - s0[1];

	if (dy<0.0f)
		ny = -1;
	else 
		ny = 1;

	if (dx<0.0f)
		nx = -1;
	else
		nx = 1;

	if (fabs(dx)<fabs(dy))
		BRESENHAMYLine( s0, s1, nx, ny, r, g, b );
	else
		BRESENHAMXLine( s0, s1, nx, ny, r, g, b  );

	EXIT( "DrawLine" );
}

Void DrawRealWorldLine( Vec_3 p0, Vec_3 p1, UChar r, UChar g, UChar b )
{
	ENTER;

	Vec_3		s0, s1;
	Vec_3		P0, P1;
	Vec_3	pP0, pP1;
	Float		dx, dy, zInv;
	Int			nx, ny;

	if (!ClipLineToWorldFrustum( p0, p1, P0, P1 ))
		return;
	
	NonObjProjectVector( s0, P0, &zInv );
	NonObjProjectVector( s1, P1, &zInv );

	if (s0[0]<0.5f)
		s0[0] = 0.5f;
	if (s0[0]>=gViewport.width-1.5f)
		s0[0] = gViewport.width-1.5f;
	if (s0[1]<0.5f)
		s0[1] = 0.5f;
	if (s0[1]>=gViewport.height-1.5f)
		s0[1] = gViewport.height-1.5f;

	if (s1[0]<0.5f)
		s1[0] = 0.5f;
	if (s1[0]>=gViewport.width-1.5f)
		s1[0] = gViewport.width-1.5f;
	if (s1[1]<0.5f)
		s1[1] = 0.5f;
	if (s1[1]>=gViewport.height-1.5f)
		s1[1] = gViewport.height-1.5f;

	s0[0] += gViewport.xoff;
	s0[1] += gViewport.yoff;
	s1[0] += gViewport.xoff;
	s1[1] += gViewport.yoff;

	VectorCopy( pP0, s0 );
	VectorCopy( pP1, s1 );

	dx = s1[0] - s0[0];
	dy = s1[1] - s0[1];

	if (dy<0.0f)
		ny = -1;
	else 
		ny = 1;

	if (dx<0.0f)
		nx = -1;
	else
		nx = 1;

	if (fabs(dx)<fabs(dy))
		BRESENHAMYLine( s0, s1, nx, ny, r, g, b );
	else
		BRESENHAMXLine( s0, s1, nx, ny, r, g, b  );

	EXIT( "DrawLine" );
}

Void RenderWorldLine( Vec_3 p0, Vec_3 p1, UChar r, UChar g, UChar b )
{
	ENTER;

	Vec_3	P0, P1;

	VectorCopy( P0, p0 );
	VectorCopy( P1, p1 );

	DrawLine( P0, P1, r, g, b );

	EXIT( "RenderWorldLine" );
}

Void RenderRealWorldLine( Vec_3 p0, Vec_3 p1, UChar r, UChar g, UChar b )
{
	ENTER;

	Vec_3	P0, P1;

	VectorCopy( P0, p0 );
	VectorCopy( P1, p1 );

	DrawRealWorldLine( P0, P1, r, g, b );

	EXIT( "RenderWorldLine" );
}
