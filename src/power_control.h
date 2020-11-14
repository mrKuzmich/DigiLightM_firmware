/** \file power_control.h
 * \brief Управление питанием
 *
 * \author ARV
 * \date	9 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 *
 * \defgroup PWR Управление питанием
 * \brief Возможности управления питанием
 *
 * \addtogroup PWR
 * @{
 */

#ifndef POWER_CONTROL_H_
#define POWER_CONTROL_H_

#include "global.h"

#define backlight_on()	PORT(BKLIGHT_PORT) |= BKLIGHT_PIN
#define backlight_off() PORT(BKLIGHT_PORT) &= ~BKLIGHT_PIN

/// проверка таймаута автовыключения питания
uint8_t power_timeout(signal_t *s);
/// выключение питания
void power_off(void);
/// сброс таймаута автоотключения питания
void reset_power_timeout(void);
/**
 * @}
 */
#endif /* POWER_CONTROL_H_ */
