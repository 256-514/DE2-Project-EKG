#ifndef ECG_LOADER_H
#define ECG_LOADER_H

#include <stdint.h>
#include <avr/pgmspace.h>

typedef struct {
    const uint8_t *dat;
    const char *hea;
    uint32_t nsamp;
    float gain;
    float baseline;
    uint8_t fmt;
    uint32_t pos;
} ecg_file_t;

void ecg_open(ecg_file_t *f, const uint8_t *dat, const char *hea);
uint8_t ecg_read_sample(ecg_file_t *f, int16_t *sample);

#endif
