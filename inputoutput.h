#ifndef INPUTOUTPUT_H
#define INPUTOUTPUT_H

#include <stdint.h>


void init_timers();
void init_io();
uint8_t get_switches();
uint8_t get_buttons();
uint8_t get_io();
void match_leds(uint8_t leds);
void delay_milliseconds(int x);
void reset_clock();
long get_clock();
void inline tick();
void delay(int cyc);


#endif