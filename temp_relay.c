/***������������� ������������ ���������***/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "delay.h"
#include "onewire.h"
#include "ds18x20.h"


char SEGMENT[ ] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

volatile unsigned char segcounter = 0;
volatile int display1 = 0;

// ���������� ���������� �� ������������ �������2
ISR (TIMER2_OVF_vect)
{    
	PORTB = 0x00; //����� ��� �������
	DDRC  = (1 << segcounter); //�������� ��������� ������
	switch (segcounter)
	{     
		case 0:
			PORTB = SEGMENT[display1 % 1000 / 100]; // ����� ������������ ����� �� �������
			break;
		case 1:
			PORTB = SEGMENT[display1 % 100 / 10] | 0x80;
			break;    
		case 2:
			PORTB = SEGMENT[display1 % 10];
			break;
	}
	if ((segcounter++) > 1) segcounter = 0;    
}

unsigned char	nDevices;	// ���������� ��������
unsigned char	owDevicesIDs[MAXDEVICES][8];	// �� ID

unsigned char search_ow_devices(void) // ����� ���� ��������� �� ����
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


/***������� �������***/
int main (void) 
{
	DDRB |= (1 << PD6)|(1 << PD5)|(1 << PD4)|(1 << PD3)|(1 << PD2)|(1 << PD1)|(1 << PD0);
	DDRC |= (1 << PB3)|(1 << PB2)|(1 << PB1)|(1 << PB0);
	PORTB = 0x00;
	PORTC = 0x00;    
	
	DDRD = 0b00000010; PORTD = 0b00000000;
	
	TIMSK |= (1 << TOIE2); // ���������� ���������� �� �������2
	TCCR2 |= (1 << CS21); //������������ �� 8 

	sei(); //��������� ��������� ����������
	
	timerDelayInit();
 
	nDevices = search_ow_devices(); // ���� ��� ����������

	//display1 = nDevices;
	//_delay_ms(1000); 
	
	unsigned char	data[2]; // ���������� ��� �������� �������� � �������� ����� ������
	char readResult;
	unsigned char	themperature[3]; // � ���� ������ ����� �������� �����������
	
	PORTC |= 0b00001000;
	
	while(1)
	{
		
		for (unsigned char i=0; i<nDevices; i++)
		{
			
			switch (owDevicesIDs[i][0])
			{
				case OW_DS18B20_FAMILY_CODE: { // ���� ������ ����������� DS18B20
					
					DS18x20_StartMeasureAddressed(owDevicesIDs[i]); // ��������� ���������
					timerDelayMs(800); // ���� ������� 750 ��, ���� �������������� �����������
					
					readResult = DS18x20_ReadData(owDevicesIDs[i], data);
					
					if (readResult == 1){
						DS18x20_ConvertToThemperature(data, themperature); // ��������������� ����������� � ���������������� ���
						
						display1 = themperature[1]*10 + themperature[2]/10;
					}
					
				} break;
			}
		}
		
		//_delay_ms(1000); // ��������
	}
}
