#include "pulse.h"
#include <avr/io.h>

#define ADC_CHANNEL 0        // A0
#define FILTER_N     8       // moving avg length

static uint16_t buf[FILTER_N];
static uint8_t idx = 0;
static uint8_t count = 0;

/* peak detection state */
static uint16_t last_sample = 0;
static uint8_t peak_detected = 0;
static uint32_t last_peak_time = 0;

/*-------------------- ADC INIT ----------------------*/
void pulse_init(void)
{
    ADMUX = (1 << REFS0) | ADC_CHANNEL;        // AVcc ref, ADC0
    ADCSRA = (1 << ADEN) | (1 << ADPS2) |
             (1 << ADPS1) | (1 << ADPS0);      // enable ADC, prescaler 128
}

/*-------------------- RAW READ ----------------------*/
uint16_t pulse_read(void)
{
    ADCSRA |= (1 << ADSC);           // start
    while (ADCSRA & (1 << ADSC));    // wait
    return ADC;
}

/*-------------------- MOVING AVERAGE ----------------*/
uint16_t pulse_read_filtered(void)
{
    uint16_t s = pulse_read();

    buf[idx++] = s;
    if (idx >= FILTER_N) idx = 0;

    if (count < FILTER_N) count++;

    uint32_t sum = 0;
    for (uint8_t i = 0; i < count; i++)
        sum += buf[i];

    return (uint16_t)(sum / count);
}

/*-------------------- PEAK DETECTION → BPM -----------*/
/*
   Ultrajednodušená detekce:
   - detekuje náběžné hrany
   - měří čas mezi dvěma špičkami (ms)
   - vrací BPM (beats per minute)
*/
uint16_t pulse_process_for_bpm(uint16_t sample)
{
    static uint16_t threshold = 520;     // budeš ladit
    static uint16_t hysteresis = 20;

    uint16_t bpm = 0;

    if (!peak_detected && sample > threshold && last_sample <= threshold)
    {
        // rising edge detected
        uint32_t now = millis();         // PlatformIO Arduino framework

        if (last_peak_time != 0)
        {
            uint32_t dt = now - last_peak_time;  // milliseconds
            if (dt > 250 && dt < 2000)           // 30–240 BPM valid range
            {
                bpm = (uint16_t)(60000UL / dt);
            }
        }

        last_peak_time = now;
        peak_detected = 1;
    }

    if (peak_detected && sample < (threshold - hysteresis))
    {
        peak_detected = 0;               // reset latch
    }

    last_sample = sample;
    return bpm;
}
