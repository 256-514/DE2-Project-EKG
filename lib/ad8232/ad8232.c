#include "ad8232.h"

void ad8232_init(void)
{
    DDRD &= ~((1 << AD8232_LO_PLUS_PIN) | (1 << AD8232_LO_MINUS_PIN));

    ADMUX = (1 << REFS0);

    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) | (1 << ADPS1);
}

uint8_t ad8232_leads_off(void)
{
    uint8_t lo_plus  = (PIND & (1 << AD8232_LO_PLUS_PIN));
    uint8_t lo_minus = (PIND & (1 << AD8232_LO_MINUS_PIN));
    return (lo_plus || lo_minus);
}

uint16_t ad8232_read(void)
{
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}
