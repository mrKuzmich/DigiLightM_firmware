/** \file ir_remote.c
 * \brief ������ ������� �������� IR-������� �������������� ����������
 *
 * \author ARV
 * \date	10 ��� 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 * \addtogroup CONTROL
 *
 */

#include <avr/io.h>
#include "avr_helper.h"
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "ir_remote.h"

/**
 * @{
 */

static  uint32_t	_code;		// ����������� ��� ��
static volatile uint8_t		_cnt;		// ������� �������� ���
static volatile uint8_t		_code_done;	// ���� ��������� �����

#define IR_LOG0		0x600
#define IR_DONE		(0x3000)

/**
 * ������������� ������ ���� ��
 */
static inline void start_ir_capture(void){
	// ���������
	_code = 0;
	_code_done = 0;
	_cnt = 0;
	// ��������� �������
	TCCR1B |= _BV(ICES1); // rising edge wait
	TIFR = _BV(ICF1); // clear interrupt flag
}

INIT(7){
	DDR(IR_CAP_PORT) &= ~IR_CAP_PIN;
	PORT(IR_CAP_PORT) |= IR_CAP_PIN;

	start_ir_capture();
	TIMSK |= _BV(TICIE1) | _BV(OCIE1B); // enable capture interrupt
	TCCR1B |= TIMER_CLK_DIV_8; // 0.5 uS
}

ISR(TIMER1_COMPB_vect){
	_code_done = 1;
	TIMSK &= ~_BV(OCIE1B);
}

static volatile uint16_t bit;

/**
 * ���������� ���������� ������� �� �������
 */
ISR(TIMER1_CAPT_vect){
	static uint16_t icr;

	// ������
	if(bit_is_set(TCCR1B, ICES1)){
		icr = ICR1;
		OCR1B = TCNT1 + IR_DONE;
		// ���� ����� ������������ ������, �� ���� �������� ������������ �������� ������
		TCCR1B &= ~_BV(ICES1); // falling edge wait
		TIFR = _BV(ICF1); // clear interrupt flag
		return;
	}
	// ���������� ��������� ����
	icr = (ICR1-icr) < 0 ? 0x10000 - ICR1 + icr : ICR1 - icr;
	_code <<= 1;
	if(icr > IR_LOG0){
		_code |= 1;
	} else {
	}
	_cnt++;

	icr = ICR1;
	// �������� ������ ����������
	TCCR1B |= _BV(ICES1); // falling edge wait
	TIFR = _BV(ICF1); // clear interrupt flag
	TIMSK |= _BV(OCIE1B);
}

#define DIV 100

/**
 * ��������� ���� IR-��
 * @return �������� ��� ��� 0, ���� ��� �� ������
 */
static uint32_t get_ir_code1(void){
	static uint32_t result;

	// ���� ��� �� ������, ���������� ����
	if(!_code_done) return 0;

	// ���� ������� ���� ���, �� ���������� �������� ���,
	// ������, ��� ��� ������ �����������
	if(_cnt >= 2){
		result = _code;
	}
	// ����� ��������� �����
	TIMSK &= ~_BV(TICIE1); // disable capture interrupt
	start_ir_capture();
	TIMSK |= _BV(TICIE1); // enable capture interrupt

	// ���������� ����������� ���
	return result;
}

uint32_t get_ir_code(void){
	static uint32_t code;
	static uint8_t div;

	uint32_t cd = get_ir_code1();
	if(cd == 0){
		return 0;
	} else {
		if(cd != code){
			code = cd;
			div = 10;
			return code;
		} else {
			div--;
			if(div == 0){
				div = 1;
				return code;
			} else {
				return 0;
			}
		}
	}
}

static EEMEM e_cmd_t e_cmd[TOTAL_STORED_IR_CMD];

control_cmd_t get_ir_command(void){
	uint32_t ir = get_ir_code();
	e_cmd_t ecmd;

	if(ir != 0) {
		for(uint8_t i=0; i < TOTAL_STORED_IR_CMD; i++){
			eeprom_read_block(&ecmd, &e_cmd[i], sizeof(e_cmd_t));
			if(ir == ecmd.ir){
				return ecmd.cmd;
			}
		}
	}

	return CMD_NONE;
}

uint32_t get_ir_cmd_code(control_cmd_t cmd){
	e_cmd_t ecmd;

	if(cmd != CMD_NONE){
		for(uint8_t i=0; i < TOTAL_STORED_IR_CMD; i++){
			eeprom_read_block(&ecmd, &e_cmd[i], sizeof(e_cmd_t));
			if(cmd == ecmd.cmd){
				return ecmd.ir;
			}
		}
	}
	return IR_NO_CMD;
}

void set_ir_cmd_code(control_cmd_t cmd, uint32_t code){
	e_cmd_t ecmd;

	for(uint8_t i=0; i < TOTAL_STORED_IR_CMD; i++){
		eeprom_read_block(&ecmd, &e_cmd[i], sizeof(e_cmd_t));
		if(cmd == ecmd.cmd){
			ecmd.cmd = cmd;
			ecmd.ir = code;
			eeprom_update_block(&ecmd, &e_cmd[i], sizeof(e_cmd_t));
			return;
		}
	}
}

void init_ir_cmd_code(uint8_t id, control_cmd_t cmd, uint32_t code){
	if(id < TOTAL_STORED_IR_CMD){
		e_cmd_t ecmd;
		ecmd.cmd = cmd;
		ecmd.ir = code;
		eeprom_update_block(&ecmd, &e_cmd[id], sizeof(e_cmd_t));
	}
}

/**
 * @}
 */
