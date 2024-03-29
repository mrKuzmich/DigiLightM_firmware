/** \file gain_ctrl.h
 * \brief ������ ���������� ���������
 *
 * \author ARV
 * \date	15 ���. 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 * \addtogroup CONTROL
 */

#ifndef GAIN_CTRL_H_
#define GAIN_CTRL_H_

/// ���������� ������� ������� �������
#define AGC_NORMAL_LVL		175
/// ���������� ���������� �������� ������ �� �����������
#define AGC_VARIATION		5
/// �������� ��������������� ����������
#define GAIN_DIV			1

typedef enum {
    MG_40DB, MG_50DB, MG_60DB,
    _MG_COUNT
} mgain_t;

/// ����������� ����������������
void sens_control(signal_t *s);
void change_mgain(void);

#endif /* GAIN_CTRL_H_ */
