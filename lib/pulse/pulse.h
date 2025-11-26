#ifndef PULSE_H
#define PULSE_H

#include <stdint.h>

/* Initialize ADC for the HW-827 pulse sensor */
void pulse_init(void);

/* Read raw ADC value (0–1023) */
uint16_t pulse_read(void);

/* Return filtered PPG value (moving average) */
uint16_t pulse_read_filtered(void);

/* Process one sample and return BPM if detected, otherwise 0 */
uint16_t pulse_process_for_bpm(uint16_t sample);

#endif
