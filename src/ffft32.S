;-----------------------------------------------------------------------------;
; Fixed-point FFT routines for megaAVRs                        (C)ChaN, 2005
;-----------------------------------------------------------------------------;
;
; void fft32_input (const int16_t *array_src, complex_t *array_bfly);
; void fft32_execute (complex_t *array_bfly);
; void fft32_output (complex_t *array_bfly, uint16_t *array_dst);
;
;  <array_src>: Wave form to be processed.
;  <array_bfly>: Complex array for butterfly operations.
;  <array_dst>: Spectrum output buffer.
;
; These functions must be called in sequence to do a DFT in FFT algorithm.
; fft32_input() fills the complex array with a wave form to prepare butterfly
; operations. A hamming window is applied at the same time.
; fft32_execute() executes the butterfly operations.
; fft32_output() re-orders the results, converts the complex spectrum into
; scalar spectrum and output it in linear scale.
;
; The number of points FFT_N is 32
;----------------------------------------------------------------------------;


.nolist
#include "asm_macros.h"
.list

#define FFT_N32 32
#define FFT_B32 5

;----------------------------------------------------------------------------;
; Constant Tables
.global tbl32_window
tbl32_window:
	.dc.w	2621, 2910, 3768, 5161, 7036, 9320, 11926, 14753, 17694, 20634, 23462, 26068, 28352, 30226, 31619, 32477
	.dc.w	32766, 32477, 31619, 30226, 28352, 26068, 23462, 20634, 17694, 14753, 11926, 9320, 7036, 5161, 3768, 2910

tbl_cos_sin:
	.dc.w	32767, 0, 32137, 6392, 30272, 12539, 27244, 18204, 23169, 23169, 18204, 27244, 12539, 30272, 6392, 32137
	.dc.w	0, 32766, -6392, 32137, -12539, 30272, -18204, 27244, -23169, 23169, -27244, 18204, -30272, 12539, -32137, 6392

.global tbl32_bitrev
tbl32_bitrev:
	.dc.w	0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30
//	.dc.w	1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31

;----------------------------------------------------------------------------;
.global fft32_input
.func fft32_input
//.section "fft32_input_sec"
fft32_input:
	pushw	T2H,T2L
	pushw	AH,AL
	pushw	YH,YL

	movw	XL, EL				;X = array_src;
	movw	YL, DL				;Y = array_bfly;
	clr	EH				;Zero
	ldiw	ZH,ZL, tbl32_window		;Z = &tbl_window[0];
	ldiw	AH,AL, FFT_N32			;A = FFT_N32;
1:	lpmw	BH,BL, Z+			;B = *Z++; (window)
	ldw	CH,CL, X+			;C = *X++; (I-axis)
	FMULS16	DH,DL,T2H,T2L, BH,BL, CH,CL	;D = B * C;
	stw	Y+, DH,DL			;*Y++ = D;
#ifdef INPUT_IQ
	ldw	CH,CL, X+			;C = *X++; (Q-axis)
	FMULS16	DH,DL,T2H,T2L, BH,BL, CH,CL	;D = B * C;
#endif
	stw	Y+, DH,DL			;*Y++ = D;
	subiw	AH,AL, 1			;while(--A)
	brne	1b				;/

	popw	YH,YL
	popw	AH,AL
	popw	T2H,T2L
	clr	r1
	ret
.endfunc

;----------------------------------------------------------------------------;
.global fft32_execute
.func fft32_execute
//.section "fft32_execute_sec"
fft32_execute:
	pushw	T2H,T2L
	pushw	T4H,T4L
	pushw	T6H,T6L
	pushw	T8H,T8L
	pushw	T10H,T10L
	pushw	T12H,T12L
	pushw	T14H,T14L
	pushw	AH,AL
	pushw	YH,YL

	movw	ZL, EL				;Z = array_bfly;
	ldiw	EH,EL, 1			;E = 1;
	ldiw	XH,XL, FFT_N32/2			;X = FFT_N32/2;
1:	ldi	AL, 4				;T12 = E; (angular speed)
	mul	EL, AL				;
	movw	T12L, T0L			;
	mul	EH, AL				;
	add	T12H, T0L			;/
	movw	T14L, EL			;T14 = E;
	pushw	EH,EL
	movw	YL, ZL				;Z = &array_bfly[0];
	mul	XL, AL				;Y = &array_bfly[X];
	addw	YH,YL, T0H,T0L			;
	mul	XH, AL				;
	add	YH, T0L				;/
	pushw	ZH,ZL
2:	clrw	T10H,T10L			;T10 = 0 (angle)
	clr	EH				;Zero reg.
3:	lddw	AH,AL, Z+0			;A = *Z - *Y; *Z++ += *Y;
	asrw	AH,AL				;
	lddw	DH,DL, Y+0			;
	asrw	DH,DL				;
	movw	CL, AL				;
	subw	AH,AL, DH,DL			;
	addw	CH,CL, DH,DL			;
	stw	Z+, CH,CL			;/
	lddw	BH,BL, Z+0			;B = *Z - *Y; *Z++ += *Y;
	asrw	BH,BL				;
	lddw	DH,DL, Y+2			;
	asrw	DH,DL				;
	movw	CL, BL				;
	subw	BH,BL, DH,DL			;
	addw	CH,CL, DH,DL			;
	stw	Z+, CH,CL			;/
	movw	T0L, ZL
	ldiw	ZH,ZL, tbl_cos_sin		;C = cos(T10); D = sin(T10);
	addw	ZH,ZL, T10H,T10L		;
	lpmw	CH,CL, Z+			;
	lpmw	DH,DL, Z+			;/
	movw	ZL, T0L
	FMULS16	T4H,T4L,T2H,T2L, AH,AL, CH,CL	;*Y++ = A * C + B * D;
	FMULS16	T8H,T8L,T6H,T6L, BH,BL, DH,DL	;
	addd	T4H,T4L,T2H,T2L, T8H,T8L,T6H,T6L;
	stw	Y+, T4H,T4L			;/
	FMULS16	T4H,T4L,T2H,T2L, BH,BL, CH,CL 	;*Y++ = B * C - A * D;
	FMULS16	T8H,T8L,T6H,T6L, AH,AL, DH,DL 	;
	subd	T4H,T4L,T2H,T2L, T8H,T8L,T6H,T6L;
	stw	Y+, T4H,T4L			;/
	addw	T10H,T10L, T12H,T12L		;T10 += T12; (next angle)
#if FFT_N32 >= 128
	sbrs	T10H, FFT_B32 - 7			;while(T10 < pi)
#else
	sbrs	T10L, FFT_B32 + 1
#endif
	rjmp	3b				;/
	ldi	AL, 4				;Y += X; Z += X; (skip split segment)
	mul	XL, AL
	addw	YH,YL, T0H,T0L			;
	addw	ZH,ZL, T0H,T0L			;
	mul	XH, AL				;
	add	YH, T0L				;
	add	ZH, T0L				;/
	ldi	EL, 1				;while(--T14)
	subw	T14H,T14L, EH,EL		;
	rjne	2b				;/
	popw	ZH,ZL
	popw	EH,EL
	lslw	EH,EL				;E *= 2;
	lsrw	XH,XL				;while(X /= 2)
	adiw	XL, 0				;
	rjne	1b				;/

	popw	YH,YL
	popw	AH,AL
	popw	T14H,T14L
	popw	T12H,T12L
	popw	T10H,T10L
	popw	T8H,T8L
	popw	T6H,T6L
	popw	T4H,T4L
	popw	T2H,T2L
;	clr	r1
	ret
.endfunc

;----------------------------------------------------------------------------;
.global fft32_output
.func fft32_output
//.section "fft32_output_sec"
fft32_output:
	pushw	T2H,T2L
	pushw	T4H,T4L
	pushw	T6H,T6L
	pushw	T8H,T8L
	pushw	T10H,T10L
	pushw	AH,AL
	pushw	YH,YL

	movw	T10L, EL			;T10 = array_bfly;
	movw	YL, DL				;Y = array_output;
	ldiw	ZH,ZL, tbl32_bitrev		;Z = tbl_bitrev;
	clr	EH				;Zero
#ifdef INPUT_IQ
	ldiw	AH,AL, FFT_N32			;A = FFT_N32; (pluse/minus)
#else
	ldiw	AH,AL, FFT_N32 / 2		;A = FFT_N32 / 2; (pluse only)
#endif
1:	lpmw	XH,XL, Z+			;X = *Z++;
	addw	XH,XL, T10H,T10L		;X += array_bfly;
	ldw	BH,BL, X+			;B = *X++;
	ldw	CH,CL, X+			;C = *X++;
	FMULS16	T4H,T4L,T2H,T2L, BH,BL, BH,BL	;T4:T2 = B * B;
	FMULS16	T8H,T8L,T6H,T6L, CH,CL, CH,CL	;T8:T6 = C * C;
	addd	T4H,T4L,T2H,T2L, T8H,T8L,T6H,T6L;T4:T2 += T8:T6;
	SQRT32					;B = sqrt(T4:T2);
	stw	Y+, BH,BL			;*Y++ = B;
	subiw	AH,AL, 1			;while(--A)
	rjne	1b				;/

	popw	YH,YL
	popw	AH,AL
	popw	T10H,T10L
	popw	T8H,T8L
	popw	T6H,T6L
	popw	T4H,T4L
	popw	T2H,T2L
	clr	r1
	ret
.endfunc

;----------------------------------------------------------------------------;
//.section "fmuls_f_sec"
//int16_t fmuls_f (int16_t, int16_t);
.global fmuls_f
.func fmuls_f
fmuls_f:
	movw	CL, EL				;C = E;
	clr	EH	;Zero
	FMULS16	ZH,ZL,XH,XL, CH,CL, DH,DL	;Z:X = C * D;
	movw	EL, ZL
	clr	r1
	ret
.endfunc

;----------------------------------------------------------------------------;
//int16_t fsqrt(int16_t, int16_t);
.global fsqrt
.func fsqrt
fsqrt:
	pushw	T2H,T2L
	pushw	T4H,T4L
	pushw	T6H,T6L
	pushw	T8H,T8L
	pushw	T10H,T10L
	movw	CL, EL
	clr		EH
	FMULS16	T4H,T4L,T2H,T2L, CH,CL, CH,CL	;T4:T2 = E * E;
	FMULS16	T8H,T8L,T6H,T6L, DH,DL, DH,DL	;T8:T6 = D * D;
	addd	T4H,T4L,T2H,T2L, T8H,T8L,T6H,T6L;T4:T2 += T8:T6;
	SQRT32					;B = sqrt(T4:T2);
	movw	EL, BL
	popw	T10H,T10L
	popw	T8H,T8L
	popw	T6H,T6L
	popw	T4H,T4L
	popw	T2H,T2L
	clr	r1
	ret
.endfunc








