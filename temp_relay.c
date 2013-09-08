/***Использование динамической индикации***/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "delay.h"
#include "onewire.h"
#include "ds18x20.h"


char SEGMENT[ ] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

volatile unsigned char segcounter = 0;
volatile int display1 = 0;

// Обработчик прерывания по переполнению таймера2
ISR (TIMER2_OVF_vect)
{    
	PORTD = 0x00; //гасим все разряды
	DDRB  = (1 << segcounter); //выбираем следующий разряд
	switch (segcounter)
	{     
		case 0:
			PORTD = SEGMENT[display1 % 1000 / 100]; // здесь раскладываем число на разряды
			break;
		case 1:
			PORTD = SEGMENT[display1 % 100 / 10] | 0x80;
			break;    
		case 2:
			PORTD = SEGMENT[display1 % 10];
			break;
	}
	if ((segcounter++) > 1) segcounter = 0;    
}

unsigned char	nDevices;	// количество сенсоров
unsigned char	owDevicesIDs[MAXDEVICES][8];	// Их ID

unsigned char search_ow_devices(void) // поиск всех устройств на шине
{ 
	unsigned char	i;
   	unsigned char	id[OW_ROMCODE_SIZE];
   	unsigned char	diff, sensors_count;

	sensors_count = 0;

	for( diff = OW_SEARCH_FIRST; diff != OW_LAST_DEVICE && sensors_count < MAXDEVICES ; )
    { 
		OW_FindROM( &diff, &id[0] );

      	if( diff == OW_PRESENCE_ERR ) break;

      	if( diff == OW_DATA_ERR )	break;

      	for (i=0;i<OW_ROMCODE_SIZE;i++)
         	owDevicesIDs[sensors_count][i] = id[i];
		
		sensors_count++;
    }
	return sensors_count;
}


/***Главная функция***/
int main (void) 
{
	DDRD |= (1 << PD6)|(1 << PD5)|(1 << PD4)|(1 << PD3)|(1 << PD2)|(1 << PD1)|(1 << PD0);
	DDRB |= (1 << PB2)|(1 << PB1)|(1 << PB0);
	PORTD = 0x00;
	PORTB = 0x00;    
	
	DDRC = 0b00000010; PORTC = 0b00000000;
	
	TIMSK |= (1 << TOIE2); // разрешение прерывания по таймеру2
	TCCR2 |= (1 << CS21); //предделитель на 8 

	sei(); //глобально разрешаем прерывания
	
	timerDelayInit();
 
	nDevices = search_ow_devices(); // ищем все устройства

	display1 = nDevices;
	_delay_ms(1000); 
	while(1)
	{
		//display1++; // увеличиваем счет от 0000 до 9999
		
		for (unsigned char i=0; i<nDevices; i++)
		{
			//display1 = owDevicesIDs[i][0];
			
			switch (owDevicesIDs[i][0])
			{
				case OW_DS18B20_FAMILY_CODE: { // если найден термодатчик DS18B20
					//printf("\r"); print_address(owDevicesIDs[i]); // печатаем знак переноса строки, затем - адрес
					//printf(" - Thermometer DS18B20"); // печатаем тип устройства 
					DS18x20_StartMeasureAddressed(owDevicesIDs[i]); // запускаем измерение
					timerDelayMs(800); // ждем минимум 750 мс, пока конвентируется температура
					unsigned char	data[2]; // переменная для хранения старшего и младшего байта данных
					DS18x20_ReadData(owDevicesIDs[i], data); // считываем данные
					unsigned char	themperature[3]; // в этот массив будет записана температура
					DS18x20_ConvertToThemperature(data, themperature); // преобразовываем температуру в человекопонятный вид
					//printf(": %d.%d C", themperature[1],themperature[2]);
					display1 = (int)(themperature[1]);
				} break;
			}
			//_delay_ms(1000); 
		}
		
		//_delay_ms(1000); // задержка
	}
}
