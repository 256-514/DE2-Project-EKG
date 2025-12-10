#ifndef PPG_SENSOR_H
#define PPG_SENSOR_H

#include <inttypes.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>

#define PPG_ADC_CHANNEL 5 

typedef uint8_t BPM_t;

void PPG_init(void);

uint16_t PPG_read_sensor_value(void);

BPM_t PPG_calculate_bpm(void);

#endif