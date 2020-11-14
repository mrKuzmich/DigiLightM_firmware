#define F_CPU 8000000UL

#define	buff_size 10

#include <stdlib.h>
#include <avr/io.h>
#include "diskio.h"
#include "pff.h"
#include "spi.h"

FATFS fs;//объявляем объект типа FATFS
BYTE read_buff[buff_size];//буфер для чтения файла
BYTE write_buff[buff_size] = "hello word";//буфер для записи в файл
UINT br; //счетчик прочитанных байт
	
int main(void)
{

	//монтируем диск
	if (pf_mount(&fs) == FR_OK )
	{
		//открываем файл лежащий в папке new
		if(pf_open("new/hello.txt") == FR_OK)
		 {
			//устанавливаем указатель
			pf_lseek(0);
			
			//записываем
			pf_write(write_buff, buff_size, &br);  

			//финализируем запись 	
			pf_write(0, 0, &br);  

			//устанавливаем указатель 
			pf_lseek(0);
			
			//читаем то что записали
			pf_read(read_buff, buff_size, &br);

			if(br != buff_size)
			 {
				//если br не равно buff_size
				 //значит мы достигли конца файла
			 }  
		 }
		 //демонтируем диск 
		 pf_mount(NULL);
	}
	
    while(1)
    {
        
    }
}
