/** \file encoder.c
 * \brief Обработка вращения энкодера
 *
 * Модуль, отслеживающий вращение энкодера, состояние кнопки отслеживается отдельно.
 * \author ARV
 * \date 04.05.2017
 * \copyright Copyright 2008 © ARV. All rights reserved.
 * \addtogroup CONTROL
 * @{
 */

#include <avr/io.h>
#include "avr_helper.h"
#include "encoder.h"
#include "hardware.h"

#define ENC_X			BTN_UP
#define ENC_Y			BTN_DN
#define ENC_ANY			(ENC_X | ENC_Y)
#define ENC_PORT		BTN_PORT

/**
 * Возвращает "мгновенное" состояние контактов энкодера
 * @return
 */
static uint8_t get_enc_state(void){
	return PIN(ENC_PORT) & ENC_ANY;
}

// обозначения битов в слове состояния энкодера
#define X1		(ENC_X)
#define X2		(ENC_X << 8)
#define Y1		(ENC_Y)
#define Y2		(ENC_Y << 8)

/*
№	y2	y1	x2	x1	Вперёд	Назад	Состояние
------------------------------------------------------------------
1	0	0	0	1	0		1		Назад		X1
8	1	0	0	0	0		1		Назад		Y2
7	0	1	1	1	0		1		Назад		Y1 | X2 | X1
E	1	1	1	0	0		1		Назад		Y2 | Y1 | X2

2	0	0	1	0	1		0		Вперёд		X2
4	0	1	0	0	1		0		Вперед		Y1
B	1	0	1	1	1		0		Вперёд		Y2 | X1 | X2
D	1	1	0	1	1		0		Вперёд		Y1 | Y2 | X1
 */

/**
 * Обработка повторотов энкодера методом конечных состояний
 * http://easyelectronics.ru/obrabotka-mnozhestva-inkrementalnyx-enkoderov-odnovremenno.html
 *
 * @return 0 - повторот не зафиксирован, +1 или -1 в случае поворота вправо или
 * влево соответственно
 */
int8_t get_encoder(void){
	static uint8_t EncState;
	static uint8_t r1, r2;

	uint8_t newValue = get_enc_state();
	uint16_t fullState = newValue | (EncState << 8);
	EncState = newValue;

	switch(fullState){
		case X1:
		case (Y1 | X2 | X1):
		case Y2:
		case (Y2 | Y1 | X2):
			r1++;
			break;
		case X2:
		case (Y2 | X1 | X2):
		case Y1:
		case (Y1 | Y2 | X1):
			r2++;
			break;
		default:
			break;
	}
	// на каждый "щелчок" энкодера формируется до 4 изменений счетчика,
	// поэтому необходимо привести его значение к множеству [-1, 0, +1]
	if(r1 >= 4){
		r1 = 0;
		return 1;
	}
	if(r2 >= 4){
		r2 = 0;
		return -1;
	}
	return 0;
}

/**
 * @}
 */
