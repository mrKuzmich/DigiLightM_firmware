/// by Chan (c)
#define FFT_N	32		/* Number of samples (64,128,256,512). Don't forget to clean! */

typedef struct _tag_complex_t {
	int16_t	r;
	int16_t i;
} complex_t;

//#ifndef __flash
//#include <avr/pgmspace.h>
//#define __flash PROGMEM
//#endif

void fft32_execute (complex_t *);
int16_t fmuls_f (int16_t, int16_t);
int16_t fsqrt(int16_t, int16_t);

extern __flash const int16_t tbl32_window[];
extern __flash const int16_t tbl32_bitrev[];

