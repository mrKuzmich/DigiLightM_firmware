/** \file ir_remote.h
 * \brief �������� ������ �������������� ����������
 *
 * \author ARV
 * \date	11 ��� 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 * \addtogroup CONTROL
 */

#ifndef IR_REMOTE_H_
#define IR_REMOTE_H_

#include "control.h"

/// ��������� ��� �������� ����� ������
typedef struct{
	control_cmd_t	cmd;	///< ������� ����������
	uint32_t		ir;		///< ��� ������
} e_cmd_t;

/// �������� "��� �������"
#define IR_NO_CMD	0xFFFFFFFF
/// ���������� ����������� ������ � EEPROM
#define TOTAL_STORED_IR_CMD	8

/// ��������� ���� ��
uint32_t get_ir_code(void);
/// ����� ������������������� �������
control_cmd_t get_ir_command(void);
/// ��������� IR-���� �������
uint32_t get_ir_cmd_code(control_cmd_t cmd);
/// ���������� ������� IR-����
void set_ir_cmd_code(control_cmd_t cmd, uint32_t code);
void init_ir_cmd_code(uint8_t id, control_cmd_t cmd, uint32_t code);

#endif /* IR_REMOTE_H_ */
