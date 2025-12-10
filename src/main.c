#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include "twi.h"
#include "oled.h"
#include "ad8232.h"
#include "bpm.h"
#include "ppg_Sensor.h"
#include <string.h>

volatile uint32_t timer0_millis = 0;
ISR(TIMER0_OVF_vect){ timer0_millis++; TCNT0 = 6; }

uint32_t millis(void){
    uint32_t t; cli(); t = timer0_millis; sei(); return t;
}
void timer_init(void){
    TCCR0B |= (1<<CS01)|(1<<CS00);
    TIMSK0 |= (1<<TOIE0);
}

#define LO_PLUS_PIN   PD3
#define LO_MINUS_PIN  PD2
#define BTN_PIN       PD6

#define WLEN 96
uint8_t ekg_wave[WLEN];
uint8_t ppg_wave[WLEN];
uint8_t ekg_idx = 0;
uint8_t ppg_idx = 0;

static uint16_t last_bpm = 0;

static uint8_t ppg_heart_state = 0;

static inline uint8_t map_raw_to_y(uint16_t v){
    uint32_t y = (uint32_t)v * 63 / 1023;
    return (uint8_t)(63 - y);
}

void show_leads_off(void){
    oled_clear_buffer();
    oled_charMode(NORMALSIZE);

    oled_goto_xpix_y(49, 22);
    oled_puts("PRIPOJTE");

    oled_goto_xpix_y(55, 38);
    oled_puts(" ELEKTRODY!");

    oled_display();
}

void draw_frame(uint8_t *buf, uint8_t idx, uint16_t bpm, const char *label, uint8_t heart_on)
{
    oled_clear_buffer();

    const uint8_t HEADER_H = 14;
    const uint8_t WAVE_H   = 50;
    const uint8_t Y_OFFSET = HEADER_H;
    const uint8_t X_SCALE  = 3;

    for (uint8_t i = 0; i < WLEN - 1; i++)
    {
        uint8_t raw_y1 = buf[(idx + i) % WLEN];
        uint8_t raw_y2 = buf[(idx + i + 1) % WLEN];

        uint8_t scaled_y1 = (uint16_t)raw_y1 * (WAVE_H - 1) / 63;
        uint8_t scaled_y2 = (uint16_t)raw_y2 * (WAVE_H - 1) / 63;

        uint8_t y1 = Y_OFFSET + scaled_y1;
        uint8_t y2 = Y_OFFSET + scaled_y2;

        uint16_t x1 = i * X_SCALE;
        uint16_t x2 = (i + 1) * X_SCALE;

        if (x2 >= 128) break;

        oled_drawLine(x1, y1, x2, y2, WHITE);
    }

    oled_charMode(NORMALSIZE);

    oled_goto_xpix_y(0, 0);
    oled_puts("BPM:");

    char bbuf[8];
    if (bpm > 0) sprintf(bbuf, "%u", bpm);
    else sprintf(bbuf, "--");

    oled_goto_xpix_y(30, 0);
    oled_puts(bbuf);

    uint8_t hs = heart_on;
    uint8_t heart_x = 30 + (strlen(bbuf) * 6) + 6; 
    uint8_t heart_y = 0;

    if (hs)
    {
        oled_drawPixel(heart_x+1, heart_y+1, WHITE);
        oled_drawPixel(heart_x+2, heart_y+1, WHITE);
        oled_drawPixel(heart_x+4, heart_y+1, WHITE);
        oled_drawPixel(heart_x+5, heart_y+1, WHITE);

        oled_drawPixel(heart_x,   heart_y+2, WHITE);
        oled_drawPixel(heart_x+3, heart_y+2, WHITE);
        oled_drawPixel(heart_x+6, heart_y+2, WHITE);

        oled_drawPixel(heart_x,   heart_y+3, WHITE);
        oled_drawPixel(heart_x+6, heart_y+3, WHITE);

        oled_drawPixel(heart_x+1, heart_y+4, WHITE);
        oled_drawPixel(heart_x+5, heart_y+4, WHITE);

        oled_drawPixel(heart_x+2, heart_y+5, WHITE);
        oled_drawPixel(heart_x+4, heart_y+5, WHITE);

        oled_drawPixel(heart_x+3, heart_y+6, WHITE);
    }
    else
    {
        for (uint8_t yy = 0; yy < 8; yy++)
            for (uint8_t xx = 0; xx < 8; xx++)
                oled_drawPixel(heart_x + xx, heart_y + yy, BLACK);
    }

    oled_goto_xpix_y(107, 0);
    oled_puts(label);

    oled_display();
}

int main(void)
{
    for(uint8_t i=0;i<WLEN;i++){
        ekg_wave[i] = 32;
        ppg_wave[i] = 32;
    }

    twi_init();
    oled_init(OLED_DISP_ON);
    oled_clrscr();

    ad8232_init();
    bpm_init();
    PPG_init();
    timer_init();
    sei();

    DDRD &= ~((1<<LO_PLUS_PIN)|(1<<LO_MINUS_PIN));
    PORTD |=  (1<<LO_PLUS_PIN)|(1<<LO_MINUS_PIN);

    DDRD &= ~(1<<BTN_PIN);
    PORTD |=  (1<<BTN_PIN);

    uint32_t last = 0;
    uint8_t mode = 0;

    while(1)
    {
        uint32_t now = millis();

        static uint8_t last_btn = 1;
        uint8_t btn = (PIND & (1<<BTN_PIN)) ? 1 : 0;

        if(last_btn==1 && btn==0){
            _delay_ms(20);
            if(!(PIND&(1<<BTN_PIN))){
                mode = (mode==0)?1:0;

                for(uint8_t i=0;i<WLEN;i++){
                    ekg_wave[i] = 32;
                    ppg_wave[i] = 32;
                }
                ekg_idx = 0;
                ppg_idx = 0;
            }
            while(!(PIND&(1<<BTN_PIN))) _delay_ms(10);
        }
        last_btn = btn;

        if(mode == 0)
        {
            uint8_t lo_plus  = (PIND & (1<<LO_PLUS_PIN))  ? 1 : 0;
            uint8_t lo_minus = (PIND & (1<<LO_MINUS_PIN)) ? 1 : 0;

            if(lo_plus || lo_minus){
                show_leads_off();
                continue;
            }

            ADMUX = (1<<REFS0) | 0;
            uint16_t raw = ad8232_read();

            uint16_t bpm_now = bpm_process_sample(raw);
            if(bpm_now > 0) last_bpm = bpm_now;

            uint8_t y0 = map_raw_to_y(raw);

            static int16_t ekg_baseline = 32;
            ekg_baseline = (ekg_baseline * 14 + y0) / 15;

            int16_t diff = y0 - ekg_baseline;

            diff = (diff * 3) / 2;

            int16_t y = ekg_baseline + diff;

            if (y < 20) y = 20;
            if (y > 60) y = 60;

            ekg_wave[ekg_idx] = (uint8_t)y;
            ekg_idx = (ekg_idx + 1) % WLEN;

            if(now - last > 16){
                last = now;
                draw_frame(ekg_wave, ekg_idx, last_bpm, "EKG", bpm_get_heart_state());
            }
        }
        else   // PPG MODE
        {
            /* přepnout ADC na A1 */
            ADMUX = (1<<REFS0) | PPG_ADC_CHANNEL;

            uint16_t raw = PPG_read_sensor_value();
            uint16_t bpm_now = PPG_calculate_bpm();

            if (bpm_now > 0) {
                last_bpm = bpm_now;
                ppg_heart_state = !ppg_heart_state;   // <<< TOTO PŘIDAT
            }

            /* správné mapování jako u EKG */
            uint8_t y0 = map_raw_to_y(raw);

            /* jemný zoom, aby byl PPG vidět */
            int16_t z = y0 - 32;
            z = z * 2;
            int16_t y = 32 + z;

            if(y < 0) y = 0;
            if(y > 63) y = 63;

            ppg_wave[ppg_idx] = (uint8_t)y;
            ppg_idx = (ppg_idx + 1) % WLEN;

            if(now - last > 16){
                last = now;
                draw_frame(ppg_wave, ppg_idx, last_bpm, "PPG", ppg_heart_state);
            }
        }
    }
}
