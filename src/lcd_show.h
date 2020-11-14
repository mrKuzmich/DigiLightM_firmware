/** \file lcd_show.h
 * \brief ��������������� ������� ���
 *
 * \author ARV
 * \date	21 ���. 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 * \addtogroup CONTROL
 * @{
 */

#ifndef LCD_SHOW_H_
#define LCD_SHOW_H_

/// ����� ������ �� flash � �������� ������� ������
void show_rpad_str_p(uint8_t line, const char *src);
/// ����� ������ � �������� ������� ������
void show_rpad_str(uint8_t line, char *src);
/// ����� ������ �� flash �� ������ �������
void center_str_p(uint8_t line, const char *src);
/// ����� ������ �� ������ �������
void center_str(uint8_t line, char *src);
/// ����������� "����������� �������" �� 2-� ������ ���
void show_spectrum(signal_t *s);
/// ����������� ����������������� �������������� �����
void show_scale(uint8_t row, uint8_t val);
/// ����� 4 ��������� ����� � HEX ������� � ����������� ������
void show_hex_long(uint32_t num);
/// ����� ����������� �����
void show_number(int32_t num);
/**
 * @}
 */
#endif /* LCD_SHOW_H_ */
