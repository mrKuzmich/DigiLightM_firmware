/** \file control.h
 * \brief �������� ������� ����������
 * \author ARV
 * \date	9 ���. 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 * \defgroup CONTROL ������ ���������� � ���������
 * \brief ���������� � ���������
 *
 *
 * \addtogroup CONTROL
 * @{
 */

#ifndef CONTROL_H_
#define CONTROL_H_

#include "global.h"
#include "hardware.h"
#include "sd_script.h"

/// ������� "���������" ����������� �� ��� ����� ���������� ������ � 10�� ����������
#define IND_TIMEOUT		150

#define ONLY_BASE_CMD

typedef enum{
	CTRL_NONE,
	CTRL_ENC_UP,
	CTRL_ENC_DN,
	CTRL_ENC_BTN,
	CTRL_ENC_BTN_UP,
	CTRL_ENC_BTN_DN,
	CTRL_BTN_PWR,
	CTRL_BTN_SET,
	//
	CTRL_CNT
} control_t;

typedef enum{
	// �������� �������
	CMD_NONE,
	CMD_VOLUME_PLUS,
	CMD_VOLUME_MINUS,
	CMD_EFFECT_PLUS,
	CMD_EFFECT_MINUS,
	CMD_PRESET_PLUS,
	CMD_PRESET_MINUS,
	CMD_SOURCE_SELECT,
	CMD_POWER,
#if !defined(ONLY_BASE_CMD)
	// ������������ �������
	CMD_EFFECT_1,
	CMD_EFFECT_2,
	CMD_EFFECT_3,
	CMD_EFFECT_4,
	CMD_EFFECT_5,
	CMD_EFFECT_6,
	CMD_EFFECT_7,
	CMD_EFFECT_8,
	CMD_EFFECT_9,
	CMD_EFFECT_10,
	CMD_PRESET_1,
	CMD_PRESET_2,
	CMD_PRESET_3,
	CMD_PRESET_4,
	CMD_PRESET_5,
	CMD_PRESET_6,
	CMD_PRESET_7,
	CMD_PRESET_8,
	CMD_PRESET_9,
	CMD_PRESET_10,
#endif
	CMD_SETUP,
	//
	CMD_CNT
} control_cmd_t;

void change_val(uint8_t *v, uint8_t lim, int8_t d);
/// ��������� ���� ������� ������
uint8_t get_btn_state(void);
/// ��������� ����������� �������
control_t get_control(void);
/// ��������� ����������������� �������
uint8_t do_effect(signal_t *s);
/// ��������� �������� �������
uint8_t do_bg_effect(signal_t *s);
/// ��������� ������� ����������
void do_control(signal_t *s);

/**
 * @}
 */
#endif /* CONTROL_H_ */
