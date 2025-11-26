#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "twi.h"
#include "oled.h"
#include "ecg_loader.h"
#include "wfdb_parser.h"

#include "dataset_14030_lr.h"     // ← toto stačí, nic extern už nedeklarovat

#define SCREEN_W 128
#define SCREEN_H 64
#define BASELINE_Y 32
#define SCALE 0.05f
#define SPEED_MS 5

uint8_t x_pos = 0;

static uint8_t ecg_to_y(int16_t s)
{
    float v = s * SCALE;
    int16_t y = BASELINE_Y - (int16_t)v;
    if (y < 0) y = 0;
    if (y >= SCREEN_H) y = SCREEN_H - 1;
    return (uint8_t)y;
}

int main(void)
{
    uart_init(UART_BAUD_SELECT(115200, F_CPU));
    twi_init();
    oled_init(0);       // init OK
    oled_sleep(0);      // zapne display
    oled_clear_buffer();
    oled_display();
    sei();

    ecg_file_t ecg;

    // ✔ správné jméno datasetu + headeru
    ecg_open(&ecg, dataset_14030_lr_dat, dataset_14030_lr_hea);

    int16_t sample;
    char buf[16];

    while (1)
    {
        if (ecg_read_sample(&ecg, &sample) != 0)
        {
            ecg.pos = 0;
            continue;
        }

        itoa(sample, buf, 10);
        uart_puts(buf);
        uart_puts("\r\n");

        uint8_t y = ecg_to_y(sample);

        oled_drawLine(x_pos, 0, x_pos, SCREEN_H - 1, BLACK);
        oled_drawPixel(x_pos, y, WHITE);

        x_pos++;
        if (x_pos >= SCREEN_W)
        {
            x_pos = 0;
            oled_clear_buffer();
        }

        oled_display();
        _delay_ms(SPEED_MS);
    }
}
