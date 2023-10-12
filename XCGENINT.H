/*******************************************************************
**    File: XCGENInt.h
**      By: Paul L. Rowan     
** Created: 970404
**
** Description:  Internal global header for XCGEN    
**
********************************************************************/
#ifndef	_XCGENINT_H_
#define _XCGENINT_H_

#define _XCGEN_INTERNAL_
#include "xcgen.h"
#include "d3d.h"
#include "xcinline.h"
#include "xcmath.h"
 
#include "xcresource.h"

//********************************************************************
//	-- DEBUGGING AND EXCEPTION HANDLING
//********************************************************************
extern Char	gErrorTxt[];	// This needs to be defined in the calling process

#ifdef _DEBUG

#include <malloc.h>

// DEBUGGING TOOLS
#define DEBUGLog(a)		(OutputDebugString(a))


#define ENTER			try {
#define EXIT(a)			}catch(CharPtr txt){		\
						strcat( txt, "\n\t" );		\
						strcat( txt, a );			\
						throw(txt);	}

#define ENTERTrap		try {
#define EXITTrapDD(a)	}catch( CharPtr pError ){		\
						strcat( pError, "\n\t" );		\
						strcat( pError, a );			\
						strcat( pError, "\t(TRAPPED)");	\
						DDScreenClose();				\
						MessageBox( NULL, pError, "Fatal Error", MB_OK );	\
						exit(FALSE);					\
						}catch( ... ){					\
						DDScreenClose();				\
						throw;}
#define EXITTrap(a)		}catch( CharPtr pError ){		\
						strcat( pError, "\n\t" );		\
						strcat( pError, a );			\
						strcat( pError, "\t(TRAPPED)");	\
						MessageBox( NULL, pError, "Fatal Error", MB_OK );	\
						exit(FALSE);					\
						}catch( ... ){					\
						throw;}

#define	DEBUGError(a)	{	 DebugBreak();\
							sprintf	(gErrorTxt,"Error:\t%s\nFile:\t%s\nLine:\t%d\n\nStack trace:", a, __FILE__, __LINE__);\
							throw(gErrorTxt);}
#define	DEBUGAssert(e)	if (!(e)){	DebugBreak();\
	sprintf	(gErrorTxt,"Assertion failed: \"%s\"\nFile:\t%s\nLine:\t%d\n\nStack trace:", #e, __FILE__, __LINE__);\
							throw(gErrorTxt);}
extern Int	gTotalMem, gMaxMem;


/*inline void* operator new(size_t nSize)
{
   void* p=malloc(nSize);
   gTotalMem += _msize(p);
   if (gTotalMem>gMaxMem)
	   gMaxMem = gTotalMem;
   return p;
}
 
inline void operator delete( void *p )
{
	gTotalMem -= _msize(p);
	free(p);
}*/
 
#else

// NO DEBUGGING TOOLS
#define DEBUGLog(a)

#define ENTER
#define EXIT(a)
#define ENTERTrap
#define EXITTrapDD(a)
#define EXITTrap(a)
#define DEBUGError(a)
#define	DEBUGAssert(e)

#endif

// FATAL RUNTIME EXCEPTIONS (RELEASE)
#define	FATALError(a)	{	sprintf	(gErrorTxt,"Error:\t%s\nFile:\t%s\nLine:\t%d\n", a, __FILE__, __LINE__);\
							throw(gErrorTxt);}
#define FATALErrorDD(a,b)	{	Char	num[128]; sprintf( num, "%s\n\tHRESULT: %08X", a, b ); FATALError(num); } 

// TIMER TOOLS
extern Long	gCycleCal;

#define TIMERBEGIN(a)	_asm 	_emit 0x0f _asm _emit 0x31	\
						_asm	sub		eax, a			\
						_asm	mov		a, eax

#define TIMEREND(a)		_asm 	_emit 0x0f _asm _emit 0x31	\
						_asm	sub		eax, 13				\
						_asm	sub		eax, a			\
						_asm	mov		a, eax	


//********************************************************************
//	-- MISCELLANEOUS DATA DEFINITIONS
//********************************************************************
#define MAX_OBJECTS				512
#define MAX_SPANS				60000
#define MAX_EDGES				2400
#define MAX_SURFS				1000
#define MAX_HEIGHT				600
#define MAX_RESTYPES			16
#define MAX_RESOURCES			256
#define MAX_ATTACHMENT_LEVELS	4
#define MAX_TOTAL_VERTICES		4000 
#define MAX_DYNAMIC_LIGHTS		8
#define MAX_POLYGON_VERTS		32
#define MAX_DUST				1024
#define MAX_PARTICLES			4096
#define MAX_D3DVERTICES			100
#define MAX_D3DPOLYBUFFERS		50

#define MAX_FRAME_TIME			67

#define Z_BUFFER_MAX			32768

#define NUM_ICONS				1
#define ICON_SLOW				0
//********************************************************************
//	-- RESOURCE MANAGEMENT
//********************************************************************

typedef struct
{
	XCGENResourcePtr	pRes;
	XCGENResName		name;
} ResEntry, *ResEntryPtr;

typedef struct
{
	XCGENResType	type;
	ResEntry		resList[MAX_RESOURCES];	
} ResArray, *ResArrayPtr;

Long				ResourceSize( XCGENResourcePtr pRes );

XCGENResourcePtr	GetResData( CharPtr pName, CharPtr pType );	// Load a resource
Int					ReleaseResData( XCGENResourcePtr pRes );

VoidPtr				GetData( CharPtr pName, LongPtr pSize );		// Just load a file
Void				ReleaseData( VoidPtr pData );	

extern ResArray	gResMap[MAX_RESTYPES];

XCGENTexturePtr GetTexture( XCGENResName pName );

//********************************************************************
//	-- GLOBAL VARIABLES
//********************************************************************
extern XCGENStats			gStats;
extern Char					szDatabasePath[];
extern Mat_3				mIDENTITY;
extern XCGENTimestamp		gLastMotionTick;

//********************************************************************
//	-- PARTICLE SYSTEM
//********************************************************************
typedef struct Particle	// Should Align on cache boundaries
{
	Vec_3				pos;
	Vec_3				vel;
	Long				flags;
	ShortPtr			pColor;
	XCGENTimestamp		tsBegin, tsDie;
	Particle			*pNext;
	Long				count;
} Particle, *ParticlePtr;

// Dust spacing 
#define SPACING				1024.0f

extern Vec_3		gDust[MAX_DUST];
extern Particle		gParticles[MAX_PARTICLES];
extern ParticlePtr	gpActiveParts, gpFreeParts;

typedef struct AttachedEmit
{
	AttachedEmit		*pNext;
	XCGENParticleEmit	emit;
	Vec_3				offset;
} AttachedEmit, *AttachedEmitPtr;

Void	DrawParticles( Void );
Void	InitParticles( Void );
Void	ProcessParticles( XCGENTimestamp stamp );
Void	Particles( XCGENParticleDefPtr pDef, XCGENTimestamp stamp );
Void	EmitParticles( XCGENParticleEmitPtr pDef, XCGENTimestamp stamp );

//********************************************************************
//	-- OBJECTS
//********************************************************************
// This is deliberately just like a XCGENObject up top, but
// contains internal goodies afterwards that I don't feel like
// giving to the outside world.  BEWARE and BE CAREFUL to cast
// appropriately! - PLR  (trained professional, don't try this at home)
typedef struct ObjectInstance
{
	XCGENObject			obj;	// Must appear at top to match external data
	ObjectInstance		*pPrev, *pNext;	// Master object list pointers
	ObjectInstance		*pBNext;		// BSP list for objects within a node
	union
	{
		MeshPtr				pMesh;
		AnimSpritePtr		pSprite;
		XCGENBeamPtr		pBeam;
	};
	Bool				bAlive;			// Flag to tell that an object is deleted
	Float				objradius;		// Object's radius
	Float				curradius;		// Temp storage for radius
	Vec_3				curproj;		// Temp storage for projected screen x,y
	Bool				bPosStored;		// Is framepos valid
	Vec_3				framepos;		// Actual position for this frame
	Vec_3				storepos;		// actual position at begin of motion
	Mat_3				framemat;		// Composite Matrix to use for this frame
	Int					attachID;		// Attached at manipulator id 
	MeshAttachPtr		pAttachPt;		// Pointer to attachment point for this child object
	Long				region;			// Region ID for collision
	Float				movebbox[6];	// Bounding box of move for this frame
	ObjectInstance		*pGroup[3];		// Occupied region groupings (one per axis)
	ObjectInstance		*pCollide;		// Collides with
	MeshNodePtr			pCollideNode;	// At node (NULL==non bsp collide)
	Vec_3				collideNorm;	// Normal vector of collision (BSP)
	Vec_3				collidePos;		// World coordinate of collision point
	Float				tCollide;		// Time of collision
	AttachedEmitPtr		pAttach;		// Attached particle emitter
} ObjectInstance, *ObjectInstancePtr;

// Master object list
extern Int					gnTotalObjects;
extern ObjectInstancePtr	gpObjects;

ObjectInstancePtr	CreateNewObject( CharPtr pName, XCGENObjType type );
Void				RemoveObject( ObjectInstancePtr pObj );
Void				AttachEmitter( ObjectInstancePtr pObj, XCGENParticleEmitPtr pEmit, Vec_3 offset, XCGENTimestamp stamp ); // In xcparticle.cpp
Void				RemoveAttachedEmitters( ObjectInstancePtr pObj ); // In xcparticle.cpp
Void				ProcessAttachedEmits( ObjectInstancePtr pObj, XCGENTimestamp stamp ); // In xcparticle.cpp

//********************************************************************
//	-- BEAM OBJECTS
//********************************************************************

Void				InitBeamObject( Void );
ObjectInstancePtr	CreateBeam( XCGENBeamPtr pTemplate );
Void				RenderBeam( ObjectInstancePtr pBeam );
ObjectInstancePtr	ClipLineToWorld( ObjectInstancePtr pObj, Float radius, Vec_3 p0, Vec_3 p1 );

//********************************************************************
//	-- RENDERING SURFACE
//********************************************************************
typedef struct
{
	Bool				bD3D;
	Bool				bAGP;			// D3D card is an agp card
	Bool				b555;
	Bool				bRaw;
	Bool				bZBuff;
	Bool				bZBWrite;
	VoidPtr				pBitmap;
	Long				rowbytes;
	LPDIRECTDRAW		pDD;			// Pointer to DirectDraw object (NULL if raw bitmap)
	LPDIRECTDRAWSURFACE	pDDS;			// Pointer to DirectDrawSurface to render to (NULL if raw)
	LPDIRECTDRAWSURFACE	pZDDS;			// Pointer to Z-Buffer
	LPDIRECT3DDEVICE	pD3DDev;		// Pointer to Direct3DDevice if 3D hardware present
	LPDIRECT3DDEVICE2	pD3DDev2;		// Pointer to Direct3DDevice2 if 3D hardware present
	ShortPtr			pSurf;			// Locked pointer to surface
	ShortPtr			pZBuff;			// Locked pointer to z-buffer
	Long				surfpitch;		// surface pitch in 16-bit words
	Long				zbuffpitch;		// z-buffer pitch in 16-bit words
} RenderSurface, *RenderSurfacePtr;

extern RenderSurface		gRenderSurf;

Void LockRenderSurface( Void );
Void UnlockRenderSurface( Void );

//********************************************************************
//	-- CAMERA AND VIEWPORT DEFINITION
//********************************************************************
#define XCGEN_VPFLAG_NOTEXTURE		XCGEN_VIEW_NOTEXTURES	// Do not texture this viewport
#define XCGEN_VPFLAG_NOBACKGROUND	XCGEN_VIEW_NOBACKGROUND	// Don't draw background objects
#define XCGEN_VPVLAG_NOPARTICLES	XCGEN_VIEW_NOPARTICLES	// Don't draw particles
#define XCGEN_VPFLAG_TOPDOWN		0x00000008	// Viewport orientation is top down
#define XCGEN_VPFLAG_ZBUFFER		0x00000010	// ZBuffer is defined

typedef struct
{
	Long		width, height;
	Float		fov, maxscale, maxscaleinv;
	Float		xscale, yscale;
	Float		xscaleinv, yscaleinv;
	Float		xcenter, ycenter;
	Long		xoff, yoff;
	Long		pitch;		
	Long		flags;
	UCharPtr	pData;
	UCharPtr	pZBuffer;
} Viewport, *ViewportPtr;

typedef enum
{
	XCGEN_ZBUFF_NONE	=	0,
	XCGEN_ZBUFF_WRITE	=	1,
	XCGEN_ZBUFF_FULL	=	2
} ZBufferMode;

extern ZBufferMode	gZBuffMode;
extern XCGENCamera	gCamera;
extern Viewport		gViewport;

extern ObjectInstancePtr	gObjZOrder[MAX_OBJECTS];
extern Double				gObjDist[MAX_OBJECTS];
extern Long					gnObjects;

extern Float	gFrustum[4][4];		// Four clipping planes (x,y,z,d)

Void	BuildFrustum( Void );
Void	RenderCamera( Void );

//********************************************************************
//	-- RENDER
//********************************************************************
#define LEFT_CLIP_PLANE		0x00000001	
#define RIGHT_CLIP_PLANE	0x00000002

typedef struct ClipPlane
{
	ClipPlane	*pNext;
	Long		flags;
	Float		plane[4];	// (x,y,z,d)
} ClipPlane, *ClipPlanePtr;

typedef struct 
{	
	ObjectInstancePtr	pObj;			// Current object instance
	MeshPtr				pMesh;			// Mesh data
	MeshLODPtr			pLOD;			// Current MeshLOD
	MeshVertexPtr		pVerts;			// Vertex data
	MeshEdgePtr			pEdges;			// Edge data
	Float				campos[3];		// Camera position (object space)
	ClipPlane			frustum[4];		// Clipping planes (object space) (x,y,z,d)
	Long				pindexes[4][6];	// Indirection into minmaxs for this frustum
	Vec_3				XLight;
	Int					nLights;
	Vec_3				Lights[MAX_DYNAMIC_LIGHTS];
	Float				LightRad[MAX_DYNAMIC_LIGHTS];
	Float				LightDist[MAX_DYNAMIC_LIGHTS];
	UChar				LightColor[MAX_DYNAMIC_LIGHTS][4];
} RenderStack, *RenderStackPtr;

extern RenderStack			gpObjStack[MAX_ATTACHMENT_LEVELS];
extern Long					gnCurObject;
extern RenderStackPtr		gpStack;
extern Long					gSurfKey;

Void GenerateFrustum( Void );
Void RenderScene( Void );
Void RenderObject( ObjectInstancePtr pObj );


//********************************************************************
//	-- D3DPOLYBUFFER
//********************************************************************
typedef struct
{
	XCGENTexturePtr	pTex;
	Int				nVerts, nIndices;
	D3DTLVERTEX		verts[MAX_D3DVERTICES];
	UShort			indices[MAX_D3DVERTICES*3];
} D3DPolyBuff, *D3DPolyBuffPtr;

extern Int		gnBuffs;

Void			BufferD3DPoly( XCGENTexturePtr pTex, D3DTLVERTEX *pVerts, Int nVerts );
Void			FlushD3DPolyBuffers( Void );

//********************************************************************
//	-- SPANS
//********************************************************************
typedef struct Span	// ----- 32 bytes (MUST ALIGN)
{
	Int			x, y;
	Int			count;
	Int			s;
	Int			ds;
	Span		*pNext;			
	Float		zinv, uoverz, voverz;
} Span, *SpanPtr;

typedef union 
{
	Span			spans[MAX_SPANS];
	D3DPolyBuff		polybuff[MAX_D3DPOLYBUFFERS];
} SharedBuffer, *SharedBufferPtr;

extern SharedBuffer	gBuff;

extern "C"
{
	Void EmitTexturedSpan( Void );
	Void EmitZWTexturedSpan( Void );
	extern UCharPtr gpScreen;
	extern SpanPtr	gpSpan;
	extern VoidPtr	gpSurf;
	extern UCharPtr gpTexture;
	extern UCharPtr	gpCTable;
	extern Int		SFixed;
	extern Int		DeltaS;
	extern Int		DeltaS16;
	extern Int		uvMask;
	extern Int		uBits;
	extern Int		uShift;
}

//********************************************************************
//	-- SURFACES
//********************************************************************
typedef struct Surf // ----- ??? bytes (MUST ALIGN)
{
	Surf				*pNext, *pPrev;
	Int					visxstart;
	Int					shade;		// Shade value for left edge
	Int					shadex;
	Int					dshade;		// Delta for shade across surface
	Int					state;
	Int					miplevel;
	MeshFacePtr			pFace;
	ObjectInstancePtr	pObj;
	Float				zinv00, zinvstepx, zinvstepy;
	Float				uoverz00, uoverzstepx, uoverzstepy;
	Float				voverz00, voverzstepx, voverzstepy;
	Float				zinvstepx8, uoverzstepx8, voverzstepx8;
	Int					dudxmod, dvdxmod;
	Int					uoff, voff;
	XCGENTexturePtr		pTex;
	SpanPtr				pSpans;
	Short				shades[16];
} Surf, *SurfPtr;

extern Surf		gSurfs[];
extern Int		gnSurfaces;
extern SurfPtr	gpAvailSurf;

Void DrawSurfaces( Void );
Void CalculateGradients( SurfPtr pSurf, Vec_3 *pP, Vec_3 *pT );

//********************************************************************
//	-- EDGE SORTING
//********************************************************************
typedef struct
{
	Float	p[3];	// Position in screen coordinates
	union
	{
		Float	n[3];	// Transformed normal vector
		UChar	c[12];	// Color information	[ r,g,b,s (diffuse) r,g,b,s (spectral) ?,?,?,? (?) ]
		Long	s[3];
	};
	Float	t[2];
} XFormedVertex, *XFormedVertexPtr;

extern XFormedVertex		gXFormVert[MAX_TOTAL_VERTICES];
extern Short				gnVertices;

// PLR - TODO: I don't think this is really aligned (44 bytes?)
typedef struct Edge	// ----- 32 bytes (MUST ALIGN)
{
	Int			x;
	Int			xstep;
	Int			s;
	Int			sstep;
	Short		v[2];		// Vertex indexes
	SurfPtr		pSurf[2];	// Leading and trailing surfaces
	Edge		*pNext, *pPrev, *pNextRemove;
	Int			flags;		// Clip result flags
} Edge, *EdgePtr;

extern Edge		gEdgeHead;
extern Edge		gEdgeTail;
		
extern Edge		gEdges[];
extern Int		gnEdges;

extern Edge		gNewEdges[];
extern EdgePtr	gpRemoveEdges[];

extern Edge		gMaxEdge;

Void	AddFace( MeshFacePtr pFace, Int clipflags );
Void	ClearEdgeLists( Void );
Void	ScanEdges( Void );

//********************************************************************
//	-- LIGHTING
//********************************************************************
Void GetVertexShade( Vec_3 norm, Vec_3 light, Float ldist, Bool selfIllum, FloatPtr pDiff );

//********************************************************************
//	-- CLIPPING AND CULLING
//********************************************************************
Int		ClipLineToFrustum( Vec_3 p0, Vec_3 p1, Vec_3 pp0, Vec_3 pp1 );

//********************************************************************
//	-- PROJECT AND TRANSFORM
//********************************************************************
Void	TransformWorldVector( Vec_3 dst, Vec_3 v );
Void	NonObjProjectVector( Vec_3 dst, Vec_3 v, FloatPtr pzInv );
Void	TransformProjectVector( Vec_3 dst, Vec_3 v );
Void	TransformViewVector( Vec_3 dst, Vec_3 v );

//********************************************************************
//	-- PHYSICS
//********************************************************************
#define HULL_NODE_EMPTY	MeshHullNodePtr(0xEFFFFFFF)
#define HULL_NODE_SOLID MeshHullNodePtr(0xEFFFFFFE)

// Collision regionalizer
typedef struct OccupiedRegion
{
	OccupiedRegion		*pPrev, *pNext;
	ObjectInstancePtr	pObj;
	Float				begin, end;
} OccupiedRegion, *OccupiedRegionPtr;

extern Int				gnRegions[3];
extern OccupiedRegion	gRegions[3][MAX_OBJECTS];
extern OccupiedRegion	gRegionHead[3];

Void	DoMotion( XCGENTimestamp stamp );

//********************************************************************
//	-- MISCELLANEOUS
//********************************************************************
extern Vec_3	gLight;
extern UChar	gLR, gLG, gLB;

Void	DrawZBuffLine( PointPtr p0, PointPtr p1, UChar r, UChar g, UChar b );
Void	DrawSprite( ObjectInstancePtr pObj );
Void	CalculateFramePosition( Void );
Void	CalculateRealPosition( Void );
Void	DisplayIcon( Int type );
Void	DisplayStats( Void );
Void	LoadIcons( Void );
Void	ExtractLights( Void );
Void	InitDust( Void );
Void	DrawDust( Void );
Void	CalibrateTimer( Void );
Void	ApplyRotation( ObjectInstancePtr pObj, Float roll, Float pitch, Float yaw );
Int		ClipLineToWorldFrustum( Vec_3 p0, Vec_3 p1, Vec_3 pp0, Vec_3 pp1 );
Void	RenderWorldLine( Vec_3 p0, Vec_3 p1, UChar r, UChar g, UChar b );
Void	RenderRealWorldLine( Vec_3 p0, Vec_3 p1, UChar r, UChar g, UChar b );
Void	DrawBox( Vec_3 v0, Vec_3 v1, UChar r, UChar g, UChar b, Bool bObject );

//********************************************************************
//	-- HARDWARE & DIRECT 3D
//********************************************************************
Void				CheckAndInitDirect3D( Void );
Void				D3DRenderFace( MeshFacePtr pFace, Int clipflags );
Void				InitD3DTextureCache( Void );
D3DTEXTUREHANDLE	GetD3DTexture( XCGENTexturePtr pTex );
Void				D3DDrawDust( Void );
Void				D3DDrawParticles( Void );
Void				D3DTestCaps( Void );
Void				FlushD3DTextureCache( Void );

extern XCGENStats	gStats;
extern Short		gIcons[1][32][32];

#endif

