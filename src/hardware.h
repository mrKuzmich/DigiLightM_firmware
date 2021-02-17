/** \file hardware.h
 * \brief Аппаратно-зависимые определения
 *
 * \author ARV
 * \date	14 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 *
 */

#ifndef HARDWARE_H_
#define HARDWARE_H_

#include <avr/io.h>

// ============ PORTA ===============
/// символ порта АЦП
#define ADC_PORT		A
/// номер пина порта АЦП для входного сигнала
#define ADC_PIN			0

/// Порт переключателя входов
#define IN_SWITCH_PORT	A
/// Пин выбора входа 1
#define IN_1_SEL_PIN	_BV(3)
/// Пин выбора входа 2
#define IN_2_SEL_PIN	_BV(4)
/// Пин выбора входа микрофона
#define IN_MC_SEL_PIN	_BV(5)

/// Символ порта индикации текущего состояния (отладка)
#define DEBUG_LED_PORT	A
#define DEBUG_LED_PIN0	6

#define WS_LOCK_PORT	A
#define WS_LOCK_PIN		_BV(7)

// ============ PORTB ===============
/// Символ порта, на который выведен аппаратный SPI
#define PORT_SPI		B
/// Номер бита линии MOSI
#define PIN_MOSI		5
/// Номер бита линии SS
#define PIN_SS			4
/// Номер бита линии SCK
#define PIN_SCK			7
#define PIN_MISO		6

/// Символ порта, к оторому подключены кнопки управления
#define BTN_PORT		B
/// Кнопка "вверх"
#define BTN_UP			_BV(0)
/// Кнопка "ввниз"
#define BTN_DN			_BV(1)
/// Кнопка изменения основного эффекта
#define BTN_CHANGE		_BV(2)
/// Кнопка настроек
#define BTN_MENU		_BV(3)
/// Маска всех кнопок
#define BTN_ANY			(BTN_MENU | BTN_UP | BTN_DN | BTN_CHANGE)
#define BTN_NONE		0

// ============ PORTC ===============
/// Порт, к которому подключен ЖКИ
#define LCD_PORT         PORTC        /**< port for the LCD lines   */

#define LCD_DATA0_PORT   LCD_PORT     /**< port for 4bit data bit 0 */
#define LCD_DATA1_PORT   LCD_PORT     /**< port for 4bit data bit 1 */
#define LCD_DATA2_PORT   LCD_PORT     /**< port for 4bit data bit 2 */
#define LCD_DATA3_PORT   LCD_PORT     /**< port for 4bit data bit 3 */
#define LCD_DATA0_PIN    3            /**< pin for 4bit data bit 0  */
#define LCD_DATA1_PIN    4            /**< pin for 4bit data bit 1  */
#define LCD_DATA2_PIN    5            /**< pin for 4bit data bit 2  */
#define LCD_DATA3_PIN    6            /**< pin for 4bit data bit 3  */
#define LCD_RS_PORT      LCD_PORT	      /**< port for RS line         */
#define LCD_RS_PIN       0            /**< pin  for RS line         */
#define LCD_RW_PORT      LCD_PORT     /**< port for RW line         */
#define LCD_RW_PIN       1            /**< pin  for RW line         */
#define LCD_E_PORT       LCD_PORT     /**< port for Enable line     */
#define LCD_E_PIN        2            /**< pin  for Enable line     */

/// Символ порта, управляющего подсветкой ЖКИ
#define BKLIGHT_PORT	C
/// Пин подсветки ЖКИ
#define BKLIGHT_PIN		_BV(7)

// ============ PORTD ===============
/// Символ порта, который используется для регулировки чувствительности - выход ШИМ OC2
#define GAIN_PORT		D
/// Выход MC_GAIN
#define MC_GAIN_PIN		_BV(5)
/// Выход AGC
#define GAIN_PIN		_BV(7)

/// Символ порта захвата IR-команд
#define IR_CAP_PORT			D
/// Пин захвата IR-команд (ICP)
#define IR_CAP_PIN		_BV(6)

#endif /* HARDWARE_H_ */
