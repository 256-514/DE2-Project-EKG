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
    static uint16_t prev_sample = 0;
    static float avg_ibi = 0.0f;
    const float ALPHA = 0.25f;
    uint32_t now = millis();
    uint32_t dt = now - last_beat_time;

    if (sample > avg_dc) avg_dc++; else if (sample < avg_dc) avg_dc--;
    threshold = avg_dc + 45;

    uint16_t out = 0;

    if (sample > threshold && !triggered && (sample > prev_sample) && (sample - prev_sample) > 18)
    {
        if (dt > 300 && dt < 2000)
        {
            uint16_t inst_bpm = (uint16_t)(60000UL / dt);

            if (avg_ibi == 0.0f) avg_ibi = dt;
            else avg_ibi = 0.2f * dt + 0.8f * avg_ibi;

            last_bpm = (uint16_t)(60000.0f / avg_ibi + 0.5f);

            last_bpm = (uint16_t)(ALPHA * inst_bpm + (1.0f-ALPHA) * last_bpm + 0.5f);

            out = last_bpm;
            heart_state = !heart_state;
        }
        last_beat_time = now;
        triggered = 1;
    }
    else if (sample < (threshold - 15) && triggered)
    {
        triggered = 0;
    }

    prev_sample = sample;
    return out;
}

uint16_t bpm_get_last_bpm(void)
{
    return last_bpm;
}

uint8_t bpm_get_heart_state(void)
{
    return heart_state;
}
