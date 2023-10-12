;/*******************************************************************
;**    File: XCSprite.asm
;**      By: Paul L. Rowan    
;** Created: 970414
;**
;** Description:
;**
;********************************************************************/

.586
.Model FLAT, C

;--------------------------------------------------------------------
; MACROS
;--------------------------------------------------------------------

;--------------------------------------------------------------------
; Handy pipe tracking do-nothing macros
;--------------------------------------------------------------------
u	TEXTEQU	<>
v	TEXTEQU <>
uu	TEXTEQU <>
vv	TEXTEQU <>

;--------------------------------------------------------------------
; Variable declaration macros
;--------------------------------------------------------------------
DD_PUBLIC	MACRO	name
	PUBLIC	name
	name	dd	?
ENDM

DD_PRIVATE	MACRO	name
	name	dd	?
ENDM

;--------------------------------------------------------------------
; Pentium cycle counts
;--------------------------------------------------------------------
TIMERSTART	MACRO	
			rdtsc
			mov		[gTime], eax
ENDM

TIMERSTOP	MACRO
			rdtsc
			sub		eax, [gTime]
			sub		eax, 13
			add		[gTimer], eax
ENDM


.DATA
ALIGN 16

DD_PUBLIC	pSprSrc
DD_PUBLIC	pSprDst

DD_PUBLIC	HStepUV
DD_PUBLIC	HStepU
HStepArray	equ	HStepU

DD_PUBLIC	VStepUV
DD_PUBLIC	VStepU
VStepArray	equ	VStepU

DD_PRIVATE	ebpsave
DD_PRIVATE	pSrcSave
DD_PUBLIC	Sprw
DD_PUBLIC	Sprh
DD_PUBLIC	Sprshw
DD_PUBLIC	Sprsvw
DD_PUBLIC	Sprcth
DD_PUBLIC	Sprctv
DD_PUBLIC	Sprshf
DD_PUBLIC	Sprsvf
DD_PUBLIC	Sprdstskip


.CODE


DrawSpriteAsm PROC C PUBLIC
		mov		esi, [pSprSrc]
		mov		edi, [pSprDst]

		mov		[ebpsave], ebp

VLoop:
		mov		[pSrcSave], esi		; save src ptr to beginning of scan

	; HORIZONTAL LOOP BEGIN
		xor		eax, eax			; pre-zero eax
		mov		edx, [Sprw]			; get width count
		mov		ecx, [Sprcth]			; get horizontal counter begin
		mov		ebx, [Sprshf]			; get fractional step

HLoop:
		mov		ax, [esi]
		cmp		eax, 0

		je		SeeThru
		mov		[edi], ax
SeeThru:

		add		ecx, ebx
		sbb		ebp, ebp			; store carry for roll-over

		; Kludgy, this will change when globals are added
		add		esi, [HStepArray + ebp*4]	; Debug -- Work
		
		add		edi, 2

		dec		edx
		jnz		HLoop
	; HORIZONTAL LOOP END

		mov		esi, [pSrcSave]		; restore src ptr to beginning of span

		mov		ebx, [Sprsvf]			; get vertical fractional step		
		add		[Sprctv], ebx
		sbb		ebp, ebp			; store carry for roll-over

		; Kludgy, this will change when globals are added
		add		esi, [VStepArray + ebp*4]	; Debug -- Work

		add		edi, [Sprdstskip]

		dec		[Sprh]
		jnz		VLoop

		mov		ebp, [ebpsave]

		ret

DrawSpriteAsm ENDP


END