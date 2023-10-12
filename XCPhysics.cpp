/*******************************************************************
**    File: XCPhysics.cpp
**      By: Paul L. Rowan     
** Created: 970516
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
#define POSGRANULAR			0.00000001f

/************************ PRIVATE SECTION **************************/
/*-------- FUNCTION PROTOTYPES ------------------------------------*/
#define INFINITY			9999999999.0f
#define FEATHERWEIGHT		0.00001f
#define MINTIMESLICE		0.0001f		// Minimum time slice
#define HULL_DIST_EPSILON	0.0f

/*-------- GLOBAL VARIABLES ---------------------------------------*/
Float				gEndEvent;
XCGENTimestamp		gLastMotionTick, gThisMotionTick;
Vec_3				gHNorm, gHCNorm, gHIPt;
Float				gHMinT;
Bool				gHSolid;


MeshHullNode	gBBoxHull[6] = 
{
	{ 0, 0, 0, { 1.0f, 0.0f, 0.0f, 0.0f }, { Long(HULL_NODE_EMPTY), Long(&gBBoxHull[1]) } },
	{ 0, 0, 0, { -1.0f, 0.0f, 0.0f, 0.0f }, { Long(HULL_NODE_EMPTY), Long(&gBBoxHull[2]) } },
	{ 0, 0, 0, { 0.0f, 1.0f, 0.0f, 0.0f }, { Long(HULL_NODE_EMPTY), Long(&gBBoxHull[3]) } },
	{ 0, 0, 0, { 0.0f, -1.0f, 0.0f, 0.0f }, { Long(HULL_NODE_EMPTY), Long(&gBBoxHull[4]) } },
	{ 0, 0, 0, { 0.0f, 0.0f, 1.0f, 0.0f }, { Long(HULL_NODE_EMPTY), Long(&gBBoxHull[5]) } },
	{ XCGEN_HULL_LEAF|XCGEN_HULL_SOLID, 0, 0, { 0.0f, 0.0f, -1.0f, 0.0f }, { Long(HULL_NODE_EMPTY), Long(HULL_NODE_SOLID) } }
};
/************************ MEMBER FUNCTIONS *************************/
Void ApplyThrust( Float time )
{
	ENTER;

	ObjectInstancePtr	pObj;
	Vec_3				thr;
	
	pObj = gpObjects->pNext;
	while( pObj )
	{
		// Dampen velocity
		if (!(pObj->obj.flags & XCGEN_OBJ_NODAMPVEL)) 
			VectorScale( pObj->obj.velocity, pObj->obj.velocity, pow(0.1f,time) );
		
		VectorScale( thr, pObj->obj.thrust, time );
		VectorAdd( pObj->obj.velocity, pObj->obj.velocity, thr );

		pObj = pObj->pNext;
	}

	EXIT( "ApplyThrust" );
}

Void ApplyAngularMomentum( Float time )
{
	ENTER;

	ObjectInstancePtr	pObj;
	
	pObj = gpObjects->pNext;
	while( pObj )
	{
		if (pObj->obj.angvel>0.0f)
		{
			ApplyRotation( pObj, pObj->obj.rotaxis[0]*time, pObj->obj.rotaxis[1]*time, pObj->obj.rotaxis[2]*time );
		}
		pObj = pObj->pNext;
	}

	EXIT( "ApplyAngularMomentum" );
}

Bool PointInHull( MeshHullNodePtr pNode, Vec_3 p0 )
{
	ENTER;

	MeshHullNodePtr	pN;
	Float			d;
	Int				side;

	if (pNode==HULL_NODE_SOLID)
		return( TRUE );
	else if (pNode==HULL_NODE_EMPTY) 
		return( FALSE );

	pN = pNode;
	while( 1 )
	{
		d = DotProduct( pN->plane, p0 ) - pN->plane[3];
		side = (d<0.0f);
		
		if (pN->pChildren[side]==HULL_NODE_SOLID)
			return( TRUE );
		else if (pN->pChildren[side]==HULL_NODE_EMPTY)
			return( FALSE );
		else
			pN = pN->pChildren[side];
	}

	EXIT( "PointInHull" );
}

MeshHullNodePtr	gpRoot;

Bool CheckHullCollision( MeshHullNodePtr pNode, Vec_3 p0, Vec_3 p1, Float t0, Float t1 )
{
	ENTER;

	Int		side;
	Float	d0, d1, t, frac;
	Vec_3	mid;

	DEBUGAssert( pNode!=NULL );

	if (pNode==HULL_NODE_EMPTY)
	{
		gHSolid = FALSE;
		return( TRUE );
	}
	else if (pNode==HULL_NODE_SOLID)
	{
		return( TRUE );
	}

	d0 = DotProduct( p0, pNode->plane ) - pNode->plane[3];
	d1 = DotProduct( p1, pNode->plane ) - pNode->plane[3];

	// If it completely misses this node, continue on the 
	//  appropriate child
	if (d0>=0.0f && d1>=0.0f)
		return( CheckHullCollision( pNode->pChildren[0], p0, p1, t0, t1 ) );
	else if (d0<0.0f && d1<0.0f)
		return( CheckHullCollision( pNode->pChildren[1], p0, p1, t0, t1 ) );

	if (d0<0.0f)
		t = (d0 + HULL_DIST_EPSILON) / (d0-d1);
	else
		t = (d0 - HULL_DIST_EPSILON) / (d0-d1);

	if (t<0.0f)
		t = 0.0f;
	if (t>1.0f)
		t = 1.0f;

	mid[0] = p0[0] + t * (p1[0]-p0[0]);
	mid[1] = p0[1] + t * (p1[1]-p0[1]);
	mid[2] = p0[2] + t * (p1[2]-p0[2]);

	frac = t0 + t * (t1 - t0);

	side = (d0<0.0f);

	if (!CheckHullCollision( pNode->pChildren[side], p0, mid, t0, frac ))
		return( FALSE );

	if (!PointInHull( pNode->pChildren[!side], mid ))
		return (CheckHullCollision( pNode->pChildren[!side], mid, p1, frac, t1 ));

	if (gHSolid)
		return( FALSE );

	while (PointInHull( gpRoot, mid ))
	{
		t -= 0.1f;
		/*if (t<0.0f)
		{
			VectorCopy( gHNorm, pNode->plane );
			if (side)
			{
				gHNorm[0] = -gHNorm[0];
				gHNorm[1] = -gHNorm[1];
				gHNorm[2] = -gHNorm[2];
			}
			VectorCopy( gHIPt, mid );
			gHMinT = MINTIMESLICE;
			return( FALSE );
		}*/
		mid[0] = p0[0] + t * (p1[0]-p0[0]);
		mid[1] = p0[1] + t * (p1[1]-p0[1]);
		mid[2] = p0[2] + t * (p1[2]-p0[2]);

		frac = t0 + t * (t1 - t0);
	}

	// Impact
	VectorCopy( gHNorm, pNode->plane );
	if (side)
	{
		gHNorm[0] = -gHNorm[0];
		gHNorm[1] = -gHNorm[1];
		gHNorm[2] = -gHNorm[2];
	}
	VectorCopy( gHIPt, mid );
	gHMinT = frac;

	return( FALSE );

	EXIT( "CheckHullCollision" );
}

#define HULL_UNSTICK	POSGRANULAR

Bool HullUnstick( MeshHullNodePtr pNode, Vec_3 p0, Vec_3 dv )
{
	ENTER;

	Vec_3	org;
	Float		x, y, z;

	VectorCopy( org, p0 );

	for ( z=-HULL_UNSTICK; z<=HULL_UNSTICK; z+=HULL_UNSTICK )
	{
		for ( y=-HULL_UNSTICK; y<=HULL_UNSTICK; y+=HULL_UNSTICK )
		{
			for ( x=-HULL_UNSTICK; x<=HULL_UNSTICK; x+=HULL_UNSTICK )
			{
				p0[0] = org[0] + dv[0]>0.0f?x:-x;
				p0[1] = org[1] + dv[1]>0.0f?y:-y;
				p0[2] = org[2] + dv[2]>0.0f?z:-z;
				if (!PointInHull( pNode, p0 ))
					return( TRUE );
			}
		}
	}

	VectorCopy( p0, org );

	return( FALSE );

	EXIT( "HullUnstick" );
}

Void MakeBBoxHull( Vec_3 pos, Float radius )
{
	ENTER;

	Int		i;

	for (i=0;i<3;i++)
	{
		gBBoxHull[2*i].plane[3] = pos[i]+radius;
		gBBoxHull[(2*i)+1].plane[3] = -pos[i]+radius;
	}

	EXIT( "MakeBBoxHull" );
}

Void BeamHullCollide( ObjectInstancePtr pObjS, ObjectInstancePtr pObjB )
{
	ENTER;

	pObjB->pBeam->vhead[0] = 0.0f;
	pObjB->pBeam->vhead[1] = 0.0f;
	pObjB->pBeam->vhead[2] = 0.0f;
	VectorCopy( pObjB->pBeam->head, pObjS->collidePos );
	
	EXIT( "BeamHullCollide" );
}

Void HullCollide( ObjectInstancePtr pObjS, ObjectInstancePtr pObjT )
{
	ENTER;

	// Update the collided objects' velocities
	Vec_3	norm, v0, vAB;
	Float	massS, massT, j, e, d;

	e = 0.5f;		// Elasticity ( 0.0 = thud,  1.0 = superball )

	// Collision normal 
	VectorCopy( norm, pObjS->collideNorm );
	norm[0] = -norm[0];
	norm[1] = -norm[1];
	norm[2] = -norm[2];

	// Approximate the object's mass by using it's volume
	// Assumes all objects are of the same density
	if (pObjS->obj.flags & XCGEN_OBJ_MASSLESS)
		massS = FEATHERWEIGHT;
	else
		massS = (pObjS->pMesh->minmaxs[3] - pObjS->pMesh->minmaxs[0]) *
				(pObjS->pMesh->minmaxs[4] - pObjS->pMesh->minmaxs[1]) *
				(pObjS->pMesh->minmaxs[5] - pObjS->pMesh->minmaxs[2]);

	if (pObjT->obj.flags & XCGEN_OBJ_MASSLESS)
		massT = FEATHERWEIGHT;
	else
		massT = (pObjT->pMesh->minmaxs[3] - pObjT->pMesh->minmaxs[0]) *
				(pObjT->pMesh->minmaxs[4] - pObjT->pMesh->minmaxs[1]) *
				(pObjT->pMesh->minmaxs[5] - pObjT->pMesh->minmaxs[2]);

	//massS = 1.0f;
	//massT = 1.0f;

	// Calculate Net Velocity at point of impact
	VectorSubtract( vAB, pObjS->obj.velocity, pObjT->obj.velocity );

	VectorScale( v0, vAB, -(1.0f + e) );
	j = DotProduct( v0, norm );

	VectorScale( v0, norm, (1.0f/massS + 1.0f/massT) );
	j /= DotProduct( v0, norm );

	d = j/massS;
	pObjS->obj.velocity[0] += d*norm[0];// - norm[0]*3.0f;
	pObjS->obj.velocity[1] += d*norm[1];// - norm[1]*3.0f;
	pObjS->obj.velocity[2] += d*norm[2];// - norm[2]*3.0f;

	d = j/massT;
	pObjT->obj.velocity[0] -= d*norm[0];// - norm[0];
	pObjT->obj.velocity[1] -= d*norm[1];// - norm[1];
	pObjT->obj.velocity[2] -= d*norm[2];// - norm[2];

	//friction = Float(pow(0.9f,pObjS->tCollide));

	//VectorScale( pObjT->obj.velocity, pObjT->obj.velocity, friction );
	//VectorScale( pObjS->obj.velocity, pObjS->obj.velocity, friction );
	
	EXIT( "HullCollide" );
}

Void CalculateMovementBox( ObjectInstancePtr pObj )
{
	ENTER;

	Float	radius;

	if (pObj->obj.flags & XCGEN_OBJ_SOLIDHULL)
		radius = pObj->objradius;	// Hulls must include ALL geometry
	else
		radius = pObj->obj.radius;	// Boxes can be any size

	for (Int i=0;i<3;i++)
	{
		pObj->movebbox[i] = pObj->framepos[i] - radius;
		pObj->movebbox[3+i] = pObj->framepos[i] + radius;
		if (pObj->obj.velocity[i]>0.0f)
			pObj->movebbox[3+i] += pObj->obj.velocity[i]*gEndEvent;
		else
			pObj->movebbox[i] += pObj->obj.velocity[i]*gEndEvent;
	}

	EXIT( "CalculateMovementBox" );
}

Void CalculateMovementBoxes( Void )
{
	ENTER;

	ObjectInstancePtr	pObj;

	pObj = gpObjects->pNext;
	while( pObj )
	{
		CalculateMovementBox( pObj );
		pObj = pObj->pNext;
	}

	EXIT( "CalculateMovementBoxes" );
}

Void ResetRegions( Void )
{
	ENTER;

	for (Int i=0;i<3;i++)
	{
		gnRegions[i] = 0;
		gRegionHead[i].pNext = NULL;
		gRegionHead[i].pPrev = NULL;
	}

	EXIT( "ResetRegions" );
}

Void AddObjectToRegions( ObjectInstancePtr pObj )
{
	ENTER;

	ObjectInstancePtr	pO, pLO, pCO, pNO;
	OccupiedRegionPtr	pR, pR2, pNext, pL;

	for (Int i=0;i<3;i++)
	{
		pR = gRegionHead[i].pNext;
		while( pR )
		{
			if ( (pR->begin <= pObj->movebbox[3+i]) &&
				 (pR->end >= pObj->movebbox[i]) )
			{
				// Object is within this region
				pR->begin = min(pR->begin,pObj->movebbox[i]);
				pR->end = max(pR->end,pObj->movebbox[3+i]);

				// Sort in object list
				pLO = NULL;
				pO = pR->pObj;
				while( pO && Int(pO)<Int(pObj) )
				{
					pLO = pO;
					pO = pO->pGroup[i];	
				}
				DEBUGAssert( Int(pO)!=Int(pObj) );
				if (pLO)
				{
					// Insert after pLO
					pObj->pGroup[i] = pLO->pGroup[i];
					pLO->pGroup[i] = pObj;
				}
				else
				{
					// Head of list
					pObj->pGroup[i] = pR->pObj;
					pR->pObj = pObj;
				}
				break;
			}
			pR = pR->pNext;
		}
		if (pR==NULL)
		{
			DEBUGAssert( gnRegions[i]<MAX_OBJECTS );

			// New region
			pR = &gRegions[i][gnRegions[i]];
			pR->begin = pObj->movebbox[i];
			pR->end = pObj->movebbox[3+i];
			pR->pObj = pObj;
			pObj->pGroup[i] = NULL;
			gnRegions[i]++;

			// Sort region in 
			pL = &gRegionHead[i];
			pR2 = gRegionHead[i].pNext;
			while( pR2 && (pR2->begin < pR->begin) )
			{
				pL = pR2;
				pR2 = pR2->pNext;
			}
			pL->pNext = pR;
			pR->pPrev = pL;
			pR->pNext = pR2;
			if (pR2)
			{
				//pR->pPrev = pR2->pPrev;
				pR2->pPrev = pR;
			}
		}
		// Check for newly overlapping regions
		//  and union them together
		pR2 = pR->pNext;
		while( pR2 )
		{
			pNext = pR2->pNext;
			if ( pR2->begin < pR->end )
			{
				// Overlapped regions, combine
				pR->end = max(pR->end,pR2->end);

				// Sort in objects
				pCO = pR2->pObj;
				while( pCO )
				{
					pNO = pCO->pGroup[i];

					pLO = NULL;
					pO = pR->pObj;
					while( pO && Int(pO)<Int(pCO) )
					{
						pLO = pO;
						pO = pO->pGroup[i];	
					}
					DEBUGAssert( Int(pO)!=Int(pCO) );
					if (pLO)
					{
						// Insert after pLO
						pCO->pGroup[i] = pLO->pGroup[i];
						pLO->pGroup[i] = pCO;
					}
					else
					{
						// Head of list
						pCO->pGroup[i] = pR->pObj;
						pR->pObj = pCO;
					}

					pCO = pNO;
				}

				// Delete region
				if (pR2->pNext)
					pR2->pNext->pPrev = pR2->pPrev;
				if (pR2->pPrev)
					pR2->pPrev->pNext = pR2->pNext;
			}
			pR2 = pNext;
		}
	}

	EXIT( "AddObjectToRegions" );
}

Void BuildObjectRegions( Void )
{
	ENTER;

	ObjectInstancePtr	pObj;

	ResetRegions();

	pObj = gpObjects->pNext;
	while( pObj )
	{
		pObj->region = 0;
		if ( (pObj->obj.flags & (XCGEN_OBJ_SOLIDBBOX+XCGEN_OBJ_SOLIDRADIUS+XCGEN_OBJ_SOLIDHULL) ) &&
			 !(pObj->obj.flags & XCGEN_OBJ_BEAM ) )
			AddObjectToRegions( pObj );
		pObj = pObj->pNext;
	}

	EXIT( "BuildObjectRegions" );
}

Float ClipLineToObject( ObjectInstancePtr pObj, Float radius, Vec_3 p0, Vec_3 p1 )
{
	ENTER;

	Vec_3	p;

	if (pObj->obj.flags & XCGEN_OBJ_SOLIDHULL)
	{
		// PLR TODO: Determine by radius which hull to use
		if (radius<4.0f)
			gpRoot = pObj->pMesh->pHull[0]->pRoot;
		else if (radius<10.0f)
			gpRoot = pObj->pMesh->pHull[1]->pRoot;
		else
			gpRoot = pObj->pMesh->pHull[2]->pRoot;

		// Transform Points into object's local coordinate system
		VectorSubtract( p, p0, pObj->framepos );
		MatVecMulInv( p0, pObj->framemat, p );
		VectorSubtract( p, p1, pObj->framepos );
		MatVecMulInv( p1, pObj->framemat, p );
	}	
	else
	{
		MakeBBoxHull( pObj->framepos, pObj->obj.radius + radius );
		gpRoot = &gBBoxHull[0];
	}

	gHSolid = TRUE;
	gHMinT = gEndEvent;

	VectorCopy( gHIPt, p1 );

	CheckHullCollision( gpRoot, p0, p1, 0.0f, gHMinT );

	VectorCopy( p1, gHIPt );

	if (pObj->obj.flags & XCGEN_OBJ_SOLIDHULL)
	{
		// Retransform Points into world coordinate system
		MatVecMul( p, pObj->framemat, p0 );
		VectorAdd( p0, p, pObj->framepos );
		MatVecMul( p, pObj->framemat, p1 );
		VectorAdd( p1, p, pObj->framepos );
	}

	if (gHMinT < gEndEvent)
		VectorCopy( gHCNorm, gHNorm );

	return( gHMinT );

	EXIT( "ClipLineToObject" );
}

Bool CanCollide( ObjectInstancePtr pObjS, ObjectInstancePtr pObjT )
{
	ENTER;

	Float	t;
	ObjectInstancePtr	pBigDaddy, pBigMommy;
	ObjectInstancePtr	pOwnerDaddy, pOwnerMommy;

	//if (pObjS==NULL)
		return( TRUE );

	// Objects attached to the same top parent do not collide
	pBigDaddy = pObjT;
	while( pBigDaddy->obj.pParent )
		pBigDaddy = ObjectInstancePtr(pBigDaddy->obj.pParent);

	pBigMommy = pObjS;
	while( pBigMommy->obj.pParent )
		pBigMommy = ObjectInstancePtr(pBigMommy->obj.pParent);
 
	if ( pBigDaddy==pBigMommy )
		return( FALSE );

	// Parents of object's owner also do not collide
	pOwnerDaddy = ObjectInstancePtr(pObjT->obj.pOwner);
	while (pOwnerDaddy)
	{
		if (pOwnerDaddy==pObjS)
			return( FALSE );
		pOwnerDaddy = ObjectInstancePtr(pOwnerDaddy->obj.pParent);
	}

	pOwnerMommy = ObjectInstancePtr(pObjS->obj.pOwner);
	while (pOwnerMommy)
	{
		if (pOwnerMommy==pObjT) 
			return( FALSE );
		pOwnerMommy = ObjectInstancePtr(pOwnerMommy->obj.pParent);
	}

	return( TRUE );

	EXIT( "CanCollide" );
}

ObjectInstancePtr ClipLineToWorld( ObjectInstancePtr pObj, Float radius, Vec_3 p0, Vec_3 p1 )
{
	ENTER;

	Float				temp;
	Vec_3				low, high;
	ObjectInstancePtr	pCObj;
	OccupiedRegionPtr	pRx, pRy, pRz;

	pCObj = NULL;

	low[0] = p0[0];
	high[0] = p1[0];
	if (low[0]>high[0])
	{
		temp = high[0];
		high[0] = low[0];
		low[0] = temp;
	}
	// CHECK X REGIONS
	pRx = gRegionHead[0].pNext;	
	while( pRx )
	{
		if ( pRx->begin<=high[0] && pRx->end>=low[0] )
		{
			low[1] = p0[1];
			high[1] = p1[1];
			if (low[1]>high[1])
			{
				temp = high[1];
				high[1] = low[1];
				low[1] = temp;
			}
			// CHECK Y REGIONS
			pRy = gRegionHead[1].pNext;
			while( pRy )
			{
				if ( pRy->begin<=high[1] && pRy->end>=low[1] )
				{
					low[2] = p0[2];
					high[2] = p1[2];
					if (low[2]>high[2])
					{
						temp = high[2];
						high[2] = low[2];
						low[2] = temp;
					}
					// CHECK Z REGIONS
					pRz = gRegionHead[2].pNext;
					while( pRz )
					{
						// INTERSECTS A REGION
						if ( pRz->begin<=high[2] && pRz->end>=low[2] )
						{
							ObjectInstancePtr	pO, pO2, pO3;

							// OBTAIN SET OF ALL OBJECTS
							// IN THIS REGION AND PERFORM ACTION 
							pO = pRx->pObj;
							while( pO )
							{
								pO2 = pRy->pObj;
								while( pO!=pObj && pO2 && pO2<=pO )
								{
									pO3 = pRz->pObj;
									while( pO2==pO && pO3 && pO3<=pO )
									{
										if (pO3==pO && CanCollide(pObj,pO))
										{
											Float	t;

											t = ClipLineToObject( pO, radius, p0, p1 );
											if ( t>MINTIMESLICE && t<gEndEvent )
											{
												pCObj = pO;
												if (pObj)
												{
													pObj->pCollide = pO;
													pObj->tCollide = t;
													VectorCopy( pObj->collideNorm, gHCNorm );
													VectorCopy( pObj->collidePos, gHIPt );
												}
											}
										}

										pO3 = pO3->pGroup[2];
									}
									pO2 = pO2->pGroup[1];
								}
								pO = pO->pGroup[0];
							}
						}
						pRz = pRz->pNext;
					}
				}
				pRy = pRy->pNext;
			}
		}
		pRx = pRx->pNext;
	}

	return( pCObj );

	EXIT( "ClipLineToWorld" );
}

Void ObjectCollide( ObjectInstancePtr pObj, ObjectInstancePtr pCObj )
{
	ENTER;

	HullCollide( pObj, pCObj );

	EXIT( "ObjectCollide" );
}

Void MoveObjects( Void )
{
	ENTER;

	Int					i;
	Float				event, offset;
	ObjectInstancePtr	pObj, pNext, pCObj;

	// Massive kludge
	event = gEndEvent;

	pObj = gpObjects->pNext;
	while( pObj )
	{
		pNext = pObj->pNext;

		offset = 0.0f;
		gEndEvent = event;

		while( pObj->bAlive && (offset < event) )
		{
			pObj->tCollide = gEndEvent;

			if (pObj->obj.flags & XCGEN_OBJ_BEAM)
			{
				Vec_3	p0, p1, p;

				VectorCopy( p0, pObj->pBeam->tail );
				VectorCopy( p1, pObj->pBeam->vhead );
				VectorScale( p1, p1, gEndEvent );
				VectorAdd( p1, p1, pObj->pBeam->head );

				for (i=0;i<3;i++)
				{
					p0[i] = floor(p0[i]*(1.0f/POSGRANULAR)+0.5f)*POSGRANULAR;
					p1[i] = floor(p1[i]*(1.0f/POSGRANULAR)+0.5f)*POSGRANULAR;
				}

				if (pObj->obj.flags & XCGEN_OBJ_SOLIDBBOX)
					pCObj = ClipLineToWorld( pObj, pObj->pBeam->hwidth, p0, p1 );
				else
					pCObj = NULL;

				for (i=0;i<3;i++)
				{
					if (p1[i]>p0[i])
						p1[i] = floor(p1[i]*(1.0f/POSGRANULAR))*POSGRANULAR;
					else
						p1[i] = ceil(p1[i]*(1.0f/POSGRANULAR))*POSGRANULAR;
				}

				if (pCObj)
				{
					pObj->pBeam->vhead[0] = 0.0f;
					pObj->pBeam->vhead[1] = 0.0f;
					pObj->pBeam->vhead[2] = 0.0f;
				}

				VectorCopy( pObj->pBeam->head, p1 );
				VectorCopy( p, pObj->pBeam->vtail );
				VectorScale( p, p, pObj->tCollide );
				VectorAdd( pObj->pBeam->tail, pObj->pBeam->tail, p );

			}
			else
			{
				Vec_3	pos, nextpos, p0;

				VectorCopy( pos, pObj->framepos );
				VectorCopy( nextpos, pObj->obj.velocity );
				VectorScale( nextpos, nextpos, gEndEvent );
				VectorAdd( nextpos, nextpos, pos );

				for (i=0;i<3;i++)
				{
					pos[i] = floor(pos[i]*(1.0f/POSGRANULAR)+0.5f)*POSGRANULAR;
					nextpos[i] = floor(nextpos[i]*(1.0f/POSGRANULAR)+0.5f)*POSGRANULAR;
				}

				if (pObj->obj.flags & (XCGEN_OBJ_SOLIDBBOX|XCGEN_OBJ_SOLIDHULL))
					pCObj = ClipLineToWorld( pObj, pObj->obj.radius, pos, nextpos );
				else
					pCObj = NULL;

				for (i=0;i<3;i++)
				{
					if (nextpos[i]>pos[i])
						nextpos[i] = floor(nextpos[i]*(1.0f/POSGRANULAR))*POSGRANULAR;
					else
						nextpos[i] = ceil(nextpos[i]*(1.0f/POSGRANULAR))*POSGRANULAR;
				}

				if (pCObj)
				{
					if (pCObj->obj.flags & XCGEN_OBJ_SOLIDHULL)
					{
						// PLR TODO: Determine by radius which hull to use
						if (pObj->obj.radius<4.0f)
							gpRoot = pCObj->pMesh->pHull[0]->pRoot;
						else if (pObj->obj.radius<10.0f)
							gpRoot = pCObj->pMesh->pHull[1]->pRoot;
						else
							gpRoot = pCObj->pMesh->pHull[2]->pRoot;

					}	
					else
					{
						MakeBBoxHull( pCObj->framepos, pCObj->obj.radius + pObj->obj.radius );
						gpRoot = &gBBoxHull[0];
					}

					if (PointInHull( gpRoot, nextpos ))
					{
						Vec_3	dv;

						for (i=0;i<3;i++)
						{
							if (pObj->collideNorm[i]>0.0f)
								dv[i] = nextpos[i] - pos[i];
							else
								dv[i] = pos[i] - nextpos[i];
						}

						VectorCopy( p0, nextpos );
						if (HullUnstick( gpRoot, p0, dv ))
						{
							// Copy the new position into the object
							//MatVecMul( p, pObjS->framemat, p0 );
							//VectorAdd( pObjT->framepos, pObjS->framepos, p );
							VectorCopy( nextpos, p0 );
						}
						else
						{
							VectorCopy( nextpos, pos );
						}
					}
				}


				if (pObj->tCollide>0.0f)
					VectorCopy( pObj->framepos, nextpos );

				if (pCObj)
					ObjectCollide( pObj, pCObj );
			}

			if (pObj->pAttach)
				ProcessAttachedEmits( pObj, gLastMotionTick+Long(pObj->tCollide*1000.0f) );

			if (pCObj)
			{
				if (pObj->obj.pCollide!=NULL)
				{
					if ((pObj->obj.pCollide)(XCGENObjectPtr(pObj),XCGENObjectPtr(pCObj),gLastMotionTick+Long(pObj->tCollide*1000.0f))==NULL)
					{
						// If proc returns NULL, no processing occured, go on to the
						//	Next proc
						if (pCObj->obj.pCollide!=NULL)
						{
							(pCObj->obj.pCollide)(XCGENObjectPtr(pCObj),XCGENObjectPtr(pObj),gLastMotionTick+Long(pObj->tCollide*1000.0f));
						}
					}
				}
				else if (pCObj->obj.pCollide!=NULL)
				{
					(pCObj->obj.pCollide)(XCGENObjectPtr(pCObj),XCGENObjectPtr(pObj),gLastMotionTick+Long(pObj->tCollide*1000.0f));
				}
			}

			if (pCObj)
			{
				offset += pObj->tCollide;
				gEndEvent -= offset;
			}
			else
			{
				offset = event;
			}
		}

		pObj = pNext;
	}

	EXIT( "MoveObjects" );
}

Void DoMotion( XCGENTimestamp stamp )
{
	ENTER;

	ObjectInstancePtr	pObj, pNext;

	// Frame zero setup
	if (gLastMotionTick==0)
		gLastMotionTick = stamp;

	gStats.tsMotion = 0;
	TIMERBEGIN( gStats.tsMotion );

	// Perform auto deletion and brains
	pObj = gpObjects->pNext;
	while( pObj )
	{
		pNext = pObj->pNext;
		if (pObj->obj.tsThink!=0 && stamp>pObj->obj.tsThink)
		{
			if (pObj->obj.pThink)
				(pObj->obj.pThink)(XCGENObjectPtr(pObj),NULL,stamp);
			// Just in case tsThink is not updated, kill it
			//  don't just keep calling it forever more
			if (stamp>pObj->obj.tsThink)
				pObj->obj.tsThink = 0;
		}
		if (pObj->obj.tsKill!=0 && stamp>pObj->obj.tsKill)
		{
			if (pObj->obj.pKill)
				(pObj->obj.pKill)(XCGENObjectPtr(pObj),NULL,stamp);
			RemoveObject( pObj );
		}
		pObj = pNext;
	}

	gThisMotionTick = stamp;
	gEndEvent = Float(stamp-gLastMotionTick)/1000.0f;	// This t value is in seconds

	ApplyThrust( gEndEvent );
	ApplyAngularMomentum( gEndEvent );
	
	gStats.tsRegions = 0;
	CalculateFramePosition();  

	//while( gEndEvent>0.0f )
	//{
		TIMERBEGIN( gStats.tsRegions );

		CalculateMovementBoxes();
		BuildObjectRegions();

		TIMEREND( gStats.tsRegions );

	//	ProcessACollision();
	//}

	MoveObjects();

	gLastMotionTick = stamp;

	ProcessParticles( stamp );

	CalculateRealPosition();

	TIMEREND( gStats.tsMotion );
	
	EXIT( "DoMotion" );
}

Void ApplyRotation( ObjectInstancePtr pObj, Float roll, Float pitch, Float yaw )
{
	ENTER;

	Float		e1dot, e2dot, e3dot, e4dot;
	FloatPtr	q;

	q = FloatPtr(&pObj->obj.rot[0]);

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

   EXIT( "ApplyRotation" );
}

