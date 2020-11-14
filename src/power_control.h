/** \file power_control.h
 * \brief ���������� ��������
 *
 * \author ARV
 * \date	9 ���. 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 *
 * \defgroup PWR ���������� ��������
 * \brief ����������� ���������� ��������
 *
 * \addtogroup PWR
 * @{
 */

#ifndef POWER_CONTROL_H_
#define POWER_CONTROL_H_

#include "global.h"

#define backlight_on()	PORT(BKLIGHT_PORT) |= BKLIGHT_PIN
#define backlight_off() PORT(BKLIGHT_PORT) &= ~BKLIGHT_PIN

/// �������� �������� �������������� �������
uint8_t power_timeout(signal_t *s);
/// ���������� �������
void power_off(void);
/// ����� �������� �������������� �������
void reset_power_timeout(void);
/**
 * @}
 */
#endif /* POWER_CONTROL_H_ */
