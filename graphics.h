#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>


uint8_t send_byte_spi(uint8_t byte);
void update_display(uint8_t * buffer);
void init_oled();
void init_display();
void display_page(int numBytes, uint8_t * buffer);
void init_all();
void reset_buffer(uint8_t * buffer);


#endif