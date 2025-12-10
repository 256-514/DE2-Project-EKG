#include "ppg_sensor.h"
#include <avr/interrupt.h> 
#include <util/delay.h>
#include <stdio.h>

// --- Externí Deklarace ---

// Deklarace externí proměnné z Arduino C++ jádra pro měření času.
extern volatile unsigned long timer0_millis; 

// --- Konstanty a Globální Proměnné pro Algoritmus ---

#define THRESHOLD 10       // SNÍŽENÁ prahová hodnota pro lepší citlivost (z 30 na 20)
#define MIN_BPM 40
#define MAX_BPM 180
#define SAMPLE_SIZE 8      // ZRYCHLENÁ velikost klouzavého průměru (z 10 na 8)
#define BEAT_COUNT 8       // Počet detekovaných tepů pro průměrování IBI

// Stavové proměnné
static uint16_t Signal = 0;
static uint16_t running_avg = 512; 
static uint16_t pulse_threshold = 512 + THRESHOLD;
static bool Pulse = false;         
static uint32_t lastBeatTime = 0;  

// Proměnné pro výpočet BPM
static uint16_t beatIntervals[BEAT_COUNT]; 
static uint8_t IBI_index = 0;
static uint8_t beatCounter = 0;
static bool firstBeat = true;

// --- Implementace Pomocných Funkcí ---

// Funkce pro bezpečné získání času v milisekundách
uint32_t get_millis(void) {
    uint32_t m;
    uint8_t oldSREG = SREG;
    cli(); 
    m = timer0_millis;
    SREG = oldSREG;
    sei(); 
    return m;
}

void PPG_init(void) {
    ADMUX = (1 << REFS0) | (PPG_ADC_CHANNEL); 
    
    // Povolit ADC (ADEN), Předdělička 128
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t PPG_read_sensor_value(void) {
    ADCSRA |= (1 << ADSC);

    while (ADCSRA & (1 << ADSC));
    
    return ADC;
}

BPM_t PPG_calculate_bpm(void) {
    uint32_t current_time;
    uint32_t TimeBetweenBeats;
    uint32_t totalIBI;
    BPM_t calculated_bpm = 0;

    Signal = PPG_read_sensor_value();

    running_avg = (running_avg * (SAMPLE_SIZE - 1) + Signal) / SAMPLE_SIZE;
    pulse_threshold = running_avg + THRESHOLD;

    if (Signal > pulse_threshold) {
        if (!Pulse) {
            Pulse = true;
            current_time = get_millis();
            TimeBetweenBeats = current_time - lastBeatTime;
            lastBeatTime = current_time;

            if (firstBeat) {
                firstBeat = false;
                return 0; 
            }

            if (TimeBetweenBeats > 1500 || TimeBetweenBeats < 330) {
                 return 0;
            }

            beatIntervals[IBI_index] = (uint16_t)TimeBetweenBeats;
            IBI_index = (IBI_index + 1) % BEAT_COUNT;
            
            if (beatCounter < BEAT_COUNT) {
                beatCounter++;
            }

            if (beatCounter >= BEAT_COUNT) {
                totalIBI = 0;
                for (uint8_t i = 0; i < BEAT_COUNT; i++) {
                    totalIBI += beatIntervals[i];
                }
                
                totalIBI /= BEAT_COUNT; 
                
                calculated_bpm = (BPM_t)(60000 / totalIBI);

                if (calculated_bpm >= MIN_BPM && calculated_bpm <= MAX_BPM) {
                    return calculated_bpm;
                } else {
                    return 0; 
                }
            }
        }
    } else {
        Pulse = false; 
    }

    if (beatCounter >= BEAT_COUNT) {
         uint32_t lastIBI = 0;
         for (uint8_t i = 0; i < BEAT_COUNT; i++) {
             lastIBI += beatIntervals[i];
         }
         lastIBI /= BEAT_COUNT;
         calculated_bpm = (BPM_t)(60000 / lastIBI);

         if (calculated_bpm >= MIN_BPM && calculated_bpm <= MAX_BPM) {
             return calculated_bpm;
         }
    }
    
    return 0; 
}