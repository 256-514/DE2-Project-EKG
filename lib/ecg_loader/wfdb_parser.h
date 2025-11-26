/**
 * @file wfdb_parser.h
 * @defgroup ecg_wfdb_parser WFDB Header Parser <wfdb_parser.h>
 * @code #include <wfdb_parser.h> @endcode
 *
 * @brief Minimal WFDB header parser for embedded ECG applications.
 *
 * This module parses WFDB .hea header files stored in PROGMEM
 * and extracts basic metadata required to read low-resolution
 * ECG datasets (e.g., PTB-XL LR).
 *
 * The parser is intentionally minimal, contains no dynamic memory,
 * and operates directly on a null-terminated string located in
 * PROGMEM. It follows the style of lightweight embedded drivers
 * (TWI, UART, OLED).
 *
 * Only the fields required for PTB-XL LR files are parsed:
 * - Number of samples (nsamp)
 * - ADC gain (gain)
 * - Baseline offset (baseline)
 * - WFDB data format (fmt, typically 212)
 *
 * @{
 */

#ifndef WFDB_PARSER_H
#define WFDB_PARSER_H

#include <stdint.h>
#include <avr/pgmspace.h>

/**
 * @brief Parse WFDB header (.hea stored in PROGMEM).
 *
 * The function scans the PROGMEM string and extracts the following:
 * - Total number of samples (nsamp)
 * - Gain (µV/LSB)
 * - Baseline offset
 * - WFDB format (e.g., 212 for packed 2×12-bit)
 *
 * The implementation is limited to the PTB-XL low-resolution
 * header structure and assumes:
 *   - 1 lead
 *   - First line contains: `<name> <nsig> <fs> <nsamp>`
 *   - Second line contains: `<lead> <gain>+<baseline> ...`
 *
 * @param hea       Pointer to PROGMEM header string.
 * @param nsamp     Output: number of samples in the ECG record.
 * @param gain      Output: ADC gain (µV/LSB).
 * @param baseline  Output: baseline offset (LSB).
 * @param fmt       Output: WFDB format (e.g., 212).
 *
 * @return None.
 */
void wfdb_parse_header(const char *hea,
                       uint32_t *nsamp,
                       float *gain,
                       float *baseline,
                       uint8_t *fmt);

#endif /* WFDB_PARSER_H */

/** @} */
