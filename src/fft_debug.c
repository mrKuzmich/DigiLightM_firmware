/** \file fft_debug.c
 * \brief отладочный модуль "эффекта"
 *
 * Модуль содержит средства, позволяющие увидеть на ЖКИ состояния внутренних
 * переменных в режиме "реального времени". Регистрируется в системе, как единственный
 * модуль "эффекта" как для фона, так и для активного режима.
 * \author ARV
 * \date	6 июня 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 *
 * Для компиляции требуется:\n
 * 	-# AVR GCC 5.2.1 или более новая версия
 *
 */

#if defined(_FFT_DEBUG_)

#include <avr/io.h>
#include "avr_helper.h"
#include <stdlib.h>
#include "global.h"
#include "fft_debug.h"
#include "lcd_show.h"
#include "lcd.h"
#include "main_effect.h"
#include "spectrum.h"

// перечисление того, что отображается на ЖКИ
typedef enum{
	ADC_OFFS,
	REAL_OFS,
	VOL_PEAK,
	VOL_AVE,
	F_0,
	F_250,
	F_500,
	F_750,
	F_1000,
	F_1300,
	F_1500,
	F_1800,
	F_2000,
	F_2300,
	F_2500,
	F_2800,
	F_3000,
	F_3300,
	F_3500,
	F_3800,
	B_LF,
	B_MF,
	B_HF,
	//
	SHOW_CNT
} show_t;

// текстовые названия выводимых значений
static __flash const char names[SHOW_CNT][9] = {
	"ADC_OFFS",
	"REAL_OFFS",
	"VOL_PEAK",
	"VOL_AVE",
	"F_0",
	"F_250",
	"F_500",
	"F_750",
	"F_1000",
	"F_1300",
	"F_1500",
	"F_1800",
	"F_2000",
	"F_2300",
	"F_2500",
	"F_2800",
	"F_3000",
	"F_3300",
	"F_3500",
	"F_3800",
	"B_LF",
	"B_MF",
	"B_HF"
};

// текущее отображаемое на ЖКИ
static show_t item = VOL_PEAK;

// отображение числа
static void show_val(uint16_t v){
	char str[16];
	utoa(v, str, 10);
	lcd_puts(str);
}

// функция индикации на ЖКИ в режиме отладки
void _fft_debug_show(signal_t *s){
	// заголовок
	show_rpad_str_p(0, PSTR("DEBUG FFT"));
	// наименование выводимой переменной
	show_rpad_str_p(1,names[item]);
	lcd_gotoxy(9,1);
	lcd_putc('=');
	// вывод значения
	switch(item){
	case ADC_OFFS:
		show_val(cfg.dc_offset);
		break;
	case REAL_OFS:
		show_val(real_offset);
		break;
	case VOL_PEAK:
		show_val(s->peak_volume);
		break;
	case VOL_AVE:
		show_val(s->average_vol);
		break;
	case F_0 ... F_3800:
		show_val(s->harmonics[item - F_0]);
		break;
	case B_LF ... B_HF:
		show_val(s->band[item - B_LF]);
		break;
	}
}

// ниже следуют обязательные для модуля эффекта описания - без комментариев

static void _effect(signal_t *s);
static preset_result_t _preset(int8_t d);

static __flash const char _name[] = "@";

static flash_effect_t effect_def = {
	.name	= _name,
	.start	= NULL,	// может быть NULL
	.work	= _effect,	// это значение не может быть NULL!
	.stop	= NULL,	// может быть NULL
	.preset	= _preset,	// может быть NULL
	.save 	= NULL		// может быть NULL
};

static flash_effect_t effect_def_b = {
	.name	= _name,
	.start	= NULL,	// может быть NULL
	.work	= _effect,	// это значение не может быть NULL!
	.stop	= NULL,	// может быть NULL
	.preset	= _preset,	// может быть NULL
	.save 	= NULL		// может быть NULL
};

INIT(7){
	register_effect(BACKGROUND_EFFECT, &effect_def_b);
	register_effect(FOREGROUND_EFFECT, &effect_def);
}

// главная функция эффекта - вызывается 100 раз в секунду для рендеринга
static void _effect(signal_t *s){
}

// смена пресета эффекта - вызывается в произвольные моменты
static preset_result_t _preset(int8_t d){
	item += d;
	if((d < 0) && (item == ADC_OFFS)) item = SHOW_CNT-1;
	if(item >= SHOW_CNT) item = ADC_OFFS;
	return PRESET_HIDDEN;
}

#endif
