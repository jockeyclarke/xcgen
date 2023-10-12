/*******************************************************************
**    File: XCRSpanit (Janet)
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
#define TIMERSTART	_asm	_emit 0x0f _asm _emit 0x31 \
					_asm	mov	cycles, eax

#define TIMERSTOP	_asm	_emit 0x0f _asm _emit 0x31 \
					_asm	sub	eax, 13			\
					_asm	sub	eax, cycles		\
					_asm	mov cycles, eax		

Long cycles;

/************************ MEMBER FUNCTIONS *************************/
Void ScanEdges( Void )
{
	ENTER;

    Int		 x, y;
    Float	 /*fx,*/ fy/*, zinv, zinv2*/;
    EdgePtr  pEdge, pEdge2, pTemp;
    SpanPtr  pSpan;
    SurfPtr  pSurf, pSurf2;

    pSpan = gBuff.spans;

	TIMERBEGIN( gStats.tsEdgeSort );

    // Set up the active edge list as initially empty, containing
    // only the sentinels (which are also the background fill). Most
    // of these fields could be set up just once at start-up
    gEdgeHead.pNext = &gEdgeTail;
    gEdgeHead.pPrev = NULL;
    gEdgeHead.x = 0-0xFFFFF;           // left edge of screen
    gEdgeHead.pSurf[0] = NULL;
	gEdgeHead.pSurf[1] = NULL;

    gEdgeTail.pNext = NULL;          // mark edge of list
    gEdgeTail.pPrev = &gEdgeHead;
    gEdgeTail.x = (gViewport.width << 20) + 0xFFFFF;    // right edge of screen
	gEdgeTail.pSurf[0] = NULL;
    gEdgeTail.pSurf[1] = NULL;

    // The background surface is the entire stack initially, and
    // is infinitely far away, so everything sorts in front of it.
    // This could be set just once at start-up
    gSurfs[0].pNext = gSurfs[0].pPrev = &gSurfs[0];
    gSurfs[0].zinv00 = -999999.0f;
    gSurfs[0].zinvstepx = gSurfs[0].zinvstepy = 0.0f;

    for (y=0 ; y<gViewport.height ; y++)
    {
        fy = (Float)y;

        // Sort in any edges that start on this scan
        pEdge = gNewEdges[y].pNext;
        pEdge2 = &gEdgeHead;
        while (pEdge != &gMaxEdge)
        {
            while (pEdge->x > pEdge2->pNext->x)
                pEdge2 = pEdge2->pNext;

            pTemp = pEdge->pNext;
            pEdge->pNext = pEdge2->pNext;
            pEdge->pPrev = pEdge2;
            pEdge2->pNext->pPrev = pEdge;
            pEdge2->pNext = pEdge;

            pEdge2 = pEdge;
            pEdge = pTemp;
        }

		// Update shade values for this scan
		for (pEdge = gEdgeHead.pNext; pEdge; pEdge=pEdge->pNext)
		{
			Int		d;

			if (pEdge->pSurf[0])
			{
				// Edge leads a surface, so set beginning shade value
				pEdge->pSurf[0]->shade = pEdge->s;
				pEdge->pSurf[0]->shadex = pEdge->x>>16;	// Temporarily store x
			}
			if (pEdge->pSurf[1])
			{
				// Edge trails a surface, so set the shade delta
				d = ((pEdge->x>>16) - pEdge->pSurf[1]->shadex);
				if (d>0)
					pEdge->pSurf[1]->dshade = (pEdge->s - pEdge->pSurf[1]->shade)/d;
				else
					pEdge->pSurf[1]->dshade = 0;
			}
		}

        // Scan out the active edges into spans

        // Start out with the left background edge already inserted,
        // and the surface stack containing only the background
        gSurfs[0].state = 1;
        gSurfs[0].visxstart = 0;

        for (pEdge=gEdgeHead.pNext ; pEdge ; pEdge=pEdge->pNext)
        {
            if (pEdge->pSurf[0]!=NULL)// && pEdge->pSurf[0]->state==0)
            {
				pSurf = pEdge->pSurf[0];

                // It's a leading edge. Figure out where it is
                // relative to the current surfaces and insert in
                // the surface stack; if it's on top, emit the span
                // for the current top.
                // First, make sure the edges don't cross
                if (++pSurf->state ==1 )
                {
                    // See if that makes it a new top surface
                    pSurf2 = gSurfs[0].pNext;

					if ( ULong(pSurf) >= ULong(pSurf2) )	//if (zinv >= zinv2)
                    {
                        // It's a new top surface
                        // emit the span for the current top
                        x = (pEdge->x) >> 16;

						if (pSurf2!=&gSurfs[0])	// Don't process the background
						{
							pSpan->count = x - pSurf2->visxstart;
							if (pSpan->count > 0)
							{
								pSpan->y = y;
								pSpan->x = pSurf2->visxstart;

								pSpan->s = (pSurf2->shade + (pSpan->x-pSurf2->shadex)*pSurf2->dshade);
								pSpan->ds = pSurf2->dshade;

								// Attach span to surface
								pSpan->pNext = pSurf2->pSpans;
								pSurf2->pSpans = pSpan;

								pSpan->zinv		= pSurf2->zinv00   + Float(pSpan->x)*pSurf2->zinvstepx   + Float(pSpan->y)*pSurf2->zinvstepy;
								pSpan->uoverz	= pSurf2->uoverz00 + Float(pSpan->x)*pSurf2->uoverzstepx + Float(pSpan->y)*pSurf2->uoverzstepy;
								pSpan->voverz	= pSurf2->voverz00 + Float(pSpan->x)*pSurf2->voverzstepx + Float(pSpan->y)*pSurf2->voverzstepy;

								// Make sure we don't overflow
								// the span array
								if (pSpan < &gBuff.spans[MAX_SPANS])
									pSpan++;

								//gStats.spans++;
							}
						}

                        pSurf->visxstart = x;

                        // Add the edge to the stack
                        pSurf->pNext = pSurf2;
                        pSurf2->pPrev = pSurf;
                        gSurfs[0].pNext = pSurf;
                        pSurf->pPrev = &gSurfs[0];
                    }
                    else
                    {
                        // Not a new top; sort into the surface stack.
                        // Guaranteed to terminate due to sentinel
                        // background surface
                        do
                        {
                            pSurf2 = pSurf2->pNext;
						} while( ULong(pSurf) < ULong(pSurf2) );	//} while (zinv < zinv2);

                        // Insert the surface into the stack
                        pSurf->pNext = pSurf2;
                        pSurf->pPrev = pSurf2->pPrev;
                        pSurf2->pPrev->pNext = pSurf;
                        pSurf2->pPrev = pSurf;
                    }
                }
            }

			if (pEdge->pSurf[1]!=NULL )//&& pEdge->pSurf[1]->state==1)
            {
				pSurf = pEdge->pSurf[1];

               // It's a trailing edge; if this was the top surface,
                // emit the span and remove it.
                // First, make sure the edges didn't cross
                if (--pSurf->state == 0)
                {
                    if (gSurfs[0].pNext == pSurf)
                    {
                        // It's on top, emit the span
                        x = (pEdge->x >> 16);
                        pSpan->count = x - pSurf->visxstart;
                        if (pSpan->count > 0)
                        {
                            pSpan->y = y;
                            pSpan->x = pSurf->visxstart;

							pSpan->s = (pSurf->shade + (pSpan->x-pSurf->shadex)*pSurf->dshade);
							pSpan->ds = pSurf->dshade;

							// Attach span to surface
							pSpan->pNext = pSurf->pSpans;
							pSurf->pSpans = pSpan;

 							pSpan->zinv		= pSurf->zinv00   + Float(pSpan->x)*pSurf->zinvstepx   + Float(pSpan->y)*pSurf->zinvstepy;
							pSpan->uoverz	= pSurf->uoverz00 + Float(pSpan->x)*pSurf->uoverzstepx + Float(pSpan->y)*pSurf->uoverzstepy;
							pSpan->voverz	= pSurf->voverz00 + Float(pSpan->x)*pSurf->voverzstepx + Float(pSpan->y)*pSurf->voverzstepy;

                           // Make sure we don't overflow
                            // the span array
                            if (pSpan < &gBuff.spans[MAX_SPANS])
                                pSpan++;

							//gStats.spans++;
                        }

                        pSurf->pNext->visxstart = x;
                    }

                    // Remove the surface from the stack
                    pSurf->pNext->pPrev = pSurf->pPrev;
                    pSurf->pPrev->pNext = pSurf->pNext;
                }
            }
        }

		// Remove rougue surfaces from the stack
		pSurf = &gSurfs[0];
		while( pSurf->pNext != &gSurfs[0] )
		{
			pSurf->state = 0;
			pSurf = pSurf->pNext;
		}
		
		gSurfs[0].pNext = gSurfs[0].pPrev = &gSurfs[0];

        // Remove edges that are done
        pEdge = gpRemoveEdges[y];
        while (pEdge)
        {
            pEdge->pPrev->pNext = pEdge->pNext;
            pEdge->pNext->pPrev = pEdge->pPrev;
            pEdge = pEdge->pNextRemove;
        }

        // Step the remaining edges one scan line, and re-sort
        for (pEdge=gEdgeHead.pNext ; pEdge != &gEdgeTail ; )
        {
            pTemp = pEdge->pNext;

            // Step the edge
            pEdge->x += pEdge->xstep;
			pEdge->s += pEdge->sstep;

            // Move the edge back to the proper sorted location,
            // if necessary
            while (pEdge->x < pEdge->pPrev->x)
            {
                pEdge2 = pEdge->pPrev;
                pEdge2->pNext = pEdge->pNext;
                pEdge->pNext->pPrev = pEdge2;
                pEdge2->pPrev->pNext = pEdge;
                pEdge->pPrev = pEdge2->pPrev;
                pEdge->pNext = pEdge2;
                pEdge2->pPrev = pEdge;
            }

            pEdge = pTemp;
        }

    }

    pSpan->x = -1;  // mark the end of the list

	TIMEREND( gStats.tsEdgeSort );
	//gStats.sortcycles = cycles;

	EXIT( "ScanEdges" );
}


