#include <avr/io.h>
#include "spi.h"
//#include "../avr_helper.h"
//#include "../hardware.h"

/*инициализаци€ SPI*/
//void SPI_Init(void)
//{
//  /*настройка портов ввода-вывода
//  все выводы, кроме MISO выходы*/
//  SPI_DDRX |= (1<<SPI_MOSI)|(1<<SPI_SCK)|(1<<SPI_SS);
//  SPI_DDRX &= ~(1<<SPI_MISO);
//
//  SPI_PORTX |= (1<<SPI_MOSI)|(1<<SPI_SCK)|(1<<SPI_SS)|(1<<SPI_MISO);
//
//  /*разрешение spi,старший бит вперед,мастер, пол€рность синхронизации 0*/
//  SPCR = (0<<SPIE)|(1<<SPE)|(0<<DORD)|(1<<MSTR)|(0<<CPOL)|(0<<CPHA)|(0<<SPR1)|(0<<SPR0);
//  SPSR = (1<<SPI2X);
//	DDRB|= 1<<SPI_MOSI| // выход
//		   0<<SPI_MISO| // ¬’ќƒ
//		   1<<SPI_SCK | // выход
//		   1<<SPI_SS;   // вход
//	DDRC|= 1<<SPI_SS;   // вход

//	PORTB|= 1<<SPI_MOSI |
//			1<<SPI_MISO | // вкл. подт€гивающий резистор
//			1<<SPI_SCK  |
//	        1<<SPI_SS;
//	PORTC|= 1<<SPI_SS;
//--SPCR Ц SPI Control Register
//	SPCR|=1<<SPE | // SPI Enable
//		  0<<DORD| // Data Order (when the DORD bit is written to zero, the MSB of the data word is transmitted first)
//		  1<<MSTR| // Master/Slave Select (1 for Master)
//		  0<<CPOL| // Clock Polarity
//		  0<<CPHA| // Clock Phase
//		  1<<SPR1| // SPI Clock Rate (This bit control the SCK rate of the device configured as a Master.)
//		  1<<SPR0; // SPI Clock Rate (This bit control the SCK rate of the device configured as a Master.)
//--SPSR Ц SPI Status Register
//	SPSR|=0<<SPI2X; // Double SPI Speed Bit (стартуем на нормальной скорости)
//}

/*отослать байт данных по SPI*/
void SPI_WriteByte(uint8_t data)
{
   SPDR = data; 
   while(!(SPSR & (1<<SPIF)));
}

/*получить байт данных по SPI*/
uint8_t SPI_ReadByte(void)
{  
   SPDR = 0xff;
   while(!(SPSR & (1<<SPIF)));
   return SPDR; 
}

/*отослать и получить байт данных по SPI*/
uint8_t SPI_WriteReadByte(uint8_t data)
{  
   SPDR = data;
   while(!(SPSR & (1<<SPIF)));
   return SPDR; 
}

/*отправить несколько байт данных по SPI*/
void SPI_WriteArray(uint8_t count, uint8_t *data)
{
   while(count--){
      SPDR = *data++;
      while(!(SPSR & (1<<SPIF)));
   }
}

/*отправить и получить несколько байт данных по SPI*/
void SPI_WriteReadArray(uint8_t count, uint8_t *data)
{
   while(count--){
      SPDR = *data;
      while(!(SPSR & (1<<SPIF)));
      *data++ = SPDR; 
   }
}
