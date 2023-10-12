/*******************************************************************
**    File: XCObject.cpp
**      By: Paul L. Rowan     
** Created: 970415
**
** Description: loading and deleting of objects
**
********************************************************************/

/*--- INCLUDES ----------------------------------------------------*/
#define INITGUID

#include "xcgenint.h"

/*--- CONSTANTS ---------------------------------------------------*/


/************************ IMPORT SECTION ***************************/
/*-------- FUNCTION PROTOTYPES ------------------------------------*/

/*-------- GLOBAL VARIABLES ---------------------------------------*/


/************************ PRIVATE SECTION **************************/
/*-------- FUNCTION PROTOTYPES ------------------------------------*/

/*-------- GLOBAL VARIABLES ---------------------------------------*/


/************************ MEMBER FUNCTIONS *************************/
Void ConvertSpriteTo565( AnimSpritePtr pSpr )
{
	ENTER;

	SpriteFramePtr	pFrame;
	ShortPtr		pDat;
	Short			color;
	Int				i, size;

	pDat = ShortPtr(UCharPtr(pSpr)+sizeof(AnimSprite) + sizeof(Long)*pSpr->nFrames);

	for (i=0;i<pSpr->nFrames;i++)
	{
		pFrame = SpriteFramePtr(UCharPtr(pSpr) + pSpr->offFrames[i]);
		size = pFrame->width*pFrame->height;
		pDat = &pFrame->sprData[0];
		while( size-- )
		{
			color = *pDat;
			color = ((color&0xFFE0)<<1) + (color&0x1F);
			*pDat = color;
			pDat++;
		}
	}

	EXIT( "ConvertSpriteTo565" );
}

AnimSpritePtr GetSprite( XCGENResName pName )
{
	ENTER;

	AnimSpritePtr	pSpr;

	pSpr = AnimSpritePtr(GetResData( pName, XCGEN_RESTYPE_SPRITE ));

	if (pSpr->header.ref==1 && !gRenderSurf.b555)
	{
		gStats.nSprites++;
		gStats.memSprites += ResourceSize( XCGENResourcePtr(pSpr) );
		ConvertSpriteTo565( pSpr );
	}

	return( pSpr );

	EXIT( "GetSprite" );
}

Void RecursiveInitNode( MeshTexListPtr pTL, MeshLODPtr pLOD, MeshNodePtr pNode )
{
	ENTER;

	Int				i;
	UCharPtr		pD;
	MeshFacePtr		pF;

	// Hook up the faces' texture pointers
	pD = UCharPtr(pNode+1);
	for (i=0;i<pNode->nFaces;i++)
	{
		pF = MeshFacePtr(pD);

		if (pF->iTexture!=-1)
		{
			pF->pTexture = pTL[pF->iTexture].pTexture;
		}
		else
		{
			pF->pTexture = NULL;
		}

		pD += sizeof(MeshFace) + pF->nEdges*sizeof(MeshEdgeIndex);		
	}

	// Initialize Attachments
	if (pNode->iAttach!=-1)
	{
		pNode->pAttach = &pLOD->pAttach[pNode->iAttach];
	}
	else
	{
		pNode->pAttach = NULL;
	}

	// Initialize any children
	for (i=0;i<2;i++)
	{
		if (pNode->children[i]==-1)
		{
			pNode->pChildren[i] = NULL;
		}
		else
		{
			pNode->pChildren[i] = MeshNodePtr(UCharPtr(pLOD)+pNode->children[i]);
			RecursiveInitNode( pTL, pLOD, pNode->pChildren[i] );
		}
	}

	EXIT ( "RecursiveInitNode" );
}

Void ConvertTextureTo565( XCGENTexturePtr pTex )
{
	ENTER;

	Int		i, j;

	for (i=0;i<16;i++)
	for (j=0;j<256;j++)
	{
		pTex->cTable[i][j] = ((pTex->cTable[i][j]&0xffe0)<<1) + (pTex->cTable[i][j]&0x1f);
	}

	EXIT( "ConvertTextureTo565" );
}

// Just in case the precalculated values for the shade table are not enough
//  let's update them on the fly
Void ChangeShadeTable( XCGENTexturePtr pTex )
{
	ENTER;

	Int		shade, color;
	Float	r, g, b, rr, gg, bb;
	Int		ir, ig, ib;

	for (color=0;color<256;color++)
	{
		r = Float((pTex->cTable[0][color]>>10)&0x1f)/32.0f;
		g = Float((pTex->cTable[0][color]>>5)&0x1f)/32.0f;
		b = Float((pTex->cTable[0][color])&0x1f)/32.0f;

		// Specular ramp
		/*for (shade=0;shade<8;shade++)
		{
			rr = 1.0f - ((1.0f-r) * Float(shade+2)/10.0f);
			gg = 1.0f - ((1.0f-g) * Float(shade+2)/10.0f);
			bb = 1.0f - ((1.0f-b) * Float(shade+2)/10.0f);

			ir = Int( rr * 32.0f );
			ig = Int( gg * 32.0f );
			ib = Int( bb * 32.0f );

			pTex->cTable[shade][color] = (ir<<10) + (ig<<5) + ib;
		}

		// Diffuse ramp
		for (shade=0;shade<8;shade++)
		{
			rr = r * Float(10-shade)/10.0f;
			gg = g * Float(10-shade)/10.0f;
			bb = b * Float(10-shade)/10.0f;

			ir = Int( rr * 32.0f );
			ig = Int( gg * 32.0f );
			ib = Int( bb * 32.0f );

			pTex->cTable[shade+8][color] = (ir<<10) + (ig<<5) + ib;
		}*/

		// Diffuse ramp
		for (shade=0;shade<16;shade++)
		{
			rr = r * Float(XCGEN_TEX_AMBIENT-shade)/Float(XCGEN_TEX_AMBIENT);
			gg = g * Float(XCGEN_TEX_AMBIENT-shade)/Float(XCGEN_TEX_AMBIENT);
			bb = b * Float(XCGEN_TEX_AMBIENT-shade)/Float(XCGEN_TEX_AMBIENT);

			ir = min(max(Int( rr * 32.0f ),0),31);
			ig = min(max(Int( gg * 32.0f ),0),31);
			ib = min(max(Int( bb * 32.0f ),0),31);

			pTex->cTable[shade][color] = (ir<<10) + (ig<<5) + ib;
		}
	}

	EXIT( "ChangeShadeTable" );
}

#define MAXTEXWIDTH	128

XCGENTexturePtr GetTexture( XCGENResName pName )
{
	ENTER;
	
	XCGENTexturePtr	pTex;
	
	if (gRenderSurf.bD3D)
	{
		LPDIRECTDRAWSURFACE	pDDS, pDDSS, pDDSNext;
		LPDIRECT3DTEXTURE	pD3DTex;			
		UCharPtr			pSurf, pMip;
		DDSURFACEDESC		ddsd;
		HRESULT				res;
		Int					level, w, h, j, k;
		UChar				color;

		pTex = XCGENTexturePtr(GetResData(pName,XCGEN_RESTYPE_TEXTURE));
		
		level = 0;

		if (!gRenderSurf.bAGP)
		{
			Int		mw;

			mw = max(pTex->width,pTex->height);
			if (mw>MAXTEXWIDTH)
			{
				while (mw>MAXTEXWIDTH)
				{
					level++;
					mw>>=1;
				}
			}
		}

		if (pTex->header.ref==1)
		{
			gStats.nTextures++;

			memset( &ddsd, 0, sizeof(DDSURFACEDESC) );
			ddsd.dwSize = sizeof(DDSURFACEDESC);
			ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT;
			ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_TEXTURE;
			if (gRenderSurf.bAGP)
			{
				ddsd.dwFlags |= DDSD_MIPMAPCOUNT;
				ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY | DDSCAPS_NONLOCALVIDMEM | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
				ddsd.dwMipMapCount = 4;
			}
			else
			{
				ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
			}
			ddsd.dwHeight = pTex->height>>level;
			ddsd.dwWidth = pTex->width>>level;
			ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
			ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
			ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
			ddsd.ddpfPixelFormat.dwRBitMask = 0x0000F800;
			ddsd.ddpfPixelFormat.dwGBitMask = 0x000007E0;
			ddsd.ddpfPixelFormat.dwBBitMask = 0x0000001F;

			res = gRenderSurf.pDD->CreateSurface( &ddsd, &pDDSS, NULL );

			if (res!=DD_OK)
			{
				FATALErrorDD( "Could not create Direct3D texture.", res );
			}

			// Set key color
			if (pTex->flags & (XCGEN_TEX_KEYBLACK|XCGEN_TEX_KEYPURPLE|XCGEN_TEX_KEYWHITE) )
			{
				LPDIRECTDRAWSURFACE3	pDDS3;
				DDCOLORKEY				clrkey;

				if (pTex->flags & XCGEN_TEX_KEYBLACK)
					clrkey.dwColorSpaceLowValue = D3DRGB( 0, 0, 0 );
				else if (pTex->flags & XCGEN_TEX_KEYPURPLE)
					clrkey.dwColorSpaceLowValue = D3DRGB( 1.0f, 0, 1.0f );
				else
					clrkey.dwColorSpaceLowValue = D3DRGB( 1.0f, 1.0f, 1.0f );

				res = pDDSS->QueryInterface( IID_IDirectDrawSurface3, (LPVOID*)&pDDS3 );
				if (res !=DD_OK)
				{
					FATALErrorDD( "Cannot get DirectDrawSurface3 interface.", res ); 
				}
				pDDS3->SetColorKey( DDCKEY_SRCBLT, &clrkey );
			}

			// Load the texture
			//pDDS->AddRef();

			// Tell Direct 3D about it
			res = pDDSS->QueryInterface( IID_IDirect3DTexture, (LPVOID*)&pD3DTex );
			if (res!=DD_OK)
			{
				FATALErrorDD( "Cannot get Direct3DTexture interface.", res ); 
			}

			res = pDDSS->GetSurfaceDesc( &ddsd );
			if (res!=DD_OK)
			{
				FATALErrorDD( "Could not get surface description", res );
			}

			pDDS = pDDSS;
			res = DD_OK;
			Int	mip = level;

			while( res==DD_OK )
			{
				res = pDDS->Lock( NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL );
				if (res!=DD_OK)
				{
					FATALErrorDD( "Cannot lock texture surface", res );
				}
				pSurf = UCharPtr(ddsd.lpSurface);
				h = ddsd.dwHeight;
				w = ddsd.dwWidth;

				// Excruciatingly slow -- PLR TODO: Fix it
				// PLR TODO: Fix animated textures
				if (pTex->flags&XCGEN_TEX_ANIMATED)
					pMip = UCharPtr(pTex) + sizeof(XCGENTexture);
				else
					pMip = UCharPtr(pTex) + pTex->offMips[mip];
				for (j=0;j<h;j++)
				{
					for (k=0;k<w;k++)
					{
						color = *UCharPtr(pMip+j*(pTex->width>>mip)+k);
						*ShortPtr(pSurf+(j*ddsd.lPitch)+(k*sizeof(Short))) = ((pTex->cTable[0][color]&0xffe0)<<1) + (pTex->cTable[0][color]&0x1f);
					}
				}

				pDDS->Unlock( ddsd.lpSurface );

				res = pDDS->GetAttachedSurface( &ddsd.ddsCaps, &pDDSNext);
				pDDS->Release();

				pDDS = pDDSNext;
				
				if (res==DD_OK)
				{
					mip++;
				}
			}
	
			pTex->width >>= level;
			pTex->height >>= level;

			// PLR - TODO: Chop off the original texture data
			pTex->d3dinfo.pDDS = pDDSS;
			pTex->d3dinfo.pD3DTex = pD3DTex;
		}
	}
	else
	{
		pTex =XCGENTexturePtr (GetResData(pName,XCGEN_RESTYPE_TEXTURE));

		// Replace width and height with number of bits for each
		if (pTex->header.ref==1)
		{
			gStats.nTextures++;
			gStats.memTextures += ResourceSize( XCGENResourcePtr(pTex) );

			switch( pTex->width )
			{
				case 256:	pTex->width = 8; break;
				case 128:	pTex->width = 7; break;
				case 64:	pTex->width = 6; break;
				case 32:	pTex->width = 5; break;
				case 16:	pTex->width = 4; break;
				case 8:		pTex->width = 3;	break;
			}

			switch( pTex->height )
			{
				case 256:	pTex->height = 8; break;
				case 128:	pTex->height = 7; break;
				case 64:	pTex->height = 6; break;
				case 32:	pTex->height = 5; break;
				case 16:	pTex->height = 4; break;
				case 8:		pTex->height = 3; break;
			}

			//ChangeShadeTable( pTL[i].pTexture );

			if (!gRenderSurf.b555)
			{
				ConvertTextureTo565( pTex );
			}
		}
	}

	return( pTex );

	EXIT( "GetTexture" );
}			
			
Void InitNewLOD( MeshLODPtr pLOD )
{
	ENTER;

	Int					i;
	MeshTexListPtr		pTL;

	pLOD->maxdist *= 2.0f;
	pLOD->pVerts = MeshVertexPtr(UCharPtr(pLOD)+pLOD->offVerts);
	pLOD->pEdges = MeshEdgePtr(UCharPtr(pLOD)+pLOD->offEdges);
	pLOD->pTexts = MeshTexListPtr(UCharPtr(pLOD)+pLOD->offTexts);
	pLOD->pAttach = MeshAttachPtr(UCharPtr(pLOD)+pLOD->offAttach);
	pLOD->pRoot = MeshNodePtr(UCharPtr(pLOD)+pLOD->offRoot);

	//Load textures
	pTL = pLOD->pTexts;
	for (i=0;i<pLOD->nTextures;i++)
	{
		DEBUGAssert( strlen(pTL[i].name) );

		pTL[i].pTexture = GetTexture(pTL[i].name);
	}

	//Load attachments (will need pre-initialized manipulator list)
	//Remains to be seen if attachments should automatically load

	//TraverseBSP
	RecursiveInitNode( pTL, pLOD, pLOD->pRoot );

	EXIT( "InitNewLOD" );
}

Void RecursiveInitHull( UCharPtr pRef, MeshHullNodePtr pNode )
{
	ENTER;

	for (Int i=0;i<2;i++)
	{
		if (pNode->children[i]!=-1)
		{
			pNode->pChildren[i] = MeshHullNodePtr( pRef + pNode->children[i] );
			RecursiveInitHull( pRef, pNode->pChildren[i] );
		}
		else
		{
			if (i==0)
				pNode->pChildren[i] = HULL_NODE_EMPTY;
			else
				pNode->pChildren[i] = HULL_NODE_SOLID;
		}
	}
	EXIT( "RecursiveInitHull" );
}

Void InitHull( MeshHullPtr pHull )
{
	ENTER;

	pHull->pRoot = MeshHullNodePtr( UCharPtr(pHull) + pHull->root );

	RecursiveInitHull( UCharPtr(pHull), pHull->pRoot );

	EXIT( "InitHull" );
}

Void InitNewMesh( MeshPtr pMesh )
{
	ENTER;

	Int		i;

	for (i=0;i<XCGEN_MAX_LODS;i++)
	{
		// If an LOD exists, initialize its pointer and itself
		if (pMesh->offLOD[i]!=-1)
		{
			pMesh->pLOD[i] = MeshLODPtr(UCharPtr(pMesh)+pMesh->offLOD[i]);
			InitNewLOD( pMesh->pLOD[i] );
		}
		else
		{
			pMesh->pLOD[i] = NULL;
		}
	}
	for (i=0;i<4;i++)
	{
		if (pMesh->offHull[i]!=-1)
		{
			pMesh->pHull[i] = MeshHullPtr(UCharPtr(pMesh)+pMesh->offHull[i]);
			InitHull( pMesh->pHull[i] );
		}
		else
		{
			pMesh->pHull[i] = NULL;
		}
	}
	
	EXIT( "InitNewMesh" );
}

MeshPtr	GetMesh( XCGENResName pName )
{
	ENTER;

	MeshPtr	pMesh;

	pMesh = MeshPtr(GetResData( pName, XCGEN_RESTYPE_MESH ));

	// If this is the first time loaded, we must traverse the object
	//  and initialize all portions
	if (pMesh->header.ref==1)
	{
		gStats.nMeshes++;
		gStats.memMeshes += ResourceSize(XCGENResourcePtr(pMesh));
		InitNewMesh( pMesh );
	}

	return( pMesh );

	EXIT( "GetMesh" );
}

Void InsertObjectIntoWorld( ObjectInstancePtr pObjI )
{
	ENTER;

	ObjectInstancePtr	pO;

	if (gnTotalObjects>=MAX_OBJECTS)
		FATALError( "Maximim object limit reached." );

	pO = gpObjects->pNext;

	gpObjects->pNext = pObjI;
	pObjI->pNext = pO;

	if (pO)
		pO->pPrev = pObjI;

	pObjI->pPrev = gpObjects;

	gnTotalObjects++;
	gStats.nObjects++;

	EXIT( "InsertObjectIntoWorld" );
}

Void RemoveObjectFromWorld( ObjectInstancePtr pObjI )
{
	ENTER;

	ObjectInstancePtr	pO;

	pO = pObjI->pPrev;

	pO->pNext = pObjI->pNext;

	if (pObjI->pNext)
		pObjI->pNext->pPrev = pO;

	gnTotalObjects--;
	gStats.nObjects--;

	EXIT( "RemoveObjectFromWorld" );
}

ObjectInstancePtr CreateNewObject( XCGENResName pName, XCGENObjType type )
{
	ENTER;

	Int					i;
	ObjectInstancePtr	pObj;

	// Create and initialize the new object instance
	pObj = new ObjectInstance;
	memset( pObj, 0, sizeof(ObjectInstance) );

	MakeIdentity( pObj->obj.mat );
	pObj->obj.rot[0] = 0.0f;
	pObj->obj.rot[1] = 0.0f;
	pObj->obj.rot[2] = 0.0f;
	pObj->obj.rot[3] = 1.0f;
	pObj->obj.flags = XCGEN_OBJ_VISIBLE;
	pObj->obj.repeat = 1.0f;	// Prevents accidental divide by zero
	pObj->obj.alpha = 1.0f;

	switch( type )
	{
		case XCGEN_TYPENOTHING:
			break;
		case XCGEN_TYPE3DOBJ:
			pObj->pMesh = GetMesh( pName );
			pObj->objradius = pObj->pMesh->radius;
			pObj->obj.radius = pObj->objradius/2;
			// Initialize manipulators
			if (pObj->pMesh->nManips>0)
			{
				MeshLODPtr	pLOD;

				pObj->obj.nManips = pObj->pMesh->nManips;
				pObj->obj.pManips = new XCGENManipulator[pObj->pMesh->nManips];
				memset( pObj->obj.pManips, 0, sizeof(XCGENManipulator)*pObj->pMesh->nManips );

				// Retrieve Attachment ID's and place them in the manipulator list
				// Currently we'll only look at the first LOD.
				// -- PLR TODO: Perhaps there's a better way.
				pLOD = pObj->pMesh->pLOD[0];
				for (i=0;i<pObj->obj.nManips;i++)
				{
					pObj->obj.pManips[i].attachID = -1;
				}
				for (i=0;i<pLOD->nAttach;i++)
				{
					pObj->obj.pManips[pLOD->pAttach[i].manipID].attachID = i;
				}
			}
			break;
		case XCGEN_TYPESPRITE:
			pObj->pSprite = GetSprite( pName );
			pObj->obj.tsBegin = GetTickCount();
			pObj->obj.tsKill = pObj->obj.tsBegin + ((pObj->pSprite->nFrames-1) * 1000 / pObj->pSprite->speed);
			pObj->obj.flags |= XCGEN_OBJ_SPRITE;
			pObj->objradius = max(pObj->pSprite->width,pObj->pSprite->height) * pObj->pSprite->scale;
			pObj->obj.radius = pObj->objradius/2;
			break;
		case XCGEN_TYPEBEAM:
			pObj->obj.flags |= XCGEN_OBJ_BEAM;
			pObj->pBeam = new XCGENBeam;
			memset( pObj->pBeam, 0, sizeof(XCGENBeam) );
			break;
		default:
			DEBUGError( "Invalid object type" );
			break;
	}

	InsertObjectIntoWorld( pObj );

	pObj->bAlive = TRUE;

	return( pObj );
	
	EXIT( "CreateNewObject" );
}

Void RemoveObject( ObjectInstancePtr pObj )
{
	ENTER;

	Int		ref, size;
	Int		i, j;

	pObj->bAlive = FALSE;

	RemoveObjectFromWorld( pObj );
	RemoveAttachedEmitters( pObj );

	//Remove manipulators
	if (pObj->pMesh->nManips>0)
	{
		delete pObj->obj.pManips;
	}

	if ( pObj->obj.flags & XCGEN_OBJ_SPRITE )
	{
		size = ResourceSize( XCGENResourcePtr(pObj->pSprite) );
		ref = ReleaseResData( XCGENResourcePtr(pObj->pSprite) );
		if (ref==0)
		{
			gStats.nSprites--;
			gStats.memSprites -= size;
		}
	}
	else if ( pObj->obj.flags & XCGEN_OBJ_BEAM )
	{
		if (pObj->pBeam->pTex)
		{
			size = ResourceSize( XCGENResourcePtr(pObj->pBeam->pTex) );
			ref = ReleaseResData( XCGENResourcePtr(pObj->pBeam->pTex) );
			if (ref==0)
			{
				gStats.nTextures--;
				gStats.memTextures -= size;
			}	
		}
	}
	else
	{
		// Release textures if resource is about to be unloaded
		if (pObj->pMesh->header.ref==1)
		{
			for (j=0;j<4;j++)
			{
				// Check each LOD
				if (pObj->pMesh->pLOD[j]!=NULL)
				{
					for (i=0;i<pObj->pMesh->pLOD[j]->nTextures;i++)
					{
						XCGENTexturePtr		pT;
						HRESULT				res;

						pT = pObj->pMesh->pLOD[j]->pTexts[i].pTexture;

						if (gRenderSurf.bD3D && pT->header.ref==1)
						{
							// If a D3D Texture is about to be released for good
							/*res = pT->d3dinfo.pD3DTex->Release();
							if (res!=DD_OK)
							{
								FATALErrorDD( "Cannot release D3D texture", res );
							}*/ 
							// Surface seems to take care of the interface
							res = pT->d3dinfo.pDDS->Release();
							if (res!=DD_OK)
							{
								FATALErrorDD( "Cannot release D3D surface", res );
							}

						}

						size = ResourceSize( XCGENResourcePtr(pT) );
						ref = ReleaseResData( XCGENResourcePtr(pT) );
						if (ref==0)
						{
							gStats.nTextures--;
							gStats.memTextures -= size;
						}	
					}
				}
			}
		}
		
		size = ResourceSize( XCGENResourcePtr(pObj->pMesh) );
		ref = ReleaseResData( XCGENResourcePtr(pObj->pMesh) );
		if (ref==0)
		{
			gStats.nMeshes--;
			gStats.memMeshes -= size;
		}
	}

	EXIT( "RemoveObject" );
}	

