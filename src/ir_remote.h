/** \file ir_remote.h
 * \brief описания модуля дистанционного управления
 *
 * \author ARV
 * \date	11 мая 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \addtogroup CONTROL
 */

#ifndef IR_REMOTE_H_
#define IR_REMOTE_H_

#include "control.h"

/// Структура для хранения кодов команд
typedef struct{
	control_cmd_t	cmd;	///< команда управления
	uint32_t		ir;		///< код пульта
} e_cmd_t;

/// Заглушка "нет команды"
#define IR_NO_CMD	0xFFFFFFFF
/// Количество сохраняемых команд в EEPROM
#define TOTAL_STORED_IR_CMD	8

/// получение кода ДУ
uint32_t get_ir_code(void);
/// поиск запрограммированной команды
control_cmd_t get_ir_command(void);
/// получение IR-кода команды
uint32_t get_ir_cmd_code(control_cmd_t cmd);
/// назначение команде IR-кода
void set_ir_cmd_code(control_cmd_t cmd, uint32_t code);
void init_ir_cmd_code(uint8_t id, control_cmd_t cmd, uint32_t code);

#endif /* IR_REMOTE_H_ */
