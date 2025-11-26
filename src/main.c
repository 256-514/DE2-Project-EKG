#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "uart.h"
#include "twi.h"
#include "oled.h"
#include "ecg_loader.h"
#include "wfdb_parser.h"
#include "dataset_14030_lr.h"

/* Display parameters */
#define SCREEN_W       128
#define SCREEN_H       64

#define BASELINE_Y     40
#define SPEED_MS       5
#define SMOOTH_N       4
#define TIME_STRETCH   2

/* Auto-scaling parameters */
#define TARGET_R_PEAK  30      // chceme aby R-špička měla ~30 px
float SCALE = 0.05f;           // počáteční hodnota, pak se automaticky přepíše
uint8_t scale_locked = 0;

/* global drawing state */
uint8_t x_pos = 0;
int16_t smooth_buf[SMOOTH_N];
uint8_t smooth_idx = 0;
uint8_t smooth_count = 0;
uint8_t prev_y = BASELINE_Y;

static uint8_t ecg_to_y(int16_t s)
{
    int16_t y = BASELINE_Y - (int16_t)(s * SCALE);

    if (y < 0) y = 0;
    if (y >= SCREEN_H) y = SCREEN_H - 1;
    return (uint8_t)y;
}

int main(void)
{
    uart_init(UART_BAUD_SELECT(115200, F_CPU));
    twi_init();
    oled_init(0);
    oled_sleep(0);
    oled_clear_buffer();
    oled_display();
    sei();

    ecg_file_t ecg;
    ecg_open(&ecg, dataset_14030_lr_dat, dataset_14030_lr_hea);

    int16_t sample;
    char buf[16];

    int16_t max_abs = 0;
    uint16_t sample_count = 0;

    while (1)
    {
        if (ecg_read_sample(&ecg, &sample) != 0)
        {
            ecg.pos = 0;
            continue;
        }

        /* --- AUTO-SCALE během prvních 200 vzorků --- */
        if (!scale_locked)
        {
            int16_t abs_v = (sample < 0) ? -sample : sample;
            if (abs_v > max_abs) max_abs = abs_v;

            sample_count++;

            if (sample_count > 200) {
                SCALE = (float)TARGET_R_PEAK / (float)max_abs;
                scale_locked = 1;
            }
        }

        /* Moving average */
        smooth_buf[smooth_idx++] = sample;
        if (smooth_idx >= SMOOTH_N) smooth_idx = 0;
        if (smooth_count < SMOOTH_N) smooth_count++;

        long sum = 0;
        for (uint8_t i = 0; i < smooth_count; i++)
            sum += smooth_buf[i];
        int16_t s_avg = (int16_t)(sum / smooth_count);

        uint8_t y = ecg_to_y(s_avg);

        for (uint8_t k = 0; k < TIME_STRETCH; k++)
        {
            uint8_t x_prev = (x_pos == 0) ? (SCREEN_W - 1) : (x_pos - 1);

            oled_drawLine(x_pos, 0, x_pos, SCREEN_H - 1, BLACK);
            oled_drawLine(x_prev, prev_y, x_pos, y, WHITE);

            prev_y = y;

            x_pos++;
            if (x_pos >= SCREEN_W)
            {
                x_pos = 0;
                oled_clear_buffer();
                prev_y = y;
            }

            oled_display();
            _delay_ms(SPEED_MS);
        }
    }
}
