/*******************************************************************
**    File: XCMain.cpp
**      By: Paul L. Rowan      
** Created: 970415
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
XCGENDLL XCGENResult XCGENSetDatabaseDir( CharPtr pPath )
{
	ENTER;

	strcpy( szDatabasePath, pPath );

	return( XCGEN_OKAY );

	EXIT ( "XCGENSetDatabaseDir" );
}

XCGENDLL XCGENResult XCGENNewTexture( XCGENTexturePtr *pTex, XCGENResName pName )
{
	ENTER;

	*pTex = GetTexture(pName);

	return( XCGEN_OKAY );

	EXIT( "XCGENNewTexture" );
}

XCGENDLL XCGENResult XCGENNewObject( XCGENObjectPtr *pObject, XCGENResName pName, XCGENObjType type )
{
	ENTER;

	*pObject = XCGENObjectPtr(CreateNewObject( pName, type ));

	return( XCGEN_OKAY );

	EXIT ( "XCGENNewObject" );
}

XCGENDLL XCGENResult XCGENDeleteObject( XCGENObjectPtr pObject )
{
	ENTER;

	RemoveObject( ObjectInstancePtr(pObject) );

	return( XCGEN_OKAY );

	EXIT ( "XCGENDeleteObject" );
}

XCGENDLL XCGENResult XCGENDeleteAllObjects( Void )
{
	ENTER;

	ObjectInstancePtr	pNext, pObject;

	pObject = gpObjects->pNext;
	while( pObject )
	{
		pNext = pObject->pNext;
		RemoveObject( pObject );
		pObject = pNext;
	}

	// PLR - TODO: This probably needs to be in a more appropriate place
	gLastMotionTick  = 0;

	return( XCGEN_OKAY );

	EXIT( "XCGENDeleteAllObjects" );
}

XCGENDLL XCGENResult XCGENRotateObject( XCGENObjectPtr pObj, Float roll, Float pitch, Float yaw )
{
	ENTER;

	ApplyRotation( ObjectInstancePtr(pObj), roll, pitch, yaw );

	return( XCGEN_OKAY );

	EXIT( "XCGENRotateObject" );
}

XCGENDLL XCGENResult XCGENAttachObject( XCGENObjectPtr pParent, XCGENObjectPtr pAttach, Int ID )
{
	ENTER;

	pAttach->flags |= XCGEN_OBJ_ATTACHED;
	pAttach->pParent = pParent;
	ObjectInstancePtr(pAttach)->attachID = ID;
	pParent->pManips[ID].type = XCGEN_MANIP_OBJECT;
	pParent->pManips[ID].pObj = pAttach;
	ObjectInstancePtr(pAttach)->pAttachPt = &(ObjectInstancePtr(pParent)->pMesh->pLOD[0]->pAttach[pParent->pManips[ID].attachID]);

	return( XCGEN_OKAY );

	EXIT( "XCGENAttachObject" );
}

XCGENDLL XCGENResult XCGENDetachObject( XCGENObjectPtr pAttach )
{
	ENTER;

	if (!(pAttach->flags & XCGEN_OBJ_ATTACHED))
		return( XCGEN_OKAY );

	pAttach->pParent->pManips[ObjectInstancePtr(pAttach)->attachID].pObj = NULL;
	pAttach->pParent->pManips[ObjectInstancePtr(pAttach)->attachID].type = XCGEN_MANIP_IGNORE;
	pAttach->flags &= ~(XCGEN_OBJ_ATTACHED);
	ObjectInstancePtr(pAttach)->attachID = -1;
	ObjectInstancePtr(pAttach)->pAttachPt = NULL;

	VectorCopy( pAttach->pos, ObjectInstancePtr(pAttach)->framepos );
	if (pAttach->flags & XCGEN_OBJ_MATONLY)
	{
		VectorCopy( pAttach->mat[0], ObjectInstancePtr(pAttach)->framemat[0] );
		VectorCopy( pAttach->mat[1], ObjectInstancePtr(pAttach)->framemat[1] );
		VectorCopy( pAttach->mat[2], ObjectInstancePtr(pAttach)->framemat[2] );
	}
	else
	{
		MatrixToQuaternion( ObjectInstancePtr(pAttach)->framemat, pAttach->rot );
	}

	return( XCGEN_OKAY );

	EXIT( "XCGENDetachObject" );	
}

XCGENDLL XCGENResult XCGENGetAttach( XCGENObjectPtr pObj, Int manipid, Vec_3 pos, Mat_3 mat )
{
	ENTER;
	
	if (pObj->pManips[manipid].attachID!=-1)
	{
		VectorCopy( pos, ObjectInstancePtr(pObj)->pMesh->pLOD[0]->pAttach[pObj->pManips[manipid].attachID].pos );
		VectorCopy( mat[0], ObjectInstancePtr(pObj)->pMesh->pLOD[0]->pAttach[pObj->pManips[manipid].attachID].mat[0] );
		VectorCopy( mat[1], ObjectInstancePtr(pObj)->pMesh->pLOD[0]->pAttach[pObj->pManips[manipid].attachID].mat[1] );
		VectorCopy( mat[2], ObjectInstancePtr(pObj)->pMesh->pLOD[0]->pAttach[pObj->pManips[manipid].attachID].mat[2] );
	}

	return( XCGEN_OKAY );
	
	EXIT( "XCGENGetAttach" );
}

XCGENDLL XCGENResult XCGENGetRealPosition( XCGENObjectPtr pObj, Vec_3 pos, Mat_3 mat )
{
	ENTER;

	ObjectInstancePtr	pO;

	pO = ObjectInstancePtr(pObj);

	VectorCopy( pos, pO->framepos );
	VectorCopy( mat[0], pO->framemat[0] );
	VectorCopy( mat[1], pO->framemat[1] );
	VectorCopy( mat[2], pO->framemat[2] );

	return( XCGEN_OKAY );

	EXIT( "XCGENGetRealPosition" );
}

XCGENDLL XCGENResult XCGENWorldToLocal( XCGENObjectPtr pObj, Vec_3 vec, Vec_3 xvec )
{
	ENTER;

	Vec_3				v;
	ObjectInstancePtr	pO;

	pO = ObjectInstancePtr(pObj);

	VectorSubtract( v, vec, pO->framepos );
	MatVecMulInv( xvec, pO->framemat, v );

	return( XCGEN_OKAY );

	EXIT( "XCGENWorldToLocal" );
}

XCGENDLL XCGENResult XCGENLocalToWorld( XCGENObjectPtr pObj, Vec_3 vec, Vec_3 xvec )
{
	ENTER;

	Vec_3				v;
	ObjectInstancePtr	pO;

	pO = ObjectInstancePtr(pObj);

	MatVecMul( v, pO->framemat, vec );
	VectorAdd( xvec, pO->framepos, v );

	return( XCGEN_OKAY );

	EXIT( "XCGENLocalToWorld" );
}

XCGENDLL XCGENResult XCGENOrientWorldToLocal( XCGENObjectPtr pObj, Vec_3 vec, Vec_3 xvec )
{
	ENTER;

	ObjectInstancePtr	pO;

	pO = ObjectInstancePtr(pObj);

	MatVecMulInv( xvec, pO->framemat, vec );

	return( XCGEN_OKAY );

	EXIT( "XCGENOrientWorldToLocal" );
}

XCGENDLL XCGENResult XCGENOrientLocalToWorld( XCGENObjectPtr pObj, Vec_3 vec, Vec_3 xvec )
{
	ENTER;

	ObjectInstancePtr	pO;

	pO = ObjectInstancePtr(pObj);

	MatVecMul( xvec, pO->framemat, vec );

	return( XCGEN_OKAY );

	EXIT( "XCGENOrientLocalToWorld" );
}

XCGENDLL XCGENResult XCGENSetGlobalLightSource( Vec_3 vec, UChar r, UChar g, UChar b )
{
	ENTER;

	VectorCopy( gLight, vec );
	NormalizeVector( gLight );
	gLR = r;
	gLG = g;
	gLB = b;

	return( XCGEN_OKAY );

	EXIT( "XCGENSetGlobalLightSource" );
}

XCGENDLL XCGENResult XCGENNewBeam( XCGENObjectPtr *ppObj, XCGENResName pTex, XCGENBeamPtr pTemplate )
{
	ENTER;

	(*ppObj) = XCGENObjectPtr(CreateBeam(pTemplate));

	if (pTex)
		ObjectInstancePtr(*ppObj)->pBeam->pTex = GetTexture( pTex );

	return( XCGEN_OKAY );

	EXIT( "XCGENNewBeam" );
}

XCGENDLL XCGENResult XCGENGetBeam( XCGENObjectPtr pObj, XCGENBeamPtr *ppBeam )
{
	ENTER;

	(*ppBeam) = ObjectInstancePtr(pObj)->pBeam;

	return( XCGEN_OKAY );

	EXIT( "XCGENGetBeam" );
}

XCGENDLL XCGENResult XCGENLineOfSight( XCGENObjectPtr *ppObj, Vec_3 start, Vec_3 end )
{
	ENTER;

	XCGENObjectPtr	pObj;

	pObj = XCGENObjectPtr(ClipLineToWorld( NULL, 0.0f, start, end ));

	if (ppObj)
		*ppObj =pObj;

	return( XCGEN_OKAY );

	EXIT( "XCGENLineOfSight" );
}

XCGENDLL XCGENResult XCGENResetUniverse( Void )
{
	ENTER;

	return( XCGEN_OKAY );

	EXIT ( "XCGENResetUniverse" );
}

XCGENDLL XCGENResult XCGENInitUniverse( XCGENUniversePtr pUniverse )
{
	ENTER;

	return( XCGEN_OKAY );

	EXIT ( "XCGENInitUniverse" );
}

XCGENDLL XCGENResult XCGENSetRenderSurface( XCGENSurfacePtr pSurf )
{
	ENTER;

	DDPIXELFORMAT	ddpf;

	gRenderSurf.pDD = pSurf->pDD;
	gRenderSurf.pDDS = pSurf->pDDS;
	gRenderSurf.pZDDS = pSurf->pZDDS;
	gRenderSurf.pD3DDev = pSurf->pD3DDev;
	gRenderSurf.pD3DDev2 = pSurf->pD3DDev2;

	if (gRenderSurf.pD3DDev2!=NULL)
	{
		D3DTestCaps();
		gRenderSurf.bD3D = TRUE;
	}
	else
	{
		gRenderSurf.bD3D = FALSE;
	}

	if (gRenderSurf.pZDDS!=NULL)
		gRenderSurf.bZBuff = TRUE;
	else
		gRenderSurf.bZBuff = FALSE;

	memset( &ddpf, 0, sizeof(DDPIXELFORMAT) );
	ddpf.dwSize = sizeof(DDPIXELFORMAT);
	gRenderSurf.pDDS->GetPixelFormat( &ddpf );

	// Test to make sure this is a 16-bit surface
	if (ddpf.dwRGBBitCount!=16)
	{
		FATALError( "Unsupported Surface Type (Not 16-bit)" );
	}
	// Test to make sure this is an RGB surface (not BGR)
	if (ddpf.dwBBitMask!=0x1f)
	{
		FATALError( "Unsupported Surface Type (BGR)" );
	}

	// Determine 555 or 565 format
	if (ddpf.dwGBitMask==0x3e0)
		gRenderSurf.b555 = TRUE;
	else
		gRenderSurf.b555 = FALSE;

	if (gRenderSurf.bD3D==TRUE)
	{
		FlushD3DTextureCache();
		InitD3DTextureCache();
		D3DTestCaps();
	}

	return( XCGEN_OKAY );

	EXIT ( "XCGENSetRenderSurface" );
}

XCGENDLL XCGENResult XCGENSetViewport( XCGENViewportDefPtr pViewport )
{
	ENTER;
	
	gViewport.width = pViewport->sWidth;
	gViewport.height = pViewport->sHeight;
	gViewport.xcenter = pViewport->sWidth / 2.0f - 0.5f;
	gViewport.ycenter = pViewport->sHeight / 2.0f - 0.5f;
	gViewport.fov = pViewport->zScale;

	gViewport.xscale = pViewport->sWidth / pViewport->zScale;
	gViewport.xscaleinv = 1.0f / gViewport.xscale;
	gViewport.yscale = pViewport->sHeight / pViewport->zScale;
	gViewport.yscaleinv = 1.0f / gViewport.yscale;

	gViewport.maxscale = (gViewport.xscale>gViewport.yscale) ? gViewport.xscale : gViewport.yscale;
	gViewport.maxscaleinv = 1.0f / gViewport.maxscale;

	gViewport.xoff = pViewport->xOff;
	gViewport.yoff = pViewport->yOff;

	gViewport.flags = pViewport->flags;

	BuildFrustum();

	if (gRenderSurf.bD3D)
	{
		LPDIRECT3DVIEWPORT2	pPort;
		D3DVIEWPORT2		d3dvp;
		HRESULT				res;

		res = gRenderSurf.pD3DDev2->GetCurrentViewport( &pPort );
		DEBUGAssert( res==D3D_OK );

		memset( &d3dvp, 0, sizeof(D3DVIEWPORT2) );
		d3dvp.dwSize = sizeof(D3DVIEWPORT2);
		d3dvp.dwX = pViewport->xOff;
		d3dvp.dwY = pViewport->yOff;
		d3dvp.dwWidth = pViewport->sWidth;
		d3dvp.dwHeight = pViewport->sHeight;
		d3dvp.dvClipX = 0.0f;
		d3dvp.dvClipY = 0.0f;
		d3dvp.dvClipWidth = 1.0f;
		d3dvp.dvClipHeight = 1.0f;
		d3dvp.dvMinZ = 0.0f;
		d3dvp.dvMaxZ = 1.0f;
		
		pPort->SetViewport2( &d3dvp );

		//gRenderSurf.pD3DDev2->SetCurrentViewport( pPort );

		pPort->Release();
	}

	//gViewport.pitch = pViewport->rowBytes;
	//gViewport.pZBuffer = pViewport->pZBuffer;

	return( XCGEN_OKAY );

	EXIT ( "XCGENSetViewport" );
}

XCGENDLL XCGENResult XCGENProjectPoint( Vec_3 pd, Vec_3 ps )
{
	ENTER;

    Float	zrecip;

    zrecip = 1.0f / ps[2];
    pd[0] = ps[0] * zrecip * gViewport.maxscale + gViewport.xcenter;
    pd[1] = gViewport.ycenter - (ps[1] * zrecip * gViewport.maxscale);
	pd[2] = ps[2];

	return( XCGEN_OKAY );

	EXIT( "XCGENProjectPoint" );
}

XCGENDLL XCGENResult XCGENGetCamera( XCGENCameraPtr *pCam )
{
	ENTER;

	*pCam = &gCamera;

	return( XCGEN_OKAY );

	EXIT( "XCGENGetCamera" );
}

XCGENDLL XCGENResult XCGENRenderFrame( XCGENTimestamp stamp )
{
	ENTER;

	gCamera.tsCurrent = stamp;

	RenderCamera();

	return( XCGEN_OKAY );

	EXIT ( "XCGENRenderFrame" );
}

XCGENDLL XCGENResult XCGENDoMotion( XCGENTimestamp stamp )
{
	ENTER;

	DoMotion( stamp );

	return( XCGEN_OKAY );

	EXIT ( "XCGENDoMotion" );
}

XCGENDLL XCGENResult XCGENEmitParticles( XCGENParticleEmitPtr pEmit, XCGENTimestamp stamp )
{
	ENTER;

	EmitParticles( pEmit, stamp );

	return( XCGEN_OKAY );

	EXIT( "XCGENEmitParticles" );
}

XCGENDLL XCGENResult XCGENParticles( XCGENParticleDefPtr pDef, XCGENTimestamp stamp )
{
	ENTER;

	Particles( pDef, stamp );

	return( XCGEN_OKAY );

	EXIT( "XCGENEmitParticles" );
}

XCGENDLL XCGENResult XCGENAttachEmitter( XCGENObjectPtr pObj, XCGENParticleEmitPtr pEmit, Vec_3 offset, XCGENTimestamp stamp )
{
	ENTER;

	AttachEmitter( ObjectInstancePtr(pObj), pEmit, offset, stamp );

	return( XCGEN_OKAY );

	EXIT( "XCGENAttachEmitter" );
}

XCGENDLL XCGENResult XCGENGetStats( XCGENStatsPtr pStats )
{
	ENTER;

	memcpy( pStats, &gStats, sizeof(XCGENStats) );

	return( XCGEN_OKAY);

	EXIT ( "XCGENGetStats" );
}

BOOL WINAPI DllMain( HINSTANCE  hinstDLL, DWORD  fdwReason, LPVOID  lpvReserved )
{
	BOOL	ret = FALSE;

	switch( fdwReason )
	{
		case DLL_PROCESS_ATTACH:
				InitDust();
				InitTrigTable();
				InitBeamObject();
				InitParticles();
				LoadIcons();
				CalibrateTimer();
				ret = TRUE;
			break;
		case DLL_PROCESS_DETACH:
			#if 0
				Char	mess[128];
				
				sprintf( mess, "XCGEN Maximum memory usage: %d\n", gMaxMem );
				DEBUGLog( mess );
				sprintf( mess, "XCGEN Remaining unfreed memory: %d\n", gTotalMem );
				DEBUGLog( mess );
			#endif
			break;
	}
 
	return( ret );
}


