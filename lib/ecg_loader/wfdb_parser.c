#include "wfdb_parser.h"
#include <avr/pgmspace.h>

void wfdb_parse_header(const char *hea,
                       uint32_t *nsamp,
                       float *gain,
                       float *baseline,
                       uint8_t *fmt)
{
    char c;
    uint16_t idx = 0;

    uint32_t number = 0;
    uint8_t stage = 0;

    while ((c = pgm_read_byte(&hea[idx])) != 0)
    {
        if (c >= '0' && c <= '9') {
            number = number * 10 + (c - '0');
        }
        else if (c == ' ') {
            if (stage == 2) { *nsamp = number; break; }
            stage++;
            number = 0;
        }
        idx++;
    }

    *gain = 200.0f;
    *baseline = 0.0f;
    *fmt = 212;
}
