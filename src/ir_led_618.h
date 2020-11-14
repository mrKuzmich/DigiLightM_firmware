/** \file ir_car_mp3.h
 * \brief Коды команд пульта управление LED618
 * \author ARV \par
 * \date	12 мая 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 *
 * Для компиляции требуется:\n
 * 	-# AVR GCC 5.2.1 или более новая версия
 *
 */

#ifndef IR_LED_618_H_
#define IR_LED_618_H_

#define IR_CH_PLUS		0xFF7FEFDF
//#define IR_CH_MINUS		0xFF7FDDAE
#define IR_NEXT		0xFF7FDF6F
#define IR_PREV		0xFF7FDFAF
//#define IR_VOL_PLUS		0xFF7FD7AB
//#define IR_VOL_MINUS		0xFF7FFFEF
#define IR_NUM_0			0xFF7FB76B
#define IR_NUM_1			0xFF7FDF77
#define IR_NUM_2			0xFF7FEF7B
#define IR_NUM_3			0xFF7FBD7A
#define IR_NUM_4			0xFF7FEF77
#define IR_NUM_5			0xFF7FDF7B
#define IR_NUM_6			0xFF7FAD5A
#define IR_NUM_7			0xFF7FBD5E
#define IR_NUM_8			0xFF7FB55A
#define IR_NUM_9			0xFF7FAD56

#define IR_EQ				0xFF7FEFB7
#define IR_NUM_PLUS100		0xFF7FEFBB
#define IR_NUM_PLUS200		0xFF7FDFB7
#define IR_PLAY_PAUSE		0xFF7FFDDE
#define IR_CHANEL			0xFF7FBD6E

#endif /* IR_LED_618_H_ */
