#include "bpm.h"
#include <stdint.h>

#define BPM_AVG_SIZE 10

extern uint32_t millis(void);

static uint32_t last_beat_time = 0;
static uint16_t last_bpm = 0;

static uint16_t avg_dc = 512;
static uint16_t threshold = 550;

static uint8_t triggered = 0;
static uint8_t heart_state = 0;

static uint16_t bpm_buf[BPM_AVG_SIZE];
static uint8_t bpm_idx = 0;
static uint8_t bpm_filled = 0;

void bpm_init(void)
{
    last_beat_time = 0;
    last_bpm = 0;
    avg_dc = 512;
    threshold = 550;
    triggered = 0;
    heart_state = 0;

    for(uint8_t i=0;i<BPM_AVG_SIZE;i++)
        bpm_buf[i] = 0;

    bpm_idx = 0;
    bpm_filled = 0;
}

uint16_t bpm_process_sample(uint16_t sample)
{
    uint32_t now = millis();
    uint32_t delta = now - last_beat_time;

    if(sample > avg_dc) avg_dc++;
    else avg_dc--;

    threshold = avg_dc + 45;

    uint16_t bpm_out = 0;

    if(sample > threshold && !triggered)
    {
        if(delta > 450)
        {
            if(delta < 2000)
            {
                uint16_t new_bpm = (uint16_t)(60000UL / delta);

                bpm_buf[bpm_idx] = new_bpm;
                bpm_idx = (bpm_idx + 1) % BPM_AVG_SIZE;

                if(bpm_filled < BPM_AVG_SIZE) bpm_filled++;

                uint32_t sum = 0;
                for(uint8_t i=0;i<bpm_filled;i++)
                    sum += bpm_buf[i];

                last_bpm = sum / bpm_filled;
                bpm_out  = last_bpm;

                heart_state = !heart_state;
            }

            last_beat_time = now;
            triggered = 1;
        }
    }
    else if(sample < (threshold - 20) && triggered)
    {
        triggered = 0;
    }

}

uint16_t bpm_get_last_bpm(void)
{
    return last_bpm;
}

uint8_t bpm_get_heart_state(void)
{
    return heart_state;
}
