
#ifndef SPI_H
#define SPI_H

//#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "../avr_helper.h"
#include "../hardware.h"

/*
#define SPI_PORTX   PORTB
#define SPI_DDRX    DDRB

#define SPI_MISO   4
#define SPI_MOSI   3
#define SPI_SCK    5
#define SPI_SS     2

*/
/*_______________ макросы ____________________*/


/*запретить SPI устройство*/
#define SPI_DisableSS_m(ss)  do{PORT(PORT_SPI) |= (1<<(PIN_SS)); }while(0)

/*разрешить SPI устройство*/
#define SPI_EnableSS_m(ss)   do{PORT(PORT_SPI) &= ~(1<<(PIN_SS)); }while(0)

/*статус SPI устройства*/
#define SPI_StatSS_m(ss)    (!(PORT(PORT_SPI) & (1<<(PIN_SS))))

/*передать байт данных по SPI*/
#define SPI_WriteByte_m(data)  do{ SPDR = data; while(!(SPSR & (1<<SPIF))); }while(0)

/*прочитать байт данных по SPI*/
#define SPI_ReadByte_m(data)  do{ SPDR = 0xff; while(!(SPSR & (1<<SPIF))); data = SPDR;}while(0)


/* ______________ встраиваемые функции _____________*/


/*получить байт данных по SPI*/
inline static uint8_t SPI_ReadByte_i(void){
   SPDR = 0xff;
   while(!(SPSR & (1<<SPIF)));
   return SPDR;   
}

/*  _________________ функции ________________ */

/*инициализаци€ SPI модул€*/
void SPI_Init(void); 

/*отправить байт данных по SPI*/
void SPI_WriteByte(uint8_t data); 

/*получить байт данных по SPI*/
uint8_t SPI_ReadByte(void);

/*отправить и получить байт данных по SPI*/
uint8_t SPI_WriteReadByte(uint8_t data);

/*отправить несколько байт данных по SPI*/
void SPI_WriteArray(uint8_t count, uint8_t *data);

/*отправить и получить несколько байт данных по SPI*/
void SPI_WriteReadArray(uint8_t count, uint8_t *data);


#endif //SPI_H
