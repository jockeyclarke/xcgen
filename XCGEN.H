/*******************************************************************
**    File: XCGEN.h
**      By: Paul L. Rowan     
** Created: 970402
**
** Description:      
**
********************************************************************/
#ifndef	_XCGEN_H_
#define _XCGEN_H_

#include "basetype.h"
#include "ddraw.h"
#include "d3d.h"
#include <string.h>
#include <stdio.h>

#ifdef _XCGEN_INTERNAL_
	#define XCGENDLL	_declspec( dllexport )
#else
	#define XCGENDLL	_declspec( dllimport )
#endif

//********************************************************************
//	-- GENERAL TYPES
//********************************************************************
typedef Float	Vec_3[3];
typedef Float	Mat_3[3][3];
typedef Float	Plane_3[4];
typedef Float	Quat[4];

typedef enum
{
	XCGEN_OKAY		=	0,
	XCGEN_ERROR		=   1
} XCGENResult;

typedef Char	XCGENResName[12];
typedef Char	XCGENResType[4];
typedef Void	*XCGENResPtr;

typedef struct XCGENPosOrient	
{
	Vec_3	pos;	// position
	Mat_3	mat;	// orientation
} XCGENPosOrient, *XCGENPosOrientPtr;

typedef Int		XCGENTimestamp;		// Milliseconds

//********************************************************************
//	-- INITS
//********************************************************************
XCGENDLL XCGENResult XCGENSetDatabaseDir( CharPtr pPath );

//********************************************************************
// -- BRAINS CALLBACK
//********************************************************************
// Forward reference for parameters
#pragma warning( disable : 4091 )  // Disable no symbol warning
typedef struct XCGENObject;
#pragma warning( default : 4091 )  // Enable no symbol warning

typedef XCGENObject *XCGENObjectPtr;

// Proc receives Source object, Collided Object, and collision time
// Proc should return TRUE iff an either object was deleted as a result
//	of the collision.
typedef	Bool	(*XCGENCallback)(XCGENObjectPtr,XCGENObjectPtr,XCGENTimestamp);

#define XCGEN_RESTYPE_MESH		"MSH"
#define XCGEN_RESTYPE_TEXTURE	"TXR"
#define XCGEN_RESTYPE_SPRITE	"SPT"

//********************************************************************
// -- RESOURCES
//********************************************************************
typedef struct XCGENResource		// ----- 32 bytes
{
	XCGENResName	name;	// Resource name (12 bytes)
	XCGENResType	type;	// Resource type (4 bytes)
	Long			ref;	// Reference count (hot loaded at runtime)
	Long			size;
	Long			pad[2];
} XCGENResource, *XCGENResourcePtr;

//********************************************************************
//	-- TEXTURE RESOURCE DATA DEFINITION
//********************************************************************
#define XCGEN_MAX_MIPLEVELS	4
#define XCGEN_MAX_SHADES	16
#define XCGEN_TEX_AMBIENT	20		// shade/AMBIENT = lighting value

#define XCGEN_TEX_ANIMATED		0x0001	// Texture is an animated texture
#define XCGEN_TEX_KEYBLACK		0x0002	// Ignore black areas ( 0, 0, 0 )
#define XCGEN_TEX_KEYPURPLE		0x0004	// Ignore purple areas (255, 0, 255)
#define XCGEN_TEX_KEYWHITE		0x0008	// Ignore white areas (255, 255, 255 )

typedef struct XCGENTexture
{
	XCGENResource	header;
	Short			width, height;					// Width and height of texture
	Short			color;							// General color for degenerative mip, dot, or line
	Short			flags;
	Short			cTable[XCGEN_MAX_SHADES][256];  // Fade table (adjusted for 555, 565)
	union
	{
		Long			offMips[XCGEN_MAX_MIPLEVELS];	// Offset to mip levels
		struct 
		{
			LPDIRECTDRAWSURFACE	pDDS;
			LPDIRECT3DTEXTURE	pD3DTex;
			Long				pad[2];
		}d3dinfo;
		struct
		{
			Long		nFrames;		// Number of frames
			Long		tsFrame;		// Milliseconds per frame
			Long		pad[2];
		}animtex;
	};
} XCGENTexture, *XCGENTexturePtr;

XCGENDLL XCGENResult XCGENNewTexture( XCGENTexturePtr *pTex, XCGENResName pName );

//********************************************************************
// -- PARTICLE DEFINITIONS
//********************************************************************
#define XCGEN_PART_LIGHT	0x00000001	// Particle has a halo of light

typedef struct XCGENParticleDef
{	
	Long				count;				// number of particles
	Long				flags;				// particle flags
	ShortPtr			pRamp;				// color ramp (always 16 colors 555 or 565)
	Float				duration;			// Lifetime of particle (s)
	Float				randlife;			// lifetime adjust (random effect - duration)
	Vec_3				pos;				// position, delta position (m/s)
	Float				scatter;			// scatter value (m) (random effect - pos) 
	Vec_3				vel;				// Base velocity
	Float				drag;				// scale base velocity (random effect - vel
	Float				spread;				// spread factor (m/s) (random effect - vel)
	Float				minspread;			// Clamp spread (low value m/s)
	Float				maxspread;			// Clamp spread (high value m/s)
	Float				squash;				// Flatten the positions and velocities (0.0:none - 1.0:flat)
	Vec_3				vsquash;			// Vector to flatten on
} XCGENParticleDef, *XCGENParticleDefPtr;

// A particle emission is a set of particle def's over time
typedef struct XCGENParticleEmit
{
	XCGENParticleDefPtr	pDef;		// Pointer to particle definition
	Float				rate;		// Rate of emission (part/s)
	Vec_3				pos;		// Final position
	Vec_3				vel;		// Final base velocity
	Vec_3				axis;		// Spin axis
	Vec_3				spinvec;	// Vector to spin around axis
	Vec_3				spinpos;	// Position to spin around axis
	Long				drotation;	// angle rotation delta
	Long				count;		// Internal 16.16 Fixed point (# to emit)
	XCGENTimestamp		sstamp;		// Internal use saved timestamp
	Long				srotation;	// Internal use saved rotation angle
	Vec_3				spos;		// Internal use saved position
} XCGENParticleEmit, *XCGENParticleEmitPtr;

XCGENDLL XCGENResult XCGENParticles( XCGENParticleDefPtr pDef, XCGENTimestamp stamp );
XCGENDLL XCGENResult XCGENEmitParticles( XCGENParticleEmitPtr pDef, XCGENTimestamp stamp );

//  This will dynamically attach a particle emitter to an object at position offset
//  The emitter will be copied, so a seperate instance need not be passed each
//		time the function is called.  Forever after, the generation of particles
//		by this emitter will be managed by XCGEN.
XCGENDLL XCGENResult XCGENAttachEmitter( XCGENObjectPtr pObj, XCGENParticleEmitPtr pEmit, Vec_3 offset, XCGENTimestamp stamp );


//********************************************************************
// -- OBJECT MANIPULATORS
//********************************************************************
typedef enum
{
	XCGEN_MANIP_IGNORE		=	0,
	XCGEN_MANIP_VECTOR		=	1,
	XCGEN_MANIP_OBJECT		=	2,
	XCGEN_MANIP_FACE		=	3,
	XCGEN_MANIP_NODE		=	4,
	XCGEN_MANIP_TEXTURE		=	5,
	XCGEN_MANIP_CALLBACK	=	6,
	XCGEN_MANIP_PARTICLE	=	7
} XCGENManipType;

#define XCGEN_MANIP_FACE_HIDDEN		0x00000001	// Don't draw the face
#define XCGEN_MANIP_FACE_SELFILLUM	0x00000002	// Face is fully lit

#define XCGEN_MANIP_NODE_HIDDEN		0x00000001	// Don't draw this node

// Manipulates objects -- Each instance needs a list of its own (externally maintained)
typedef struct XCGENManipulator
{
	XCGENManipType	type;
	Int				attachID;				// ID for attachment info. (-1 if none)
	union
	{
		XCGENObjectPtr			pObj;		// Pointer to sub-object
		Long					faceFlags;	// Face drawing flags
		Long					nodeFlags;	// BSP node drawing flags
		XCGENTexturePtr			pTexture;	// Pointer to texture to use
		XCGENCallback			pFunc;		// Pointer to callback 
		XCGENParticleEmitPtr	pParticle;	// Pointer to particle emission
	};
} XCGENManipulator, *XCGENManipulatorPtr;

//********************************************************************
// -- BEAM OBJECTS
//********************************************************************
// Special case beam objects
typedef struct XCGENBeam
{
	Vec_3				head, tail;
	Vec_3				vhead, vtail;
	Vec_3				shead, stail;
	Float				hwidth, twidth;
	XCGENTexturePtr		pTex;
	UChar				r, g, b;
} XCGENBeam, *XCGENBeamPtr;

XCGENDLL XCGENResult XCGENNewBeam( XCGENObjectPtr *ppObj, XCGENResName pTex, XCGENBeamPtr pTemplate );
XCGENDLL XCGENResult XCGENGetBeam( XCGENObjectPtr pObj, XCGENBeamPtr *ppBeam );

// LINE OF SIGHT ( RETURN: *ppObj = object hit, end = hit point )
XCGENDLL XCGENResult XCGENLineOfSight( XCGENObjectPtr *ppObj, Vec_3 start, Vec_3 end );


//********************************************************************
// -- OBJECTS
//********************************************************************
typedef enum
{
	XCGEN_TYPENOTHING		=	0,		// Dummy object
	XCGEN_TYPE3DOBJ			=	1,		// Regular object (base, ship, missile)
	XCGEN_TYPESPRITE		=   2,		// Sprite object
	XCGEN_TYPEBEAM			=	3		// Beam object
} XCGENObjType;

// WARNING:  There is a special restriction currently regarding attached
//				sub-objects.  The engine assumes that any attached object
//				instance has been created after its parent.  

// WARNING:  Some of these flags may not be changeable
#define XCGEN_OBJ_VISIBLE			0x00000001	// Visibility
#define XCGEN_OBJ_SELFILLUM			0x00000002	// Always fully lit
#define XCGEN_OBJ_SPRITE			0x00000004	// Is it a sprite?
#define XCGEN_OBJ_CAMRELATIVE		0x00000008	// Positions relative to camera, not world (spatially repeated objects)
#define XCGEN_OBJ_SOLIDHULL			0x00000010	// Use prebuilt BSP hull
#define XCGEN_OBJ_SOLIDRADIUS		0x00000020	// Collide at extents only
#define XCGEN_OBJ_SOLIDBSP			0x00000040	// Collide at face level
#define XCGEN_OBJ_SOLIDBBOX			0x00000080	// Collide at bound box extents
#define XCGEN_OBJ_LIGHTSOURCE		0x00000100	// Object gives off light 
#define XCGEN_OBJ_ATTACHED			0x00000200	// Object is attached to another object (posorient relative to attach pt)
#define XCGEN_OBJ_BACKGROUND		0x00000400	// All background obejcts are rendered first and relative to camera
#define XCGEN_OBJ_BASEOBJ			0x00000800	// Big and immoble
#define XCGEN_OBJ_NODAMPVEL			0x00001000	// Do not dampen velocity
#define XCGEN_OBJ_MASSLESS			0x00002000	// Object is light as a feather
#define XCGEN_OBJ_CAMFACING			0x00004000	// Object always faces camera (points along z)
#define XCGEN_OBJ_MATONLY			0x00008000	// Object only uses its matrix for rotation
#define XCGEN_OBJ_BEAM				0x00010000	// Object is a beam object

typedef struct XCGENObject
{
	Int					flags;			// Flags
	Quat				rot;			// Rotational quaternions
	Int					offHPR[3];		// Orientation offset (HPR: 0 -> UNITANGLE-1)
	Mat_3				mat;			// Orientation matrix
	Vec_3				pos;			// Position in world coordinates
	Vec_3				velocity;		// Velocity (meters/second)
	Vec_3				thrust;			// Acceleration (meters/second*second)
	Vec_3				rotaxis;		// Axis about which object rotates
	Float				angvel;			// Angular velocity (UNITANGLE/second)
	Float				light;			// Light radius extent (meters)
	UChar				lcolor[4];		// Light color (hardware only) ( 0x00rrggbb )
	Float				repeat;			// Repeat scale (CAMRELATIVE only)
	Float				ambient;		// Ambient lighting on object
	Float				alpha;			// Object's alpha value (HW only 0.0-1.0)
	Float				radius;			// Size of object for collisions
	Long				nManips;		// Number of manipulators
	XCGENObject			*pOwner;		// Pointer to object's owner
	XCGENObject			*pParent;		// Pointer to object's parent (ATTACHED objects only)
	XCGENManipulatorPtr	pManips;		// Pointer to this object's manipulators
	XCGENTimestamp		tsBegin;		// Timestamp for creation of object
	XCGENTimestamp		tsUpdate;		// Timestamp of latest update (milliseconds)
	XCGENTimestamp		tsKill;			// If non-zero, auto delete at time
	XCGENCallback		pKill;			// Auto-deletion callback
	XCGENTimestamp		tsThink;		// Perform auto-think
	XCGENCallback		pThink;			// Thinking callback
	XCGENCallback		pCollide;		// Collision callback
	VoidPtr				pUser;			// Pointer to user data
} XCGENObject, *XCGENObjectPtr;

XCGENDLL XCGENResult XCGENNewObject( XCGENObjectPtr *pObject, XCGENResName pName, XCGENObjType type );
XCGENDLL XCGENResult XCGENDeleteObject( XCGENObjectPtr pObject );
XCGENDLL XCGENResult XCGENDeleteAllObjects( Void );
XCGENDLL XCGENResult XCGENRotateObject( XCGENObjectPtr pObj, Float roll, Float pitch, Float yaw );
XCGENDLL XCGENResult XCGENAttachObject( XCGENObjectPtr pParent, XCGENObjectPtr pAttach, Int ID );
XCGENDLL XCGENResult XCGENDetachObject( XCGENObjectPtr pAttach );
// Get information about an attament point
XCGENDLL XCGENResult XCGENGetAttach( XCGENObjectPtr pObj, Int manipID, Vec_3 pos, Mat_3 mat );
// Get an attached object's position and orientation in the world
XCGENDLL XCGENResult XCGENGetRealPosition( XCGENObjectPtr pObj, Vec_3 pos, Mat_3 mat );
// Transform a vector into an object's local coordinate system and vice-versa
XCGENDLL XCGENResult XCGENWorldToLocal( XCGENObjectPtr pObj, Vec_3 vec, Vec_3 xvec );
XCGENDLL XCGENResult XCGENLocalToWorld( XCGENObjectPtr pObj, Vec_3 vec, Vec_3 xvec );
// Transform a vector into an object's local orientation and vice-versa
XCGENDLL XCGENResult XCGENOrientWorldToLocal( XCGENObjectPtr pObj, Vec_3 vec, Vec_3 xvec );
XCGENDLL XCGENResult XCGENOrientLocalToWorld( XCGENObjectPtr pObj, Vec_3 vec, Vec_3 xvec );

// Set the parallax light source
// Software mode uses the red component for brightness
XCGENDLL XCGENResult XCGENSetGlobalLightSource( Vec_3 vec, UChar r, UChar g, UChar b );
//********************************************************************
// -- UNIVERSE
//********************************************************************
typedef struct XCGENUniverse
{
	Int				nBackdrops;
	XCGENResName	*pBackdrops;	// Points to an array of XCGENResNames
	Int				nBases;
	XCGENResName	*pBases;		// Points to an array of XCGENResNames
	Int				nObjects;
	XCGENResName	*pObjects;		// Points to an array of XCGENResNames
} XCGENUniverse, *XCGENUniversePtr;

XCGENDLL XCGENResult XCGENResetUniverse( Void );
XCGENDLL XCGENResult XCGENInitUniverse( XCGENUniversePtr pUniverse );

//********************************************************************
// -- VIEWPORT MAPPING
//********************************************************************
typedef struct	
{
	LPDIRECTDRAW		pDD;			// Pointer to DirectDraw object (NULL if raw bitmap)
	LPDIRECTDRAWSURFACE	pDDS;			// Pointer to DirectDrawSurface to render to (NULL if raw)
	LPDIRECTDRAWSURFACE	pZDDS;			// Pointer to Z-Buffer Surface
	LPDIRECT3DDEVICE	pD3DDev;		// Pointer to Direct3DDevice if 3D hardware present
	LPDIRECT3DDEVICE2	pD3DDev2;		// Pointer to Direct3DDevice2 if 3D hardware present
	VoidPtr				pBitmap;		// Pointer to raw bitmap (overridden by pDD)
	Long				rowBytes;		// If raw bitmap, rowByte value (negative for bottom up orientation)
	UCharPtr			pZBuffer;		// Pointer to 16-bit Z buffer (sWidth * sHeight)
} XCGENSurface, *XCGENSurfacePtr;

XCGENDLL XCGENResult XCGENSetRenderSurface( XCGENSurfacePtr pSurf );

#define XCGEN_VIEW_NOTEXTURES		0x00000001	// Draw flat shaded random colors
#define XCGEN_VIEW_NOBACKGROUND		0x00000002	// Don't draw background objects
#define XCGEN_VIEW_NOPARTICLES		0x00000004	// Don't draw particles
#define XCGEN_VIEW_BBOXOBJECT		0x00000008	// Draw object bounding boxes
#define XCGEN_VIEW_BBOXMOVE			0x00000010	// Draw movement boxes
#define XCGEN_VIEW_BBOXREGIONS		0x00000020	// Draw collision regions
#define XCGEN_VIEW_SHOWSTATS		0x00000040	// Display statistics

typedef struct
{
	Long				xOff, yOff;		// Top, left offset 
	Long				sWidth, sHeight;// Viewport width and height
	Float				yAspect;		// Aspect ratio of pixels
	Float				zScale;			// Zoom factor
	Long				flags;			// Viewport flags
} XCGENViewportDef, *XCGENViewportDefPtr;

typedef struct XCGENCamera
{
	Vec_3			pos;
	Mat_3			mat;
	XCGENTimestamp	tsCurrent;
} XCGENCamera, *XCGENCameraPtr;

XCGENDLL XCGENResult XCGENSetViewport( XCGENViewportDefPtr pViewport );
XCGENDLL XCGENResult XCGENGetCamera( XCGENCameraPtr *pCam );
XCGENDLL XCGENResult XCGENProjectPoint( Vec_3 pProj, Vec_3 pOrig );

//********************************************************************
// -- FRAME RENDER
//********************************************************************
XCGENDLL XCGENResult XCGENRenderFrame( XCGENTimestamp stamp );

//********************************************************************
// -- PHYSICS
//********************************************************************
XCGENDLL XCGENResult XCGENDoMotion( XCGENTimestamp stamp );

//********************************************************************
// -- STATISTICS
//********************************************************************
typedef struct
{
	// General stats
	Int				nObjects;	// Number of objects
	Int				nSprites;	// Number of sprites
	Int				nMeshes;	// Number of meshes
	Int				nTextures;	// Number of textures

	// Memory stats
	Int				memCode;	// Code size
	Int				memStatic;	// Static data size
	Int				memObjects;	// Object memory usage
	Int				memMeshes;	// Mesh memory usage
	Int				memSprites;	// Sprite memory usage
	Int				memTextures;// Texture memory usage
	Int				memTotal;	// Total memory consumption

	// Rendering stats (ms.)
	Int				nPolygons;	// Number of polygons in this frame
	XCGENTimestamp	tsFrame;	// Gross time to render frame
	XCGENTimestamp	tsEdgeSort;	// Time spent sorting edges
	XCGENTimestamp	tsSurfDraw;	// Time spent drawing spans
	XCGENTimestamp	tsParticles;// Time spent drawing particles
	XCGENTimestamp	tsLastFrame;// Time since last render (frame rate)

	// Physics stats 
	XCGENTimestamp	tsMotion;	// Gross time to perform motion
	XCGENTimestamp	tsRegions;	// Time to create collision regions

	// Direct 3D caps
	Bool			bViewport2;	// D3D has D3DVIEWPORT2 interface
	Bool			bSurface3;	// D3D has DIRECTDRAWSURFACE3 interface
	Bool			bDrawPrim;	// Card cupports the draw primitive call
	Bool			bVertSysMem;// Vertices can be read from system memory
	Bool			bTexSysMem;	// Textures can be read from system memory
	Bool			b16Bit;		// Card is a 16 bit color model
	Bool			bFilterTex;	// Textures are filtered
	Bool			bAlpha;		// 3D Card can alpha blend
	Bool			bColorKey;	// 3D Card can perform color keying
	Bool			bPaletteTex;// 3D Card can use 8-bit palettized textures
	Bool			bFogVert;	// 3D Card can do vertex fogging effects
	Bool			bFogTable;	// 3D Card can do table driven pixel fogging

	// Direct 3D stats
	Int				nFullBuffs;	// Number of full polybuffers
	Int				nTexLoads;	// Number of textures loads
	
} XCGENStats, *XCGENStatsPtr;

XCGENDLL XCGENResult XCGENGetStats( XCGENStatsPtr pStats );
#endif
