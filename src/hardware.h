/** \file hardware.h
 * \brief ���������-��������� �����������
 *
 * \author ARV
 * \date	14 ���. 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 *
 */

#ifndef HARDWARE_H_
#define HARDWARE_H_

#include <avr/io.h>

// ============ PORTA ===============
/// ������ ����� ���
#define ADC_PORT		A
/// ����� ���� ����� ��� ��� �������� �������
#define ADC_PIN			0

/// ���� ������������� ������
#define IN_SWITCH_PORT	A
/// ��� ������ ����� 1
#define IN_1_SEL_PIN	_BV(3)
/// ��� ������ ����� 2
#define IN_2_SEL_PIN	_BV(4)
/// ��� ������ ����� ���������
#define IN_MC_SEL_PIN	_BV(5)

/// ������ ����� ��������� �������� ��������� (�������)
#define DEBUG_LED_PORT	A
#define DEBUG_LED_PIN0	6

#define WS_LOCK_PORT	A
#define WS_LOCK_PIN		_BV(7)

// ============ PORTB ===============
/// ������ �����, �� ������� ������� ���������� SPI
#define PORT_SPI		B
/// ����� ���� ����� MOSI
#define PIN_MOSI		5
/// ����� ���� ����� SS
#define PIN_SS			4
/// ����� ���� ����� SCK
#define PIN_SCK			7
#define PIN_MISO		6

/// ������ �����, � ������� ���������� ������ ����������
#define BTN_PORT		B
/// ������ "�����"
#define BTN_UP			_BV(0)
/// ������ "�����"
#define BTN_DN			_BV(1)
/// ������ ��������� ��������� �������
#define BTN_CHANGE		_BV(2)
/// ������ ��������
#define BTN_MENU		_BV(3)
/// ����� ���� ������
#define BTN_ANY			(BTN_MENU | BTN_UP | BTN_DN | BTN_CHANGE)
#define BTN_NONE		0

// ============ PORTC ===============
/// ����, � �������� ��������� ���
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

/// ������ �����, ������������ ���������� ���
#define BKLIGHT_PORT	C
/// ��� ��������� ���
#define BKLIGHT_PIN		_BV(7)

// ============ PORTD ===============
/// ������ �����, ������� ������������ ��� ����������� ���������������� - ����� ��� OC2
#define GAIN_PORT		D
/// ����� MC_GAIN
#define MC_GAIN_PIN		_BV(5)
/// ����� AGC
#define GAIN_PIN		_BV(7)

/// ������ ����� ������� IR-������
#define IR_CAP_PORT			D
/// ��� ������� IR-������ (ICP)
#define IR_CAP_PIN		_BV(6)

#endif /* HARDWARE_H_ */
