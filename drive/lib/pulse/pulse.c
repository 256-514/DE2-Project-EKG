#include "pulse.h"
#include <avr/io.h>
#include <stdlib.h> // pro abs()

#define ADC_CHANNEL 0  // A0
#define FILTER_SIZE 16 // Zvětšeno pro hladší signál

static uint16_t filter_buf[FILTER_SIZE];
static uint8_t filter_idx = 0;

// Proměnné pro BPM
static uint16_t threshold = 550; // Výchozí práh
static uint8_t triggered = 0;
static uint32_t last_beat_time = 0;

// Proměnné pro automatické nastavení prahu (DC offset filter)
static uint32_t sum_dc = 0;
static uint16_t avg_dc = 512;

void pulse_init(void)
{
    // Vref=AVcc, Channel=0
    ADMUX = (1 << REFS0) | ADC_CHANNEL;
    // Prescaler 128 (16MHz/128 = 125kHz ADC clock)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    
    // Init buffer
    for(int i=0; i<FILTER_SIZE; i++) filter_buf[i] = 512;
}

uint16_t pulse_read(void)
{
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

uint16_t pulse_read_filtered(void)
{
    uint16_t raw = pulse_read();
    
    // Klouzavý průměr signálu
    filter_buf[filter_idx] = raw;
    filter_idx = (filter_idx + 1) % FILTER_SIZE;
    
    uint32_t sum = 0;
    for(int i=0; i<FILTER_SIZE; i++) sum += filter_buf[i];
    uint16_t filtered = (uint16_t)(sum / FILTER_SIZE);

    // Dynamický výpočet DC složky (velmi pomalý průměr)
    // Toto nám dá "střední hodnotu" signálu, kolem které pulz osciluje
    // avg_dc = 0.99 * avg_dc + 0.01 * filtered (zjednodušeně v integer aritmetice)
    // Použijeme jednoduchý trik: průměr z dlouhé historie
    // Nebo jednodušeji: práh nastavíme fixně nad průměr
    
    // Jednoduchý DC tracker:
    if (filtered > avg_dc) avg_dc++;
    else if (filtered < avg_dc) avg_dc--;
    
    // Práh nastavíme dynamicky: Průměr + Offset
    // Offset musí být dost velký, aby ignoroval šum, ale menší než tep
    threshold = avg_dc + 30; // Offset 30-50 funguje dobře pro HW-827
    
    return filtered;
}

uint16_t pulse_process_for_bpm(uint16_t signal)
{
    uint16_t bpm = 0;
    uint32_t now = millis();
    uint32_t delta = now - last_beat_time;

    // Detekce náběžné hrany nad prahem
    if (signal > threshold && !triggered)
    {
        // Časový filtr: Ignoruj tepy rychlejší než 200 BPM (300ms)
        // Tím odfiltrujeme "dvojité zákmity" na vrcholu vlny
        if (delta > 300) 
        {
            triggered = 1;
            
            // Výpočet BPM (platný interval 300ms až 2500ms = 24-200 BPM)
            if (delta < 2500) {
                bpm = (uint16_t)(60000UL / delta);
                
                // Jednoduchý průměr pro stabilitu (volitelné)
                // bpm = (last_bpm + new_bpm) / 2; 
            }
            last_beat_time = now;
        }
    }
    // Reset triggeru s hysterezí
    // Musí klesnout pod (Práh - Hystereze), aby se mohl detekovat nový tep
    else if (signal < (threshold - 20) && triggered)
    {
        triggered = 0;
    }
    
    return bpm;
}
