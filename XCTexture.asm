;/*******************************************************************
;**    File: XCTexture.asm
;**      By: Paul L. Rowan    
;** Created: 970414
;**
;** Description:
;**
;********************************************************************/

INCLUDE ApTexture.inc

.DATA
ALIGN 16

;--------Cache line-------
FixedScale		dd		65536.0
FixedScale2		dd		32768.0
FixedScale3		dd		21845.33333
FixedScale4		dd		16384.0
FixedScale5		dd		13107.2
FixedScale6		dd		10922.66667
FixedScale7		dd		9362.285714
FixedScale8		dd		8192.0
;--------Cache line-------
FixedScale9		dd		7281.777778
FixedScale10	dd		6553.6
FixedScale11	dd		5957.81812
FixedScale12	dd		5461.333333
FixedScale13	dd		5041.230769
FixedScale14	dd		4681.142857
FixedScale15	dd		4369.066667
FixedScale16	dd		4096.0

;--------Cache line-------
DD_PRIVATE	gTime			; Mostly hit by FPU section
DD_PUBLIC	gTimer		

DD_PRIVATE	saveEBP
DD_PRIVATE	saveESP

DD_PRIVATE	zinvsxrem
DD_PRIVATE	uoverzsxrem
DD_PRIVATE	voverzsxrem

AffineLength	dd		16.0

;--------Cache line-------
DD_PRIVATE	leftover		; Hit during setup

DD_PRIVATE	DeltaU			; 8.16 - Pre scaled to texture width and miplevel
DD_PRIVATE	DeltaV			; 8.16 - Pre scaled to texture width and miplevel

DD_PRIVATE	UFixed			; 8.16 - Pre scaled to texture width and miplevel
DD_PRIVATE	VFixed			; 8.16 - Pre scaled to texture width and miplevel

DD_PRIVATE	jumppt
DD_PUBLIC	gpSurf

FPUCW			dw		?
OldFPUCW		dw		?

;--------------------------------------------------------------------
; These must be set externally before a call to EmitTexturedSpan
;--------------------------------------------------------------------
DD_PUBLIC	gpScreen		;  Preset to beginning of span
DD_PUBLIC	gpSpan			;  Points to span to be rendered
DD_PUBLIC	gpTexture		;  Points to beginning of texture
DD_PUBLIC	gpCTable		;  Points to color table
				
DD_PRIVATE	spanrem
DD_PRIVATE	count
DD_PRIVATE	spancnt
DD_PRIVATE	floattemp

; ----- Cache Line -----
DD_PUBLIC	uvMask			; pre-rotate uv mask - Set once at beginning of span
DD_PUBLIC	uBits			; [ width bits(4-8) - miplevel(0-3) ] - Set once at beginning of span
DD_PUBLIC	uShift			; [ 16 - uBits ] - Set once at beginning of span
DD_PRIVATE	dPl
DD_PRIVATE	dPh
DD_PUBLIC	SFixed			; 4.8 Shade value			
DD_PUBLIC	DeltaS			; 4.8 Delta Shade
DD_PUBLIC	DeltaS16		; 4.8 Delta Shade value (complete span)

;----------------------------------------------------------------------------
;						esi							eax
;		+-------------------------------++----------------------------+
;	P	|   U   ||  Uf  |        |   V  ||   Vf  |      |  S  ||   Sf |                                                              |
;       +-------------------------------++----------------------------+
;	
;		+-------------------------------++----------------------------+
;	dP	|   dU  || dUf  |        |   dV ||  dVf  |      |  dS ||  dSf |
;		+-------------------------------++----------------------------+

SetupAffineSpan MACRO
	mov		ebp, [gpSurf]

 	mov		ecx, [uShift]	; Pack Parameter set
	mov		esi, [UFixed]
	add     esi,(Surf PTR [ebp]).dudxmod; fixup rounding rule
	add		esi,(Surf PTR [ebp]).uoff
	mov		ebx, [DeltaU]
	shl		esi, cl
	shl		ebx, cl

	mov		edx, [VFixed]
	add     edx,(Surf PTR [ebp]).dvdxmod; fixup rounding rule
	add		edx,(Surf PTR [ebp]).voff
	mov		eax, [DeltaV]
	shr		edx, 16
	shr		eax, 16
	or		esi, edx
	or		ebx, eax			

	mov		[dPh], ebx		; HIWORD COMPLETE  (freeze esi)

	mov		eax, [VFixed]
	add     eax,(Surf PTR [ebp]).dvdxmod; fixup rounding rule
	add		eax,(Surf PTR [ebp]).voff
	mov		ebx, [DeltaV]
	shl		eax, 16
	shl		ebx, 16
	mov		ecx, [SFixed]
	mov		edx, [DeltaS]
	shr		ecx, 8
	shr		edx, 8
	or		eax, ecx
	or		ebx, edx

	mov		[dPl], ebx		; LOWORD COMPLETE	(freeze eax)

	mov		ebx, [uBits]	; Calculate jump point
	dec		ebx				; Only seven jump tables
	shl		ebx, 6			; uShift jump pt

	mov		ecx, 16
	sub		ecx, [spanrem]
	and		ecx, 0Fh
	shl		ecx, 1			; Shortword scale	(screen)
	sub		esp, ecx		; Cue screen pointer back to match offsets

	shl		ecx, 1			; Longword scale	(jump table)			
	add		ebx, ecx

	add		ebx, APJumpTable
	mov		ecx, [ebx]
	mov		[jumppt], ecx

	; Pre span register setup

 	mov		ebp, [gpTexture]
	mov		ecx, [uBits]
	mov		edx, esi
	and		edx, [uvMask]
	rol		edx, cl						; 1 cy -- Must be U pipe
	mov		ebx, eax					; 1 cy  

	add		eax, [dPl]					; 2 cy 
	adc		esi, [dPh]					; 2 cy 

	and		ebx, 00000F00h				; 1 cy 

	add		eax, [dPl]					; 2 cy 
	mov		bl, BYTE PTR [ebp+edx]		; 1 cy -- 1 cycle PRS -  6 cycle CACHE (RA)

	mov		edx, esi					; 1 cy 
	adc		esi, [dPh]					; 2 cy 

	and		edx, [uvMask]				; 2 cy -- UVMask
	mov		cx, WORD PTR [edi+ebx*2]	; 1 cy -- 1 cycle PRS -  6 cycle CACHE (RA)

	jmp		[jumppt]		; Jump to correct spot for this span

ENDM

; eax	Param set (LO DWORD)
; ebx	bh = shade, bl = texel color index
; ecx	cx = color value (16-bit)
; edx	texel offset
; esi	Param set (HI DWORD)
; esp	pScreen
; edi	Color Table ptr
; ebp	pTexture

AffinePixel MACRO offset:REQ, shift:REQ

	rol		edx, shift					; 1 cy -- Must be U pipe
	mov		ebx, eax					; 1 cy  

	mov		[esp+offset*2], cx			; 1 cy -- 1 cycle PRS -  1 cycle CACHE (1/32th)
	and		ebx, 00000F00h				; 1 cy 

	add		eax, [dPl]					; 2 cy 
	mov		bl, BYTE PTR [ebp+edx]		; 1 cy -- 1 cycle PRS -  6 cycle CACHE (RA)

	mov		edx, esi					; 1 cy 
	adc		esi, [dPh]					; 2 cy 

	and		edx, [uvMask]				; 2 cy -- UVMask
	mov		cx, WORD PTR [edi+ebx*2]	; 1 cy -- 1 cycle PRS -  6 cycle CACHE (RA)

ENDM
; 11 cycles + 12 cycles (Random memory accesses) = 23 cycles per pixel

CleanupAffineSpan MACRO
	add		esp, 32			; Advance screen ptr

	mov		eax, [SFixed]
	add		eax, [DeltaS16]
	and		eax, 000FFFFFh
	mov		[SFixed], eax
ENDM


.CODE

;--------------------------------------------------------------------
; Build jump table to use for partial spans.
;--------------------------------------------------------------------

APJumpTableBegin:
FOR ubits, <1,2,3,4,5,6,7,8>
FOR offset, <0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15>
	AP&ubits&JumpT&offset	dd	AP&ubits&Jump&offset		
ENDM
ENDM
	APJumpTable	equ	APJumpTableBegin

;--------------------------------------------------------------------
; Declare the texture proc
;--------------------------------------------------------------------
EmitAffineSpan MACRO

	SetupAffineSpan

	FOR ubits, <1,2,3,4,5,6,7,8>
	FOR offset, <0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15>
		AP&ubits&J&offset:		AffinePixel	offset,ubits
		AP&ubits&Jump&offset	equ	AP&ubits&J&offset
	ENDM
		jmp	AffineSpanDone
	ENDM

AffineSpanDone:		
	CleanupAffineSpan

ENDM

;--------------------------------------------------------------------
; Emit a textured span 
;--------------------------------------------------------------------
EmitTexturedSpan PROC C PUBLIC

	PerspectiveTexture1

	EmitAffineSpan

	PerspectiveTexture2

EmitTexturedSpan ENDP


END