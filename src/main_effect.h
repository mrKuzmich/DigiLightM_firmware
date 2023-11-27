/** \file main_effect.h
 * \brief ��������� ���������� ��������
 *
 * \author ARV
 * \date	18 ���. 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 * \defgroup EFFECTS ���������� ������� �����������
 * \brief ����������� �������� ��� ������������ �������
 *
 * ���������� ������� ��������� �� �������� � �������.
 * \n �������� ������� ������������ ����������� ������, �.�. ��� � ���� ����������
 * �����������.
 * \n ������� ������� �������� ��� ���������� �������� �������, �.�. ���������
 * ������� ��������� � ������.
 * \par �������� �������� ��������
 * \n ��� ������� ����������� � ���� ����������� �������, ������� ������������� �����������
 * � ����� ������� ��� ������ ������� #register_effect. ������ �������, �� �����������
 * ���� ��������, � �������� �������� �� �������� (����������� ������������� �����������).
 * \n ������ ������� ������ ����������� ��������� ������� ������� �������, ��������
 * ������ ������� ����� �����: ���������� ���������������� ���������� � ���������
 * ��������� ��������� ������� � �� ������ ���� ���������� �������� ����������
 * ������� ��������. ������� ������� ���������� ������ 10 ��, �.�. 100 ��� � �������,
 * ��� ����� ������������ �� ������� ������� ��� �������� ��������.
 * \n ����� ������� �������, ������ ������� ����� ��������� � ���������������
 * �������, ������� ������������ ��� ��������� ��������� ��������� ������� �������.
 * \par ������� ������:
 * - \ref FGEFFECTS
 * - \ref BGEFFECTS
 *
 * \defgroup FGEFFECTS �������� �������
 * ���������������� ������� ��������, ���� �� ���� ��������� �������� ������.
 *
 * \defgroup BGEFFECTS ������� ������� ���������
 *
 * \addtogroup EFFECTS
 * @{
 */

#ifndef MAIN_EFFECT_H_
#define MAIN_EFFECT_H_

#include "global.h"
#include "sd_script.h"

/// ���������� ���������� �������� ������ ����
#define MAX_EFFECT_CNT		7
/// ������� ������
#define BACKGROUND_EFFECT	0
/// �������� ������
#define FOREGROUND_EFFECT	1

/// ��������� �������� �� FLASH
typedef __flash const char * flash_str_t;

/// ��������� ������� ����� �������
typedef enum{
	// ����� ������������� ����� ��� ���� - ����� �������
	PRESET_NOTHING	= -1, ///< ��� ��������
	PRESET_HIDDEN	= -2  ///< ���������� �������� ������� �� ����
} preset_result_t;

typedef enum{
	INFO_ONCE,
	INFO_ALWAYS
} effect_info_t;

/// ��� ��������� �������� �������
typedef struct {
	flash_str_t		name;					///< ��������
	void			(*start)(void);			///< ������ ������
	void			(*work)(signal_t *s);	///< ���������
	preset_result_t	(*preset)(int8_t d);	///< ����� �������
	void			(*stop)(void);			///< ����� ������
	void			(*save)(void);			///< ���������� ��������
	effect_info_t	(*info)(uint8_t show);			///< ����� ����������
} effect_t;

/// ��� �������� �������, ������������ �� FLASH
typedef const __flash effect_t flash_effect_t;

/// ����������� ������� � �������
void register_effect(uint8_t toe, flash_effect_t *eff);
/// ��������� �������
void execute_effect(signal_t *s);
/// ����� ������� ��� ��������������� ���������������
int8_t change_preset(signal_t *s, int8_t delta);
/// �������� �������� �������
char* get_effect_name(signal_t *s);
/// ����� ������� ���������������
void change_effect(signal_t *s, int8_t delta);
/// ����� ������� ���������������
void select_fg_effect(signal_t *s, uint8_t num);
/// ����� ������� ���������������
void select_bg_effect(signal_t *s, uint8_t num);
/// ���������� �������� ���� ��������
void save_all_effects(void);
/// �������� �������� ������� ��� �������� �������
//char* get_preset_name(signal_t *s);
void show_info(signal_t *s);

/// �������� ����� �������� ������� �� �������� ��� ��������
uint8_t effect_auto_changed(signal_t *s);

#endif /* MAIN_EFFECT_H_ */

/**
 * @}
 */
