#ifndef AD8232_H
#define AD8232_H

#include <stdint.h>
#include <avr/io.h>

#define AD8232_LO_PLUS_PIN   PD3
#define AD8232_LO_MINUS_PIN  PD2

void ad8232_init(void);
uint8_t ad8232_leads_off(void);
uint16_t ad8232_read(void);

#endif
