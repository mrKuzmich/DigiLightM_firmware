/** \file global.h
 * \brief ����� ��������
 *
 * \author ARV
 * \date	5 ���. 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 *
 */

#ifndef EFFECTS_H_
#define EFFECTS_H_
#include <stdint.h>
#include "in_switch.h"

#define MAX_GROUP_OF_PIX		128
#define MIN_GROUP_OF_PIX		16
#define MAX_PIX_IN_GROUP		8
#define MIN_PIX_IN_GROUP		1
#define MAX_TOTAL_PIX			128
#define STEP_GROUP_OF_PIX		8
#define STEP_PIX_IN_GROUP		1

/// ������ ������ ���������, ����������� �������
#define DEF_SILENCE_LVL	11

/// ������������ �������� (��� ���������� - ���������� � ��������)
typedef enum harmonic_freq_t{
	F_DC,		///< ���������� ������������ (����������� �������)
	F_250_HZ,	///< ��������� 250 ��
	F_500_HZ,	///< ��������� 500 ��
	F_750_HZ,	///< ��������� 750 ��
	F_1000_HZ,	///< ��������� 1000 ��
	F_1300_HZ,	///< ��������� 1250 ��
	F_1500_HZ,	///< ��������� 1500 ��
	F_1800_HZ,	///< ��������� 1750 ��
	F_2000_HZ,	///< ��������� 2000 ��
	F_2300_HZ,	///< ��������� 2250 ��
	F_2500_HZ,	///< ��������� 2500 ��
	F_2800_HZ,	///< ��������� 2750 ��
	F_3000_HZ,	///< ��������� 3000 ��
	F_3300_HZ,	///< ��������� 3250 ��
	F_3500_HZ,	///< ��������� 3500 ��
	F_3800_HZ,	///< ��������� 3750 ��
	// �� �������� ���� ����� �����������!
	F_CNT		///< ����� ���������� ��������
} harmonc_freq_t;

/// ����������� ���������� ��������� ����� �������
typedef enum {
	LF, MF, HF,
	// �� �������� ���� ����� �����������!
	_BAND_CNT
} band_t;

/// ���������� ���������� ��������� ����� �������
#define BAND_CNT	_BAND_CNT

/// ������� ����� ������ �������, ������������ � ��-������
#define BAND_LF	_BV(F_250_HZ) |	_BV(F_500_HZ)
/// ������� ����� ������ �������, ������������ � ��-������
#define BAND_MF	_BV(F_750_HZ) | _BV(F_1000_HZ) | _BV(F_1300_HZ) | _BV(F_1500_HZ)
/// ������� ����� ������ �������, ������������ � ��-������
#define BAND_HF	_BV(F_2800_HZ) | _BV(F_3000_HZ) | _BV(F_3300_HZ) | _BV(F_3500_HZ) | _BV(F_3800_HZ)
/// ������� ����� ������ �������, ����������� ��� ������� ������ ������� (���, ����� DC)
#define BAND_FULL ~_BV(F_DC)

/// ��� ��� ������ ���������
typedef uint8_t		volume_t;
/// ��� ��� "��������" �������� ��������� ������������
typedef uint16_t	power_t;

/// ��������� ���������� ��������� �������
typedef struct {
	volume_t	peak_volume;	///< ������� ���������
	volume_t	average_vol;	///< ������� ���������
	power_t		band[BAND_CNT];	///< "��������" ����� �������
	uint8_t		beat;			///< �������� �����
	uint16_t	harmonics[F_CNT];///< ��������� ���� �������� �������
} signal_t;

/// ������������� ���������� ������� ������������� ����������������
#define MAX_LEVEL	80

/// ��������� ������������ ����������
typedef struct {
	uint8_t		clear;				///< ������� ������ ��������
	uint8_t		group_of_pixels;	///< ���������� ����� ��������
	uint8_t		pixels_in_group;	///< ���������� �������� � ������
	uint8_t		time_to_sleep;		///< ����� ��������������
	uint8_t		agc_enabled;		///< ��� �������� (����� ���� ������������� ������ �����)
	uint8_t		lcd_enabled;		///< ��� ������������
	uint16_t	dc_offset;			/// ��������� ���������� ������������
	/// TODO: dc_offset ������� ������� ��� ������� �����, ��������� �� ����� ��� ������ ����� ����������� ��������
	uint8_t		input;				///< ����� ��������� �����
	uint8_t		colog_order;		///< ������������������ ���������� �������� ������������
//	uint8_t		mic_gain;			///< �������� ���������
	// mic_gain ����� ��������� � ��������� �������� sensitivity
	uint8_t		sensitivity[IN_CNT];///< ������� ����������� ���������������� (�� ������)
	/// �� ����� ���� sensitivity ����� ���� ��� ������ 1, 2 � mic
//	uint16_t	band_mask[BAND_CNT];///< ������� ����� �����
} config_t;

extern config_t cfg;

/** ���� ���������� ������� �������
 *
 * ���� ��� ���������� �� ����� ����, �� ������������� �������,
 * FFT ������ � ���������� ���������� ������� �� �����������. � ���� ������
 * ����� ������������� ������� ������ ���� �������.
 */
extern uint8_t lock_input;
/// ������ ��������� ������
void* get_reserved_memory(uint16_t *size);

typedef uint8_t (*effect_func_t)(signal_t *s);
typedef void (*effect_ini_t)(void);
typedef void (*effect_fini_t)(void);


/// ��������� �������
extern signal_t	music; // �� ������ ���!


#if defined(_DEBUG_)
#define debug_led_on()			PORT(DEBUG_LED_PORT) &= ~_BV(DEBUG_LED_PIN0)
#define debug_led_off()			PORT(DEBUG_LED_PORT) |= _BV(DEBUG_LED_PIN0)
#define debug_led_toggle()		PORT(DEBUG_LED_PORT) ^= _BV(DEBUG_LED_PIN0)
#else
#define debug_led_on()
#define debug_led_off()
#define debug_led_toggle()
#endif

#include "configuration.h"

#endif /* EFFECTS_H_ */
