#include <stdint.h>
#include <pic32mx.h>

long clock;

void init_timers()
{
	
	TMR2 = 0x0;
	PR2 = 0x7A12; 
	T2CON = 0b1000000001110000;


	TMR3 = 0x0;
	PR3 = 0x4E2;
	T3CON = 0b1000000001100000;


	TMR4 = 0x0;
	PR4 = 0x1F4;
	T4CON = 0b1000000001000000;

	
	clock = 0;
	IEC(0) |= 0x10000;
	IPC(4) |= 0x1C;
	IPC(4) |= 0x3;

	enable_interrupt();

	return;
}



void init_io()
{
    TRISECLR = 0xFF; 
    PORTE    = 0x0;
    TRISDSET = 0xF00; 
    TRISFSET = 0x2;   
    TRISDSET = 0xE0;  
    //TRISBSET = 0x4;
}


uint8_t get_switches()
{
    uint8_t sw = PORTD >> 8;
    sw &= 0xf;
    return sw;
}


uint8_t get_buttons()
{
    uint8_t btn1 = PORTF >> 1;
    btn1 &= 0x1;
    uint8_t btns = PORTD >> 4;
    btns &= 0xE;
    btns |= btn1;
    return btns;
}


uint8_t get_io()
{
    return (get_switches() << 4) | get_buttons();
}


void match_leds(uint8_t leds)
{
    PORTE = leds;
}



void delay_milliseconds(int x){
	int counter = 0;
	TMR3 = 0x0;

	while(counter < x)
	{
		if(IFS(0) & 0x1000)
		{
			IFS(0) &= ~0x1000;
			counter++;
		}
	}
	return;
}


void reset_clock()
{
    clock = 0;
    return;
}

long get_clock()
{
    return clock;
}

void inline tick()
{
    clock++;
    return;
}

void delay(int cyc) {
	int i;
	for(i = cyc; i > 0; i--);
}

