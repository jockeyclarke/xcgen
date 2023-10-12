;*******************************************************************
;*    File: ApTexture.inc
;*      By: Paul L. Rowan    
;* Created: 961211
;*
;* Description:  Textured span routines
;*
;********************************************************************

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

;--------------------------------------------------------------------
; C interface structures
;--------------------------------------------------------------------
FLOAT	TEXTEQU	<DWORD>
FX1616	TEXTEQU <DWORD>

Surf STRUCT	4
	pNext			DWORD	?			
	pPrev			DWORD	?
	visxstart		DWORD	?
	shade			DWORD	?
	shadex			DWORD	?
	dshade			DWORD	?
	state			DWORD	?
	miplevel		DWORD	?
	pFace			DWORD	?
	pObj			DWORD	?
	zinv00			FLOAT	?	
	zinvstepx		FLOAT	?	
	zinvstepy		FLOAT	?	
	uoverz00		FLOAT	?	
	uoverzstepx		FLOAT	?	
	uoverzstepy		FLOAT	?	
	voverz00		FLOAT	?	
	voverzstepx		FLOAT	?	
	voverzstepy		FLOAT	?	
	zinvstepx8		FLOAT	?	
	uoverzstepx8	FLOAT	?	
	voverzstepx8	FLOAT	?	
	dudxmod			FX1616	?	
	dvdxmod			FX1616	?
	uoff			FX1616	?	
	voff			FX1616	?
	pTex			DWORD	?
	pSpans			DWORD	?	
Surf ENDS

Span STRUCT	4
	x				DWORD	?
	y				DWORD	?
	count			DWORD	?
	s				DWORD	?
	dels			DWORD	?
	pNext			DWORD	?
	zinv			FLOAT	?
	uoverz			FLOAT	?
	voverz			FLOAT	?
Span ENDS	

;--------------------------------------------------------------------
; Miscellaneous EQU's
;--------------------------------------------------------------------
ScaleTable		equ		FixedScale
SPANLEN			equ		16
CTABLESIZE		equ		8192

;--------------------------------------------------------------------
; Setup and first half of texturing loop (FPU only)
;--------------------------------------------------------------------
PerspectiveTexture1	MACRO
	push	esi
	push	edi
	push	ebx

	mov		[saveEBP], ebp
	mov		[saveESP], esp

	;TIMERSTART

	;fstcw	[OldFPUCW]						; Store copy of CW
	;mov		ax, [OldFPUCW]					
	;and		ax, NOT 1100000000y				; 24 bit precis.
	;mov		[FPUCW], ax
	;fldcw	[FPUCW]

	mov		esi, [gpSpan]					; Get Span ptr
	mov		edx, [gpSurf]					; Get Surf ptr
	mov		esp, [gpScreen]
	mov		edi, [gpCTable]

	mov		ecx, (Span PTR [esi]).count		; get number of pixels
	mov		eax, ecx
	shr		ecx, 4							; get number of spans
	and		eax, 15							; get remaining pixels

	mov		[leftover], eax

@@:

	mov		[spancnt], ecx
	mov		eax, 0
	mov		[spanrem], 0

	;
	; Get left side values
	;
											;	st(0)	st(1)	st(2)	st(3)	st(4)	st(5)	st(6)
	fld		(Span PTR [esi]).voverz			;	v/zl
	fld		(Span PTR [esi]).uoverz			;	u/zl	v/zl
	fld		(Span PTR [esi]).zinv			;	1/zl	u/zl	v/zl
	fld1									;	1.0		1/zl	u/zl	v/zl
	fdiv	st, st(1)						;	zl		1/zl	u/zl	v/zl
	fld		st								;	zl		zl		1/zl	u/zl	v/zl	
	fmul	st, st(4)						;	vl		zl		1/zl	u/zl	v/zl	
	fxch	st(1)							;	zl		vl		1/zl	u/zl	v/zl	
	fmul	st, st(3)						;	ul		vl		1/zl	u/zl	v/zl

	fstp	st(5)							;	vl		1/zl	u/zl	v/zl	ul
	fstp	st(5)							;	1/zl	u/zl	v/zl	ul		vl		

	;
	;	Calculate right side 1/z values
	;
											;	st(0)	st(1)	st(2)	st(3)	st(4)	st(5)	st(6)

	fadd	(Surf PTR [edx]).zinvstepx8		;	1/zr	u/zl	v/zl	ul		vl
	fxch	st(1)							;	u/zl	1/zr	v/zl	ul		vl
	fadd	(Surf PTR [edx]).uoverzstepx8	;	u/zr	1/zr	v/zl	ul		vl
	fxch	st(2)							;	v/zl	1/zr	u/zr	ul		vl
	fadd	(Surf PTR [edx]).voverzstepx8	;	v/zr	1/zr	u/zr	ul		vl

	;
	;	Calculate right side coords
	;
											;	st(0)	st(1)	st(2)	st(3)	st(4)	st(5)	st(6)	st(7)
	fld1									;	1.0		v/zr	1/zr	u/zr	ul		vl
	fdiv	st, st(2)						;	zr		v/zr	1/zr	u/zr	ul		vl
	fld		st								;	zr		zr		v/zr	1/zr	u/zr	ul		vl
	fmul	st, st(2)						;	vr		zr		v/zr	1/zr	u/zr	ul		vl
	fxch	st(1)							;	zr		vr		v/zr	1/zr	u/zr	ul		vl		

	;fist	[lastz]

	fmul	st, st(4)						;	ur		vr		v/zr	1/zr	u/zr	ul		vl	

	test	ecx, ecx						; check for any full spans
	jz		HandleLeftoverPixels

SpanLoop:
	;	FPU		contains					;	st(0)	st(1)	st(2)	st(3)	st(4)	st(5)	st(6)
	;										;	ur		vr		v/zr	1/zr	u/zr	ul		vl	
	
	;
	;	Convert left side coords
	;
	fld		st(5)							;	ul		ur		vr		v/zr	1/zr	u/zr	ul		vl	
	fmul	[FixedScale]					;	ul16	ur		vr		v/zr	1/zr	u/zr	ul		vl	
	fistp	[UFixed]						;	ur		vr		v/zr	1/zr	u/zr	ul		vl	
	
	fld		st(6)							;	vl		ur		vr		v/zr	1/zr	u/zr	ul		vl	
	fmul	[FixedScale]					;	vl16	ur		vr		v/zr	1/zr	u/zr	ul		vl	
	fistp	[VFixed]						;	ur		vr		v/zr	1/zr	u/zr	ul		vl	

	;
	;	Calculate deltas
	;
	fsubr	st(5), st						;	ur		vr		v/zr	1/zr	u/zr	du		vl	
	fxch	st(1)							;	vr		ur		v/zr	1/zr	u/zr	du		vl	
	fsubr	st(6), st						;	vr		ur		v/zr	1/zr	u/zr	du		dv
	fxch	st(6)							;	dv		ur		v/zr	1/zr	u/zr	du		vr
	
	fmul	[FixedScale16]					;	dv8		ur		v/zr	1/zr	u/zr	du		vr	
	fistp	[DeltaV]						;	ur		v/zr	1/zr	u/zr	du		vr	

	fxch	st(4)							;	du		v/zr	1/zr	u/zr	ur		vr	
	fmul	[FixedScale16]					;	du8		v/zr	1/zr	u/zr	ur		vr	
	fistp	[DeltaU]						;	v/zr	1/zr	u/zr	ur		vr	

	;
	;	Increment gradients (now left side)	;	v/zl	1/zl	u/zl	ul		vl
	;
	mov		edx, [gpSurf]
	fadd	(Surf PTR [edx]).voverzstepx8	;	v/zr	1/zl	u/zl	ul		vl
	fxch	st(1)							;	1/zl	v/zr	u/zl	ul		vl
	fadd	(Surf PTR [edx]).zinvstepx8		;	1/zr	v/zr	u/zl	ul		vl
	fxch	st(2)							;	u/zl	v/zr	1/zr	ul		vl
	fadd	(Surf PTR [edx]).uoverzstepx8	;	u/zr	v/zr	1/zr	ul		vl
	fxch	st(2)							;	1/zr	v/zr	u/zr	ul		vl
	fxch	st(1)							;	v/zr	1/zr	u/zr	ul		vl

	;
	;	Calculate right side coords			
	;										;	st(0)	st(1)	st(2)	st(3)	st(4)	st(5)	st(6)	st(7)
	fld1									;	1.0		v/zr	1/zr	u/zr	ul		vl
	fdiv	st, st(2)						;	zr		v/zr	1/zr	u/zr	ul		vl

BeginAffine:

ENDM


;--------------------------------------------------------------------
; Cleanup, return and second half of texturing loop (FPU only)
;--------------------------------------------------------------------
PerspectiveTexture2 MACRO
	;
	;	fdiv should be done, finish right
	;										;	st(0)	st(1)	st(2)	st(3)	st(4)	st(5)	st(6)	st(7)
	fld		st								;	zr		zr		v/zr	1/zr	u/zr	ul		vl
	fmul	st, st(2)						;	vr		zr		v/zr	1/zr	u/zr	ul		vl
	fxch	st(1)							;	zr		vr		v/zr	1/zr	u/zr	ul		vl

	;fist	[lastZ]

	fmul	st, st(4)						;	ur		vr		v/zr	1/zr	u/zr	ul		vl

	dec		[spancnt]
	jnz		SpanLoop

HandleLeftoverPixels:
	;mov		esi, [gpScaledTex]

	cmp		[leftover], 0
	jz		FPUReturn

	;
	;	Decrement gradients (to true right side)	
	;										;	st(0)	st(1)	st(2)	st(3)	st(4)	st(5)	st(6)	st(7)
	fstp	[floattemp]						;	vr		v/zr	1/zr	u/zr	ul		vl
	fstp	[floattemp]						;	v/zr	1/zr	u/zr	ul		vl

	mov		edx, [gpSurf]

	fild	[leftover]						;	lfo		v/zr	1/zr	u/zr	ul		vl
	fld		[AffineLength]					;	16		lfo		v/zr	1/zr	u/zr	ul		vl
	fsub	st, st(1)						;	left	lfo		v/zr	1/zr	u/zr	ul		vl
	fxch	st(1)							;	lfo		left	v/zr	1/zr	u/zr	ul		vl
	fstp	[floattemp]						;	left	v/zr	1/zr	u/zr	ul		vl

	fld		(surf PTR [edx]).voverzstepx	;	v/zsx	left	v/zr	1/zr	u/zr	ul		vl
	fmul	st, st(1)						;	lv/zsx	left	v/zr	1/zr	u/zr	ul		vl
	fxch	st(1)							;	left	lv/zsx	v/zr	1/zr	u/zr	ul		vl
	fld		(surf PTR [edx]).uoverzstepx	;	u/zsx	left	lv/zsx	v/zr	1/zr	u/zr	ul		vl
	fmul	st, st(1)						;	lu/zsx	left	lv/zsx	v/zr	1/zr	u/zr	ul		vl
	fxch	st(2)							;	lv/zsx	left	lu/zsx	v/zr	1/zr	u/zr	ul		vl
	fstp	[voverzsxrem]					;	left	lu/zsx	v/zr	1/zr	u/zr	ul		vl
	fld		(Surf PTR [edx]).zinvstepx		;	1/zsx	left	lu/zsx	v/zr	1/zr	u/zr	ul		vl
	fmul	st, st(1)						;	l1/zsx	left	lu/zsx	v/zr	1/zr	u/zr	ul		vl
	fxch	st(2)							;	lu/zsx	left	l1/zsx	v/zr	1/zr	u/zr	ul		vl
	fstp	[uoverzsxrem]					;	left	l1/zsx	v/zr	1/zr	u/zr	ul		vl
	fxch	st(1)							;	l1/zsx	left	v/zr	1/zr	u/zr	ul		vl
	fstp	[zinvsxrem]						;	left	v/zr	1/zr	u/zr	ul		vl

	fstp	[floattemp]						;	v/zr	1/zr	u/zr	ul		vl
			
	fsubr	[voverzsxrem]					;	v/zrn	1/zr	u/zr	ul		vl
	fxch	st(1)							;	1/zr	v/zrn	u/zr	ul		vl
	fsubr	[zinvsxrem]						;	1/zrn	v/zrn	u/zr	ul		vl
	fxch	st(2)							;	u/zr	v/zrn	1/zrn	ul		vl
	fsubr	[uoverzsxrem]					;	u/zrn	v/zrn	1/zrn	ul		vl
	fxch	st(2)							;	1/zrn	v/zrn	u/zrn	ul		vl
	fxch	st(1)							;	v/zrn	1/zrn	u/zrn	ul		vl

	;
	;	Calculate right side coords
	;
											;	st(0)	st(1)	st(2)	st(3)	st(4)	st(5)	st(6)	st(7)
	fld1									;	1.0		v/zr	1/zr	u/zr	ul		vl
	fdiv	st, st(2)						;	zr		v/zr	1/zr	u/zr	ul		vl
	fld		st								;	zr		zr		v/zr	1/zr	u/zr	ul		vl
	fmul	st, st(2)						;	vr		zr		v/zr	1/zr	u/zr	ul		vl
	fxch	st(1)							;	zr		vr		v/zr	1/zr	u/zr	ul		vl		
	fmul	st, st(4)						;	ur		vr		v/zr	1/zr	u/zr	ul		vl	

	;
	;	Convert left side coords
	;
	fld		st(5)							;	ul		ur		vr		v/zr	1/zr	u/zr	ul		vl	
	fmul	[FixedScale]					;	ul16	ur		vr		v/zr	1/zr	u/zr	ul		vl	
	fistp	[UFixed]						;	ur		vr		v/zr	1/zr	u/zr	ul		vl	
	
	fld		st(6)							;	vl		ur		vr		v/zr	1/zr	u/zr	ul		vl	
	fmul	[FixedScale]					;	vl16	ur		vr		v/zr	1/zr	u/zr	ul		vl	
	fistp	[VFixed]						;	ur		vr		v/zr	1/zr	u/zr	ul		vl	

	;
	;	Calculate deltas
	;
	fsubr	st(5), st						;	ur		vr		v/zr	1/zr	u/zr	du		vl	
	fxch	st(1)							;	vr		ur		v/zr	1/zr	u/zr	du		vl	
	fsubr	st(6), st						;	vr		ur		v/zr	1/zr	u/zr	du		dv
	fxch	st(6)							;	dv		ur		v/zr	1/zr	u/zr	du		vr

	mov		eax, [leftover]
	dec		eax

	fmul	[FixedScale+eax*4]				;	dv8		ur		v/zr	1/zr	u/zr	du		vr	
	fistp	[DeltaV]						;	ur		v/zr	1/zr	u/zr	du		vr	

	fxch	st(4)							;	du		v/zr	1/zr	u/zr	ur		vr	
	fmul	[FixedScale+eax*4]				;	du8		v/zr	1/zr	u/zr	ur		vr	
	fistp	[DeltaU]						;	v/zr	1/zr	u/zr	ur		vr	

	mov		eax, [leftover]
	mov		[spanrem], eax
	mov		eax, 0
	inc		[spancnt]
	mov		[leftover], eax

	jmp		BeginAffine


FPUReturn:
	ffree	st(0)
	ffree	st(1)
	ffree	st(2)
	ffree	st(3)
	ffree	st(4)
	ffree	st(5)
	ffree	st(6)

Return:
	;fldcw	[OldFPUCW]

	;TIMERSTOP

	mov		esp, [saveESP]
	mov		ebp, [saveEBP]

	pop		ebx
	pop		edi
	pop		esi	

	ret
ENDM