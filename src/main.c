#include <avr/io.h>         // AVR device-specific IO definitions
#include <util/delay.h>
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <twi.h>            // I2C/TWI library for AVR-GCC
#include <oled.h>
#include <uart.h>           // Peter Fleury's UART library
#include <stdio.h>          // C library. Needed for `sprintf`
#include "timer.h"
#include "sine_lut.h"

#define W 128
#define H 64
#define PERIOD 64

void scroll_left(void)
{
   for (uint8_t y = 0; y < H; y++)
   {
       for (uint8_t x = 0; x < W - 1; x++)
       {
           uint8_t pix = oled_check_buffer(x + 1, y);
           oled_drawPixel(x, y, pix);
       }
       oled_drawPixel(W - 1, y, 0);  // clear last column
   }
}

int main(void)
{
   oled_init(OLED_DISP_ON);
   oled_display();

   uint8_t idx = 0;

   while (1)
   {
       scroll_left();

       uint8_t y = sine_lut[idx];
       idx = (idx + 1) % PERIOD;

       oled_drawPixel(W - 1, H - 1 - y, 1);

       // update by pages (blocks) for NO FLICKERING
       for (uint8_t page = 0; page < H/8; page++)
       {
           oled_display_block(0, page, W);
       }

       _delay_ms(16);
   }
}