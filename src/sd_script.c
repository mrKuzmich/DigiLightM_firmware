/** \file sd_script.c
 * \brief \b "SCRIPT PLAYER"
 * \n Mодуль воспроизведения скриптов с SD-карты
 *
 * Модуль работает, как эффект фона. Модуль отключает анализ звукового сигнала,
 * поэтому может быть выключен только принудительно.
 * \author ARV
 * \date	15 нояб. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 *
 * Для компиляции требуется:\n
 * 	-# AVR GCC 5.2.1 или более новая версия
 *
 * \addtogroup BGEFFECTS
 * \copybrief sd_script.c См. \ref sd_script.c
 */

#include <avr/io.h>
#include <avr_helper.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "global.h"
#include "pixel.h"
#include "color_transform.h"
#include "avr_helper.h"
#include "hardware.h"
#include "main_effect.h"
#include "lcd_show.h"
#include <util/delay.h>
#include "PFFS/pff.h"
#include "sd_script.h"
#if !defined(DISABLE_SD_SCRIPT)
/**
 * @{
 */

// костыль - запрет WS2812b
#define enter_sd_mode()		do { SPCR = _BV(SPE) | _BV(MSTR); PORT(WS_LOCK_PORT) &= ~WS_LOCK_PIN;} while(0)
// костыль - разрешение WS2812b
#define leave_sd_mode()		do { SPCR = _BV(SPE) | _BV(MSTR) | _BV(CPHA); SPDR=0;  while(!(SPSR & (1<<SPIF))); PORT(WS_LOCK_PORT) |= WS_LOCK_PIN; } while(0)

#define MODULE_NAME		"SCRIPT PLAYER"

static __flash const char _name[] = MODULE_NAME;

/// структура скрипта
typedef struct{
	char		filename[13];	///< имя файла скрипта
	uint32_t	pos;			///< следующая позиция в файле для чтения
	uint16_t 	readed;			///< считанное в последний раз кол-во байтов
	char 		*cmd;
} script_t;

/// текущий скрипт
static script_t	script;

/// КОНСТАНТА - размер блока чтения с карты. Может быть МЕНЬШЕ, но НЕ БОЛЬШЕ 256
#define SD_BUF_SZ	256UL

/// тип текущего состояния модуля
typedef enum{
	SCRIPT_OK,			///< нормальная работа
	SCRIPT_NO_SD,		///< карта не вставлена
	SCRIPT_NOTFOUND,	///< файл скрипта не найден на карте
	SCRIPT_ERROR_IO,	///< ошибка чтения с карты
	SCRIPT_INFLOOP		///< в скрипте есть бесконечный цикл
#if defined(_DEBUG_)
	,DBG_ERROR			// отладочное состояние
#endif
} script_result_t;

static script_result_t	sdcard_open(void);
static uint16_t get_script_count(void);
static script_result_t	script_open(script_t *s, uint16_t id);

/// считывание строки из файла в буфер
static script_result_t script_read_str(script_t *s, char *str);
static script_result_t script_seek(script_t *s, uint32_t pos);
static void script_exec(signal_t *s);

static int8_t select_next = 0;

/// КОНСТАНТА - количество переменных в скрипте, НЕ МЕНЯТЬ
#define VAR_CNT		16
/// глубина вложенности циклов в скрипте
#define STACK_DEPTH	5

/// структура параметров скрипта
typedef struct{
	uint8_t		bright;			///< яркость
	uint8_t		fade;			///< затухание
	rgb_t		color;			///< цвет
	uint8_t		var[VAR_CNT];	///< переменные
	uint8_t		rev;			///< реверс
	uint8_t		stack_head;		///< "голова" стека циклов
	uint32_t	loop_stack[STACK_DEPTH];///< стек циклов
} script_param_t;

/// параметры скрипта
static script_param_t	param;
/// указатель на буфер кеширования
static char *buf;
/// текущее состояние
static script_result_t status;
/// количество файлов со скриптами на карте
static uint16_t total_script_files;
/// номер текущего исполняемого скрипта
static uint16_t script_id;
/// файловая система
static FATFS	fs;
/// текущая директория
static DIR		dir;

/** Обнаружение SD-карты
 *
 * @return результат обнаружения
 */
static script_result_t	sdcard_open(void){
	// монтируем файловую систему
	if(pf_mount(&fs) == FR_OK){
		// если смонтировалась - открываем корневую директорию
		if(pf_opendir(&dir, "") == FR_OK){
			// если открылась - ОК
			return SCRIPT_OK;
		}
	}
	// при любом сбое возвращаем ошибку
	return SCRIPT_NO_SD;
}

/**
 *
 * @return кол-во найденных файлов скриптов
 */
static uint16_t get_script_count(void){
	FRESULT res;
	FILINFO file;
	uint16_t count = 0;

	// инициализация сканирования директории
	res = pf_readdir(&dir, 0);
	// пока сканирование успешно
	while(res == FR_OK){
		// считываем очередную запись о файле
		res = pf_readdir(&dir, &file);
		// прекращаем цикл при ошибке или конце записей
		if (res || !file.fname[0]) break;
		// проверяем, что запись - НЕ директория и НЕ скрытый файл,
		// и при этом расширение у него правильное
		if (!(file.fattrib & (AM_DIR|AM_HID)) && strstr(file.fname, ".SC"))
			count++; // ведем подсчет "правильных" файлов
	}
	return count;
}

/**
 *
 * @param s ссылка на объект скрипта
 * @param id номер файла
 * @return результат операции
 */
static script_result_t	script_open(script_t *s, uint16_t id){
	FRESULT res;
	FILINFO file;
	// файлы отсчитываются с 0, но их количество - с 1, поэтому корректируем номер
	id++;
	// отсчитываем нужное количество файлов в директории
	res = pf_readdir(&dir, 0);
	do {
		res = pf_readdir(&dir, &file);
		if (res || !file.fname[0]) break;
		if (!(file.fattrib & (AM_DIR|AM_HID)) && strstr(file.fname, ".SC")) id--;
	} while(id && (res == FR_OK));

	if(res != FR_OK)
		return SCRIPT_NO_SD;
	else {
		// если добрались до указанного номера файла - инициализируем объект скрипта
		memset(s, 0, sizeof(script_t));
		memset(&param, 0, sizeof(script_param_t));
		memset(buf, 0, SD_BUF_SZ);
		strncpy(s->filename, file.fname, 12);
		// и пробуем открыть его
		return pf_open(s->filename) == FR_OK ? SCRIPT_OK : SCRIPT_NOTFOUND;
	}
}

/**
 * Если при считывании возникла ошибка, буфер в первой позиции будет содержать 0
 * @param s указатель на скрипт
 * @param str указатель на буфер
 * @return результат операции
 */
static script_result_t script_read_str(script_t *s, char *str){
	uint16_t n = SD_BUF_SZ;
	if(status == SCRIPT_OK){
		// позиционируем файл
		if(pf_lseek(s->pos) == FR_OK){
			// читаем в кэш
			if(pf_read(str, SD_BUF_SZ, &s->readed) == FR_OK){
				// корректируем считанное кол-во байтов
				if(s->readed < n) n = s->readed;
				// ищем последнюю "целую" команду и после неё ставим пометку конца строки
				uint8_t b = n-1;
				// поиск "целой" команды прост: ищем первый символ-разделитель от конца считанных данных
				while(n && str[b] && (str[b] != ' ') && (str[b] != '\n') && (str[b] != '\r') && (str[b] != '\t')) {
					n--;
					b = n-1;
				}
				// помечаем найденное место
				str[n] = 0;
				// корректируем считанное количество и позицию файла
				s->readed = n;
				s->pos += n;
				return SCRIPT_OK;
			}
		}
	}
	return SCRIPT_ERROR_IO;
}

/**
 *
 * @param s указатель на объект скрипта
 * @param pos новая позиция в файле
 * @return результат операции
 */
static script_result_t script_seek(script_t *s, uint32_t pos){
	s->pos = pos;
	return pf_lseek(pos) == FR_OK ? SCRIPT_OK : SCRIPT_ERROR_IO;
}

/**
 * Извлечение шестнадцатеричной цифры
 * @param str указатель на символ в строке
 * @return значение 0...15 или 255, если символ ошибочный
 */
static uint8_t get_script_digit(char *str){
	if((str[0] >= '0') && (str[0] <= '9')) return str[0] - '0';
	else if((str[0] >= 'A') && (str[0] <= 'F')) return str[0] - 'A' + 10;
	else return 255;
}

#define ERROR_CH	0xFF00

/**
 * Извлечение ЧИСЛА скрипта из строки
 * \note Под числом понимается в том числе и значение переменных или функций.
 * @param str указатель на число
 * @return значение числа в пределах 0...255 или 0xFF00 в случае некорректного числа в строке
 */
static uint16_t get_script_val(char *str){
	// это переменная?
	if(str[0] == 'V'){
		// извлекаем номер переменной
		uint8_t n = get_script_digit(++str);
		// номер переменной должен быть корректным
		return n < VAR_CNT ? param.var[n] : ERROR_CH;
	} else if (str[0] == 'R'){
		// функция RD или RP
		if(str[1] == 'D') return rand() & 0x00FF;
		else if(str[1] == 'P') return rand() % PIXEL_CNT;
		else return ERROR_CH;
	} else if (str[0] == 'T'){
		// функция TP
		if(str[1] == 'P') return PIXEL_CNT-1;
		else return ERROR_CH;
	} else {
		// байт
		uint8_t nh = get_script_digit(str++);
		uint8_t nl = get_script_digit(str);
		if((nh < 0x10) && (nl < 0x10)) return (nh << 4) | nl;
		else return ERROR_CH;
	}
}

/**
 * Извлекает позицию файла из стека циклов
 * @return новая позиция
 */
static uint32_t popstack(void){
	// стек должен быть заполнен
	if(param.stack_head != 0){
		// сдвигаем "голову"
		param.stack_head--;
		// возвращаем позицию
		return param.loop_stack[param.stack_head];
	}
	else // если стек пуст - придется с начала файла повторять
		return 0;
}

/**
 * Погружение в стек циклов
 * @param d позиция первой команды цикла
 */
static void pushstack(uint32_t d){
	// в стеке должно быть место
	if(param.stack_head < (STACK_DEPTH-1)){
		param.loop_stack[param.stack_head++] = d;
	}
	// а если места в стеке нет - цикл будет потерян, и это ошибка скрипта
}

/**
 * Исполнение команды из строки
 * @param cmd указатель на команду в строке
 * @return длительность задержки, которую надо выдержать до исполнения следующей команды
 */
static uint8_t execute_cmd(char *cmd){
	uint16_t tmp;
	// цикл только для удобства прекращения парсинга
	do{
		// проверка на команду с переменными
		if(cmd[0] == 'V'){
			// номер переменной
			uint8_t n = get_script_digit(cmd+1);
			if(n >= VAR_CNT) break;
			// параметр команды
			tmp = get_script_val(cmd+3);
			if(tmp > 255) break;
			// разбор оператора команды
			switch(cmd[2]){
			case '=': param.var[n] = tmp; break;
			case '+': tmp += param.var[n]; param.var[n] = tmp > 255 ? 255 : tmp; break;
			case '-': tmp = param.var[n] - tmp; param.var[n] = tmp < 0 ? 255: tmp; break;
			case '*': tmp *= param.var[n]; param.var[n] = tmp > 255 ? 255 : tmp; break;
			case '/': if(tmp == 0) return 0;
					  param.var[n] /= tmp;
					  break;
			case '%': if(tmp == 0) return 0;
					  param.var[n] %= tmp;
					  break;
		    // пока не отлаженные действия
			//case '>' param.var[n] = param.var[n] > tmp; break;
			//case '<' param.var[n] = param.var[n] < tmp; break;
			//case '=' param.var[n] = param.var[n] == tmp; break;
			//case '!' param.var[n] = param.var[n] != tmp; break;
			default:
				// недопустимый оператор
				break;
			}
			// любая строка, начинающаяся на V, но не попавшая в разбор - игнор
			break;
		}
		// проверка на команду с параметром
		if(cmd[2] == '='){
			// получаем параметр
			tmp = get_script_val(cmd+3);
			if(tmp > 255) break;
			// если параметр валидный - разбираем команду
			if(strncmp_P(cmd, PSTR("PB"), 2) == 0){
				// яркость
				param.bright = tmp;

			} else if(strncmp_P(cmd, PSTR("GB"), 2) == 0){
				// глобальная яркость
				for(uint8_t i=0; i<PIXEL_CNT; i++)
					pixels[i].bright = tmp;

			} else if(strncmp_P(cmd, PSTR("PF"), 2) == 0){
				// затухание
				param.fade = tmp;

			} else if(strncmp_P(cmd, PSTR("GF"), 2) == 0){
				// глобальное затухание
				for(uint8_t i=0; i<PIXEL_CNT; i++)
					pixels[i].delta = tmp;

			} else if(strncmp_P(cmd, PSTR("PC"), 2) == 0){
				// цвет
				uint16_t tmp2 = get_script_val(cmd+5);
				uint16_t tmp3 = get_script_val(cmd+7);
				if((tmp2 > 255) || (tmp3 > 255)) break;
				param.color.r = tmp;
				param.color.g = tmp2;
				param.color.b = tmp3;

			} else if(strncmp_P(cmd, PSTR("GC"), 2) == 0){
				// цвет глобально
				uint16_t tmp2 = get_script_val(cmd+5);
				uint16_t tmp3 = get_script_val(cmd+7);
				if((tmp2 > 255) || (tmp3 > 255)) break;
				for(uint8_t i=0; i<PIXEL_CNT; i++)
					set_rgb_color(i, tmp, tmp2, tmp3);

			} else if(strncmp_P(cmd, PSTR("PM"), 2) == 0){
				// карта пикселов
				char *ptr = cmd+5;
				while(tmp <= 255){
					if(tmp < PIXEL_CNT){
						if(param.rev) tmp = PIXEL_CNT - tmp - 1;
						// только для допустимого номера пиксела
						pixels[tmp].bright = param.bright;
						pixels[tmp].delta = param.fade;
						pixels[tmp].r = param.color.r;
						pixels[tmp].g = param.color.g;
						pixels[tmp].b = param.color.b;
					} // недопустимые номера пикселов игнорируются без прекращения парсинга
					tmp = get_script_val(ptr);
					ptr += 2;
				}

			} else if(strncmp_P(cmd, PSTR("WT"), 2) == 0){
				// задержка
				return tmp;

			}
		}
		// проверка на команду конца цикла
		if(strncmp_P(cmd, PSTR("LV"), 2) == 0){
			// номер переменной цикла
			uint8_t n = get_script_digit(cmd+2);
			if(n > VAR_CNT) break;
			// проверка условия
			if((cmd[3] == '=') || (cmd[3] == '>') || (cmd[3] == '<') || (cmd[3] == '!')){
				tmp = get_script_val(cmd+4);
				if(tmp > 255) break;
				switch(cmd[3]){
				case '=': if(param.var[n] == tmp) goto do_loop_cmd;
						  break;
				case '>': if(param.var[n] > tmp) goto do_loop_cmd;
						  break;
				case '<': if(param.var[n] < tmp) goto do_loop_cmd;
						  break;
				case '!': if(param.var[n] != tmp) goto do_loop_cmd;
						  break;
				}
				goto do_skip_loop;
			}
			// цикл повторяется, если переменная не равна нулю
			if(param.var[n] != 0){
				// автодекремент переменной
				param.var[n]--;
	do_loop_cmd:
				// переход на начало цикла
				status = script_seek(&script, popstack());
				cmd[0] = 0; // !!!! магия!!!
				if(status == SCRIPT_OK) pushstack(script.pos);
			} else {
				// если переменная цикла обнулена - извлечем из стека без перехода,
				// чтобы не засорять стек
	do_skip_loop:
				popstack();
			}

			break;
		}
		// проверка на команду без параметров
		if(strncmp_P(cmd, PSTR("END"), 3) == 0){
			// конец скрипта
			cmd[0] = 0;
			status = SCRIPT_ERROR_IO;
			break;
		} else if(strncmp_P(cmd, PSTR("CLR"), 3) == 0){
			// очистка
			memset(pixels, 0, PIXEL_CNT * sizeof(pixel_t));
			memset(&param, 0, sizeof(script_param_t));
			param.bright = 0xFF;

		} else if(strncmp_P(cmd, PSTR("INF"), 3) == 0){
			// бесконечный цикл
			goto do_loop_cmd;
			//status = script_seek(&script, popstack());
			//cmd[0] = 0; // !!!! магия!!!
			//if(status == SCRIPT_OK) pushstack(script.pos);

		} else if(strncmp_P(cmd, PSTR("RST"), 3) == 0){
			// рестарт
			status = script_seek(&script, 0);
			cmd[0] = 0; // !!!! магия!!!

		} else if(strncmp_P(cmd, PSTR("RPT"), 3) == 0){
			// начало цикла
			// магия указателей !!!!
			uint32_t next = script.pos - (script.readed - (cmd - buf + 3));
			pushstack(next);
			//pushstack(script.pos);

		} else if(strncmp_P(cmd, PSTR("PNT"), 3) == 0){
			// отрисовка
			return 1;
		}else if(strncmp_P(cmd, PSTR("REV"), 3) == 0){
			// реверс
			param.rev = !param.rev;

		} else if(strncmp_P(cmd, PSTR("NEG"), 3) == 0){
			// инверсия яркости
			for(uint8_t i=0; i<PIXEL_CNT; i++)
				pixels[i].bright ^= 0xFF;
		}
	} while(0);

	return 0;
}

/**
 * Старт модуля
 */
static void _start(void){
	uint16_t buf_sz;
	// ставим блокировку
	lock_input = 1;
	// пикселы гасим
	off_all_pixels();
	// запрашиваем память под кэш
	buf_sz = SD_BUF_SZ;
	buf = get_reserved_memory(&buf_sz);
#if defined(_DEBUG_)
	if(buf_sz != SD_BUF_SZ){
		status = DBG_ERROR;
		return;
	}
#endif
	// пробуем открыть карту
	status = sdcard_open();
	if(status == SCRIPT_OK){
		// считаем скрипты на карте
		total_script_files = get_script_count();
		script_id = 0;
		if(total_script_files > 0){
			// пробуем открыть первый скрипт
			status = script_open(&script, script_id);
		} else
			status = SCRIPT_NOTFOUND;
	}
}

/**
 * Пропуск команды в строке
 * @param s указатель на команду в строке
 * @return указатель на первый после команды символ-разделитель
 */
static char* skip_cmd(char *s){
	uint8_t n = 0;
	// находим первый не значащий символ
	while(s[n] && (s[n] != ' ') && (s[n] != '\n') && (s[n] != '\r') && (s[n] != '\t')) n++;
	return s + n;
}

/**
 * Пропуск всех символов разделителей в строке
 * @param s указатель на строку
 * @return указатель на первый символ очередной команды или NULL, если команд больше
 * нет
 */
static char* get_cmd(char *s){
	uint8_t n = 0;
	// находим первый значащий символ
	while(s[n] && ((s[n] == ' ') || (s[n] == '\n') || (s[n] == '\r') || (s[n] == '\t'))) n++;
	return s[n] ? s + n : NULL;
}

/**
 * Разбор кэша и исполнение команд из него
 * @param buf адрес буфера кэша
 * @return длительность задержки, которую надо отработать перед продолжением
 */
uint8_t script_execute(char *buf){
	uint8_t inf = 0;
	uint8_t result;

	do{
		if(!++inf){
			// если прошло больше 255 итераций цикла - это ошибка в скрипте
			status = SCRIPT_INFLOOP;
			break;
		}
		if(script.cmd == NULL){
			// если текущая команда отсутствует, надо заполнить кэш
			status = script_read_str(&script, buf);
			// команда будет с начала кэша
			script.cmd = get_cmd(buf);
			continue;
		}
		// исполняем текущую команду
		result = execute_cmd(script.cmd);
		// и берем следующую
		script.cmd = get_cmd(skip_cmd(script.cmd));
		// если нужно - выход для выполнения задержки
		if(result) return result;
	}while(status == SCRIPT_OK);

	return 0;
}

/**
 * Активация исполнения следующего файла скрипта
 * @param d направление изменения номера файла +1 или -1
 * @return всегда PRESET_HIDDEN для совместимости
 */
static preset_result_t next_file(int8_t d){
	select_next = d;
	return PRESET_NOTHING;
}

/// длительность индикации состояния модуля в интервалах по 10 мс
#define IND_SD_STATUS		50

/**
 * Исполнение скрипта в виде главной функции модуля фонового эффекта
 * @param s указатель на параметры звукового сигнала (игнорируется)
 */
void script_exec(signal_t *s){
	static uint8_t div;

	if(select_next != 0){
		div = 0;
		enter_sd_mode();
		goto next_file_load;
	}

	if(div){
		// отрабатывание задержки в скрипте
		if(--div)	return;
	}

	enter_sd_mode();

	switch(status){
	case SCRIPT_OK:
		// скрипт открыт успешно
		div = script_execute(buf);
		break;
	case SCRIPT_NO_SD:
		div = IND_SD_STATUS;
		_start();
		break;
	case SCRIPT_NOTFOUND:
		div = IND_SD_STATUS;
		_start();
		break;
	case SCRIPT_INFLOOP:
		// бесконечный цикл в скрипте путем имитации ошибки чтения файла
		// вызывает запуск следующего скрипта
		div = IND_SD_STATUS;
		status = SCRIPT_ERROR_IO;
		break;
	case SCRIPT_ERROR_IO:
		// ошибка при чтении скрипта - в нормальном случае возникает после
		// считывания всех строк файла, иначе - сбой карты
		if(select_next == 0){
			select_next = 1;
		}
	next_file_load:

		if(total_script_files){
			if(select_next < 0){
				if(script_id == 0)
					script_id = total_script_files-1;
				else
					script_id--;
			} else {
				script_id += select_next;
				if(script_id >= total_script_files) script_id = 0;
			}
		}
		status = script_open(&script, script_id);
		select_next = 0;

	default:
		break;
	}

	leave_sd_mode();
}

/**
 * Завершение работы модуля
 */
static void _stop(void){
	// снимаем блокировку
	lock_input = 0;
}

/// период анимации при воспроизведении скрипта в интервалах по 10 мс
#define ANIMATE_DELAY		50
#define ANIMATE_LEN			3

/**
 * Вывод информации о состоянии модуля
 * @param show если не ноль, то выводим
 * @return всегда INFO_ALWAYS
 */
static effect_info_t _info(uint8_t show){
	static uint8_t ind_pos;
	static uint8_t div = ANIMATE_DELAY;
	if(show){
		char str[17];
		switch(status){
		case SCRIPT_NO_SD:
			center_str_p(1, PSTR("NO CARD"));
			break;
		case SCRIPT_NOTFOUND:
			center_str_p(1, PSTR("NO SCRIPT"));
			break;
		case SCRIPT_ERROR_IO:
			center_str_p(1, PSTR("SEARCH..."));
			break;
		case SCRIPT_OK:
			// анимация перед имененм файла
			for(uint8_t i=0; i<ANIMATE_LEN; i++){
				str[i] = i == ind_pos ? '>' : '-';
			}
			if(!--div){
				div = ANIMATE_DELAY;
				if(++ind_pos >= ANIMATE_LEN) ind_pos = 0;
			}
			str[ANIMATE_LEN] = ' ';
			str[ANIMATE_LEN+1] = 0;
			// имя файла
			strncat(str, script.filename, 11);
			show_rpad_str(1, str);
			break;
		case SCRIPT_INFLOOP:
			center_str_p(1, PSTR("INF.LOOP"));
			break;
#if defined(_DEBUG_)
		case DBG_ERROR:
			center_str_p(1, PSTR("<NO MEMORY>"));
			break;
#endif
		}
	}
	return INFO_ALWAYS;
}

/// структура описания модуля
static flash_effect_t effect_def = {
	.name = _name,
	.start = _start,
	.work = script_exec,
	.stop = _stop,
	.preset = next_file,
	.save = NULL,
	.info = _info
};

INIT(7){
	// регистрация эффекта
	register_effect(BACKGROUND_EFFECT, &effect_def);
	// так же пин управления блокировкой WS2812b
	DDR(WS_LOCK_PORT) |= WS_LOCK_PIN;

	leave_sd_mode();
}
/**
 * @}
 */

#endif
