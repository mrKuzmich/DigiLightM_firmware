/** \file gain_ctrl.h
 * \brief Модуль управления усилением
 *
 * \author ARV
 * \date	15 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \addtogroup CONTROL
 */

#ifndef GAIN_CTRL_H_
#define GAIN_CTRL_H_

/// Нормальный средний уровень сигнала
#define AGC_NORMAL_LVL		175
/// Допустимое отклонение среднего уровня от нормального
#define AGC_VARIATION		5
/// Задержка автоматического регулятора
#define GAIN_DIV			1

/// Регулировка чувствительности
void sens_control(signal_t *s);

#endif /* GAIN_CTRL_H_ */
