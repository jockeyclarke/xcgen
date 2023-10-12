/*******************************************************************
**    File: XCVars.cpp
**      By: Paul L. Rowan     
** Created: 970404
**
** Description: XCGEN Global variables
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
Surf				gSurfs[MAX_SURFS];
Int					gnSurfaces;

XFormedVertex		gXFormVert[MAX_TOTAL_VERTICES];
Short				gnVertices;

Edge				gEdges[MAX_EDGES];
Int					gnEdges;
	
SharedBuffer		gBuff;		
Int					gnBuffs;

Edge				gNewEdges[MAX_HEIGHT];
EdgePtr				gpRemoveEdges[MAX_HEIGHT];
		
Edge				gEdgeHead;
Edge				gEdgeTail;
		
Edge				gMaxEdge = {0x7fffffff,0,0,0,{0,0},{0,0},NULL,NULL,NULL,NULL};

Float				gTrigTable[QUADRANT];

Char				szDatabasePath[128];

Int					gnTotalObjects;
ObjectInstance		goHeader;
ObjectInstancePtr	gpObjects = &goHeader;

ObjectInstancePtr	gObjZOrder[MAX_OBJECTS];
Double				gObjDist[MAX_OBJECTS];
Long				gnObjects;

Int					gnRegions[3];
OccupiedRegion		gRegions[3][MAX_OBJECTS];
OccupiedRegion		gRegionHead[3];

RenderStack			gpObjStack[MAX_ATTACHMENT_LEVELS];
Long				gnCurObject;
RenderStackPtr		gpStack;

Vec_3				gDust[MAX_DUST];
Particle			gParticles[MAX_PARTICLES];
ParticlePtr			gpActiveParts, gpFreeParts;

Vec_3				gLight = {0.0f,0.707f,0.707f};
UChar				gLR = 64, gLG = 16, gLB = 16;

Float				gFrustum[4][4];		
RenderSurface		gRenderSurf;
Viewport			gViewport;
XCGENCamera			gCamera;
ZBufferMode			gZBuffMode;
Long				gSurfKey;

ResArray			gResMap[MAX_RESTYPES];

Char				gErrorTxt[1024];

Long				gCycleCal;
XCGENStats			gStats;
Short				gIcons[1][32][32];

ObjectInstancePtr	gpCurObj;
