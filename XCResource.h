/*******************************************************************
**    File: XCResource.h
**      By: Paul L. Rowan     
** Created: 970403
**
** Description: Main resource type definitions (shared with editor)     
**
********************************************************************/
#ifndef	_XCRESOURCE_H_
#define _XCRESOURCE_H_

#include "XCGEN.h"

#pragma pack( push, oldpack, 4 )	// Let's ensure it's only packed on dwords

//********************************************************************
//	-- MISCELLANEOUS DATA DEFINITIONS
//********************************************************************
XCGENResourcePtr	XCGENGetResource( XCGENResName name, XCGENResType type );
XCGENResult			XCGENReleaseResource( XCGENResourcePtr pResource );

//********************************************************************
//	-- MESH RESOURCE DATA DEFINITION
//********************************************************************
// General order in resource file:
//		( all randomly accessed elements should align on 16 byte boundaries )
//
//	Mesh-
//		MeshLOD - (xLODs)				16 byte aligned
//			MeshVertex - (xVerts)		16 byte aligned
//			MeshEdges - (xEdges)		16 byte aligned
//			MeshTexList - (xTextures)	16 byte aligned
//			MeshAttach - (xAttachments)	16 byte aligned
//			MeshNode - (xNodes)			16 byte aligned
//				MeshFace - (xFaces)				no alignment needed (linear traversal)
//					MeshEdgeIndex - (xEdges)		"
//				(padding)				n bytes to match alignment
//		MeshLOD2 - (xLODs)				16 byte aligned
//			MeshVertex	(xVerts)
//			etc.
//
//	During load process item indexes will be replaced
//		by the cooresponding pointer value.  This will speed traversal.
	

#define XCGEN_MAX_LODS		4

typedef struct MeshTexList	// ----- 32 bytes (MUST ALIGN)
{
	Char			name[16];	// Resource name of texture
	XCGENTexturePtr	pTexture;	// Hot loaded pointer to loaded texture
	Long			pad[3];
} MeshTexList, *MeshTexListPtr; 

typedef struct MeshVertex	// ----- 32 bytes (MUST ALIGN)
{
	Float		p[3];		// Vertex position
	Float		n[3];		// Vertex normal
	Float		t[2];		// Texture map coordinates
} MeshVertex, *MeshVertexPtr;

typedef struct MeshEdge		// ----- 16 bytes (MUST ALIGN)
{
	Long		v[2];		// Vertex Indexes
	Long		iCache;		// Cached edge index (hot loaded during clip process) -1 = new, -2 = fully clipped
	UChar		r, g, b, a;	// Special case for line-set
} MeshEdge, *MeshEdgePtr;

typedef struct MeshAttach	// ----- 64 bytes (MUST ALIGN)	
{
	Float			pos[3];		// Position relative to mesh origin
	Float			mat[3][3];		// Attachment orientation matrix
	Long			manipID;	// ID into manipulator list for indirection
	union
	{
		Long			iBNext;	// Strings attachments together
		MeshAttach		*pBNext;
	};
	Long			pad[2];
} MeshAttach, *MeshAttachPtr;

#define XCGEN_FACE_SELFILLUM	0x80000000	// Light sources do not affect
#define XCGEN_FACE_SOLIDCOLOR	0x40000000	// Use texture's base color and fill

#define XCGEN_FACE_MANIPULATOR	0x00100000  // Face refers to a manipulator for draw options
#define XCGEN_FACE_MANIPMASK	0x000FF000	// (Mask>>12) Low byte specifies manipulator id

#define XCGEN_FACE_TEXTURE		0x00000100	// Face refers to a manipulator for texture
#define XCGEN_FACE_TEXTUREMASK	0x000000FF	// specifies a manipulator id

typedef Long	MeshEdgeIndex; // Negative values denote edge in reverse direction
typedef Long	*MeshEdgeIndexPtr;

typedef struct MeshFace		// ----- ?? bytes
{
	Float		plane[4];	// Plane definition (x,y,z,d)
	Long		flags;		// Flag values 
	UChar		r, g, b, a;	// Polygon color and transparency (yeah, right)
	Float		emit;		// Emmisive value (self illumination) (0.0 - 1.0)
	Float		texelscale;	// Minimum (u,v) scale in world units of one texel
	union
	{
		Long				iTexture;	// Texture index
		XCGENTexturePtr		pTexture;	// Hot loaded pointer to texture resource
	};
	Long		nEdges;		// Number of edges for this face 
} MeshFace, *MeshFacePtr;

#define XCGEN_NODE_EMPTY		0x80000000	// Node contains nothing
#define XCGEN_NODE_SOLID		0x40000000	// Node is solid matter

#define XCGEN_NODE_MANIPULATOR	0x00000100	// Node refers to a manipulator for draw instructions
#define XCGEN_NODE_MANIPMASK	0x000000FF	// Low byte specifies manipulator id

#define XCGEN_NODE_CALLBACK		0x00100000	// Node refers to a manipulator for collision callback
#define XCGEN_NODE_CALLBACKMASK	0x000FF000	// (Mask>>12) specifies a manipulator id

typedef struct MeshNode		// ----- 64 bytes	(MUST ALIGN)
{
	Float		plane[4];	// Plane definition (x,y,z,d)
	Float		minmaxs[6];	// Bound box min and max
	union
	{
		Long			children[2];	// Child node front, back (MeshLOD relative)
		MeshNode		*pChildren[2];	// Hot loaded pointer to children
	};
	Long		flags;		// Flags about this node
	Long		nFaces;		// Number of faces at this node
	union
	{
		Long			iAttach;	// Index of sub-object attachment (only one per node)
		MeshAttachPtr	pAttach;	// Hot loaded pointer to attachment
	};
	XCGENObjectPtr		pObjects;	// Other objects contained in this object
} MeshNode, *MeshNodePtr;

#define XCGEN_MESH_LINESET		0x00000001	// This LOD is a set of lines (special case)

typedef struct MeshLOD		// ----- 48 bytes (MUST ALIGN)
{
	Float		maxdist;	// Distance at which LOD becomes invalid
	Long		flags;		// Mesh LOD Flags
	Long		nVerts;		// Number of vertices
	union
	{
		Long			offVerts;	// Offset to vertices (MeshLOD relative)
		MeshVertexPtr	pVerts;		// Hot loaded pointer to vertices
	};
	Long		nEdges;		// Number of edges
	union
	{
		Long			offEdges;	// Offset to edges (MeshLOD relative)
		MeshEdgePtr		pEdges;		// Hot loaded pointer to edges
	};
	Long		nTextures;	// Number of textures
	union
	{
		Long			offTexts;	// Offset to texture list
		MeshTexListPtr	pTexts;		// Hot loaded pointer to texture list
	};
	Long		nAttach;	// Number of attachments
	union
	{
		Long			offAttach;	// Offset to attachment list (MeshLOD relative)
		MeshAttachPtr	pAttach;	// Hot loaded pointer to attachment list
	};
	union
	{
		Long			offRoot;	// Offset to root node (MeshLOD relative)
		MeshNodePtr		pRoot;		// Hot loaded pointer to root node
	};
	Long		pad;
} MeshLOD, *MeshLODPtr;

#define XCGEN_HULL_LEAF			0x80000000	// Node is a leaf node
#define XCGEN_HULL_SOLID		0x00000001	// Leaf node is solid 

typedef struct MeshHullNode	// ---- 32 bytes (MUST ALIGN)
{
	Long		flags;		// HullNode flags
	Short		planetype;	// 0 = X-AXIS,  1 = Y-AXIS,  2 = Z-AXIS,  >3 = ARBITRARY
	Short		signbits;	// 0x1 = Negative x,   0x2 = Negative y,  0x04 = Negative z
	Float		plane[4];	// Plane definition (x,y,z,d)	
	union
	{
		Long			children[2];	// Child node front, back (MeshHull relative)
		MeshHullNode	*pChildren[2];	// Hot loaded pointer to children
	};
} MeshHullNode, *MeshHullNodePtr;

typedef struct MeshHull		// ----- 16 bytes (MUST ALIGN)
{
	union
	{
		Long			root;	// Offset to root hull node (MeshHull relative)
		MeshHullNodePtr	pRoot;	// Hot loaded pointer to root
	};
	Long		pad[3];	
} MeshHull, *MeshHullPtr;

typedef struct Mesh			// ----- 80 bytes (MUST ALIGN)
{
	XCGENResource	header;		// 32 bytes
	Float			radius;		// Radius
	Float			mass;		// Mass
	Float			minmaxs[6];	// Bounding box
	Long			nManips;	// Number of manipulators
	union
	{
		Long			offHull[4];	// Offset to Hulls if any (Mesh relative)	
		MeshHullPtr		pHull[4];	// Hot loaded pointers to Hulls
	};
	union
	{
		Long			offLOD[4];	// Offset to LOD's (Mesh relative)
		MeshLODPtr		pLOD[4];	// Hot loaded pointers to LOD's
	};
	Long			pad[3];
} Mesh, *MeshPtr;

//********************************************************************
//	-- SPRITE RESOURCE DATA DEFINITION
//********************************************************************
//
//	AnimSprite - 
//		SpriteFrame - (xFrames)
//			SpriteData - (xwidth*height)
//		SpriteFrame2
//			etc.

typedef struct SpriteFrame
{
	Long		offx, offy;		// Offset from total rect to this rect
	Long		width, height;	// Width and Height of this frame
	Long		rowBytes;		// Sprite Data pitch
	Short		sprData[1];		// Actual Sprite data
} SpriteFrame, *SpriteFramePtr;

typedef struct AnimSprite
{
	XCGENResource	header;
	Long			width, height;	// Width and Height
	Long			hotx, hoty;		// Hotspot
	Float			scale;			// Sprite pixel to world ratio
	Long			nFrames;		// Number of frames
	Long			speed;
	Long			offFrames[1];	// Offsets to frames (AnimSprite relative)
} AnimSprite, *AnimSpritePtr;

#pragma pack( pop, oldpack )

#endif

