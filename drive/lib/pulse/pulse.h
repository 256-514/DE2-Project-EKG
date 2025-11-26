#ifndef PULSE_H
#define PULSE_H

#include <stdint.h>

// Prototyp millis(), která je definována v main.c
uint32_t millis(void);

void pulse_init(void);
uint16_t pulse_read(void);
uint16_t pulse_read_filtered(void);
uint16_t pulse_process_for_bpm(uint16_t sample);

#endif
