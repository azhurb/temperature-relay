/***������������� ������������ ���������***/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
char SEGMENT[ ] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

volatile unsigned char segcounter = 0;
volatile int display1 = 0;

// ���������� ���������� �� ������������ �������2
ISR (TIMER2_OVF_vect)
{    
	PORTD = 0x00; //����� ��� �������
	DDRB  = (1 << segcounter); //�������� ��������� ������
	switch (segcounter)
	{     
		case 0:
			PORTD = SEGMENT[display1 % 1000 / 100]; // ����� ������������ ����� �� �������
			//PORTD = 0x3F;
			break;
		case 1:
			PORTD = SEGMENT[display1 % 100 / 10] | 0x80;
			//PORTD = 0x06;
			break;    
		case 2:
			PORTD = SEGMENT[display1 % 10];
			//PORTD = 0x5B;
			break;
	}
	if ((segcounter++) > 1) segcounter = 0;    
}

/***������� �������***/
int main (void) 
{
	DDRD |= (1 << PD6)|(1 << PD5)|(1 << PD4)|(1 << PD3)|(1 << PD2)|(1 << PD1)|(1 << PD0);
	DDRB |= (1 << PB2)|(1 << PB1)|(1 << PB0);
	PORTD = 0x00;
	PORTB = 0x00;    
	TIMSK |= (1 << TOIE2); // ���������� ���������� �� �������2
	TCCR2 |= (1 << CS21); //������������ �� 8 

	sei(); //��������� ��������� ����������

	while(1)
	{
		display1++; // ����������� ���� �� 0000 �� 9999
		delay_ms(500); // ��������
	}
}

void delay_ms(uint16_t x)
{
  uint8_t y, z;
  for ( ; x > 0 ; x--){
    for ( y = 0 ; y < 90 ; y++){
      for ( z = 0 ; z < 6 ; z++){
        asm volatile ("nop");
      }
    }
  }
}