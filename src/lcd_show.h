/** \file lcd_show.h
 * \brief Вспомогательные эффекты ЖКИ
 *
 * \author ARV
 * \date	21 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \addtogroup CONTROL
 * @{
 */

#ifndef LCD_SHOW_H_
#define LCD_SHOW_H_

/// вывод строки из flash с очисткой дисплея справа
void show_rpad_str_p(uint8_t line, const char *src);
/// вывод строки с очисткой дисплея справа
void show_rpad_str(uint8_t line, char *src);
/// вывод строки из flash по центру дисплея
void center_str_p(uint8_t line, const char *src);
/// вывод строки по центру дисплея
void center_str(uint8_t line, char *src);
/// отображение "мгновенного спектра" на 2-й строке ЖКИ
void show_spectrum(signal_t *s);
/// отображение псевдографической горизонтальной шкалы
void show_scale(uint8_t row, uint8_t val);
/// вывод 4 байтового числа в HEX формате с лидирующими нулями
void show_hex_long(uint32_t num);
/// вывод десятичного числа
void show_number(int32_t num);
/**
 * @}
 */
#endif /* LCD_SHOW_H_ */
