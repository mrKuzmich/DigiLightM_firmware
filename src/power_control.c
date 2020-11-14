/** \file power_control.c
 * \brief ”правление питанием
 *
 * \author ARV
 * \date	9 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \addtogroup PWR
 * @{
 */

#include <avr/io.h>
#include "avr_helper.h"
#include <avr/pgmspace.h>
#include <stdlib.h>
#include "lcd.h"
#include "pixel.h"
#include "ws2812.h"
#include "control.h"
#include "main_effect.h"
#include "power_control.h"
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "lcd_show.h"
#include <avr/eeprom.h>

static uint8_t sec100 = 100;
static uint8_t sec=60, min=0;

uint8_t power_timeout(signal_t *s){
	if(cfg.time_to_sleep != 0){
		if(s->average_vol > DEF_SILENCE_LVL){
			reset_power_timeout();
		} else {
			if(!--sec100){
				sec100 = 100;
				if(!--sec){
					sec = 60;
					if(!--min){
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

void reset_power_timeout(void){
	sec100 = 100;
	sec = 60;
	min = cfg.time_to_sleep;// * 5;
}

ISR(INT2_vect){

}

INIT(6){
	DDR(BKLIGHT_PORT) |= BKLIGHT_PIN;
	backlight_off();
}

void power_off(void){
	if(get_btn_state()) return;
	save_all_effects();
	if(cfg.lcd_enabled){
		lcd_clrscr();
		center_str_p(0,PSTR("DIGILIGHT V1.0"));
		center_str_p(1,PSTR("ARV(c)2017"));
	}
	for(uint8_t i=0; i<PIXEL_CNT; i++)
		bright_ctrl(i, 0, 0);
	ws2812_show();
	backlight_off();
	eeprom_busy_wait();
	cli();
	GICR |= _BV(INT2);
	GIFR = _BV(INTF2);
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sei();
	sleep_mode();
	GICR &= ~_BV(INT2);
	backlight_on();
}

/**
 * @}
 */
