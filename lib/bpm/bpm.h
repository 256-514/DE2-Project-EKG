#ifndef BPM_H
#define BPM_H

#include <stdint.h>

void bpm_init(void);

uint16_t bpm_process_sample(uint16_t sample);

uint16_t bpm_get_last_bpm(void);

uint8_t bpm_get_heart_state(void);

#endif
