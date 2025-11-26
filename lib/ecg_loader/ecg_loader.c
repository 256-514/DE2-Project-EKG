#include "ecg_loader.h"
#include "wfdb_parser.h"

void ecg_open(ecg_file_t *f, const uint8_t *dat, const char *hea)
{
    f->dat = dat;
    f->hea = hea;
    f->pos = 0;

    wfdb_parse_header(f->hea, &f->nsamp, &f->gain, &f->baseline, &f->fmt);
}

uint8_t ecg_read_sample(ecg_file_t *f, int16_t *sample)
{
    if (f->fmt != 212) return 1;

    // 3-byte pack
    uint8_t b0 = pgm_read_byte(&f->dat[f->pos]);
    uint8_t b1 = pgm_read_byte(&f->dat[f->pos+1]);
    uint8_t b2 = pgm_read_byte(&f->dat[f->pos+2]);

    int v1 = (b1 & 0x0F) << 8 | b0;
    if (v1 & 0x800) v1 |= 0xF000;

    *sample = (int16_t)v1;

    f->pos += 3;

    return 0;
}
