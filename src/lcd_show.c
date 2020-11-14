/** \file lcd_show.c
 * \brief Эффекты ЖКИ
 *
 * Вспомогательные функции для индикации на ЖКИ
 * \author ARV
 * \date	21 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \addtogroup CONTROL
 * @{
 */

#include <avr/io.h>
#include "avr_helper.h"
#include <string.h>
#include <stdlib.h>
#include <avr/wdt.h>
#include "lcd.h"
#include "global.h"
#include "lcd_show.h"

#define SYMBOL_SPECTRUM		0
#define SYMBOL_SCALE		1

static uint8_t lcd_symbol_mode;

/**
 * Установка пользовательского знакогенератора для отображения спектра
 */
static void set_chargen_spectrum(){
	lcd_command(0x40); 							// установка адреса знакогенератора
	for(uint8_t i=0; i<8; i++)					// запись всех дополнительных символов
		for(uint8_t j=0; j < 8; j++)			// строки символа
			lcd_data(j<(7-i) ? 0 : 0xFF);		// постепенное заполнение
	lcd_symbol_mode = SYMBOL_SPECTRUM;
}

/**
 * Установка пользовательского знакогенератора для отображения шкалы
 */
static void set_chargen_scale(){
	uint8_t mask = 0;

	lcd_command(0x40); 							// установка адреса знакогенератора
	for(uint8_t i=0; i<8; i++){					// запись всех дополнительных символов
		for(uint8_t j=0; j < 8; j++){			// строки символа
			lcd_data(j==3 ? 0xFF : mask);		// постепенное заполнение
		}
		mask = (mask >> 1) | 0x10;
	}
	lcd_symbol_mode = SYMBOL_SCALE;
}

/**
 * @{
 */

INIT(0){
	// важно - запрет WDT на самом раннем этапе
	wdt_disable();				//
}


INIT(7){
	// отработка наличия LCD
#if defined(__AVR_ATmega32__)
	if(MCUCSR & (_BV(PORF) | _BV(EXTRF))){
		// подано питание
		MCUCSR &= ~(_BV(PORF) | _BV(EXTRF));
#elif defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
	if(MCUSR & (_BV(PORF) | _BV(EXTRF))){
		MCUSR &= ~(_BV(PORF) | _BV(EXTRF));
#endif
		wdt_enable(WDTO_30MS);		// включаем сторожа
		lcd_init();					// пробуем инициализировать
		wdt_disable();				// сюда попадаем, если не зависли
		lcd_clrscr();
		cfg.lcd_enabled = 1;		// ЖКИ есть
		set_chargen_spectrum();
	} else {
		wdt_disable();
		cfg.lcd_enabled = 0;		// ЖКИ нет
	}

}

// размер вспомогательного буфера на 1 байт больше, чем строка ЖКИ
#define BUF_SZ	(LCD_DISP_LENGTH+1)
// вспомогательный буфер
static char tmps[BUF_SZ];
/**
 * вывод содержимого вспомогательного буфера в указанную строку ЖКИ
 * @param row номер строки (начиная с 0)
 */
static void put_tmps(uint8_t row){
	tmps[LCD_DISP_LENGTH] = 0;
	lcd_gotoxy(0, row);
	lcd_puts(tmps);
}

void show_number(int32_t num){
	char str[17];
	ltoa(num, str, 10);
	show_rpad_str(1, str);
}

/**
 * Выводит строку из flash по центру
 * @param row номер строки для вывода на ЖКИ
 * @param src строка во flash
 */
void center_str_p(uint8_t row, const char *src){
	memset(tmps, ' ', LCD_DISP_LENGTH);
	uint8_t l = strlen_P(src);
	memcpy_P(&tmps[(LCD_DISP_LENGTH-l)/2], src, l);
	put_tmps(row);
}

/**
 * Выводит строку по центру
 * @param row номер строки для вывода на ЖКИ
 * @param src строка
 */
void center_str(uint8_t row, char *src){
	memset(tmps, ' ', LCD_DISP_LENGTH);
	uint8_t l = strlen(src);
	memcpy(&tmps[(LCD_DISP_LENGTH-l)/2], src, l);
	put_tmps(row);
}

/**
 * Выводит строку из flash, затирая свободные правые позиции на ЖКИ
 * @param line номер строки для вывода
 * @param src строка во flash
 */
void show_rpad_str_p(uint8_t line, const char *src){
	memset(tmps, ' ', LCD_DISP_LENGTH);
	uint8_t l = strlen_P(src);
	memcpy_P(tmps, src, l);
	put_tmps(line);
}

/**
 * Выводит строку, затирая свободные правые позиции на ЖКИ
 * @param line номер строки для вывода
 * @param src строка во flash
 */
void show_rpad_str(uint8_t line, char *src){
	memset(tmps, ' ', LCD_DISP_LENGTH);
	uint8_t l = strlen(src);
	memcpy(tmps, src, l);
	put_tmps(line);
}

#define D0		5
#define D1		1
#define D2		1

/**
 * Отображает спектр звукового сигнала
 * @param s параметры звукового сигнала
 */
void show_spectrum(signal_t *s){
	static uint8_t div;
	static uint8_t v[F_CNT];
	static uint8_t d[F_CNT];

	if(--div) return;
	div = D0;

	if(lcd_symbol_mode != SYMBOL_SPECTRUM) set_chargen_spectrum();

	uint8_t ch;
	lcd_gotoxy(0,1);
	for(uint8_t i=0; i<F_CNT; i++){
		ch = (s->harmonics[i] / ((i < 2) ? 40 : 20));
		if(ch > 8) ch = 8;
		if(v[i] < ch){
			v[i] = ch;
			d[i] = D1;
		}
		if(d[i]){
			d[i]--;
			if(!d[i]){
				if(v[i]) v[i]--;
				if(v[i]) d[i] = D2;
			}
		}

		lcd_putc(v[i] ? v[i]-1 : ' ');
	}
}

/**
 * Отображает шкалу уровня в виде полоски "#######------"
 * @param row номер строки для вывода шкалы
 * @param val значение уровня от 0 до #LCD_DISP_LENGTH
 * \note Значение val может быть и больше, но любые значения более
 * указанного предела никак не отобразятся на шкале: она будет однаково
 * заполненной для всех этих значений.
 */
void show_scale(uint8_t row, uint8_t val){
	if(lcd_symbol_mode != SYMBOL_SCALE) set_chargen_scale();

	//val = MAX_LEVEL - val;

	uint8_t full = val / (MAX_LEVEL / LCD_DISP_LENGTH);
	uint8_t nf = val % (MAX_LEVEL / LCD_DISP_LENGTH);
	lcd_gotoxy(0,row);
	for(uint8_t i=0; i<LCD_DISP_LENGTH; i++){
		if(i < full) lcd_putc(6);
		if(i > (full)) lcd_putc(0);
		if(i == (full)) lcd_putc(nf);
	}
}

void show_hex_long(uint32_t num){
	static __flash const char chr[16] = "0123456789ABCDEF";
	uint8_t *ptr = (void*)&num;
	for(int8_t i=3; i >= 0; i--){
		lcd_putc(chr[ptr[i] >> 4]);
		lcd_putc(chr[ptr[i] & 0x0F]);
	}
	for(uint8_t x=8; x < LCD_DISP_LENGTH; x++)
		lcd_putc(' ');
}

/**
 * @}
 */
