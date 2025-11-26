// ... (includes a init funkce zůstávají stejné) ...
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>


#include "timer.h" // Váš poskytnutý timer.h
#include "pulse.h"
#include "oled.h"


// --- Millis implementace pomocí Timer0 Overflow ---
volatile uint32_t ms_counter = 0;


// Timer0 Overflow ISR - voláno každou 1ms (viz nastavení níže)
ISR(TIMER0_OVF_vect)
{
    ms_counter++;
    // TCNT0 se v Normal mode resetuje samo po přetečení (255 -> 0)
    // Ale pozor: Pro přesně 1ms při 16MHz a prescaleru 64 potřebujeme 250 tiků.
    // Overflow je 256 tiků. To je chyba cca 2.4%.
    // Pro BPM to stačí, ale můžeme to kompenzovat nastavením TCNT0 = 6;
    TCNT0 = 6;
}


uint32_t millis(void)
{
    uint32_t m;
    cli();
    m = ms_counter;
    sei();
    return m;
}


int main(void)
{
    // ... (init hardware stejný jako předtím) ...
       
    // Inicializace Timer0 pro millis() pomocí maker z timer.h
    // tim0_ovf_1ms() nastaví prescaler 64 -> overflow cca každou 1.024 ms
    tim0_ovf_1ms();
    tim0_ovf_enable(); // Povolit přerušení
   
    pulse_init();      // Inicializace ADC
   
    // Oled init (v oled.c se často používá I2C, ověřte zda je I2C init součástí oled_init)
    // Pokud vaše oled.c nepoužívá twi_init, musíte ho zavolat zde.
    // Ale většina oled knihoven si I2C nastaví.
   
    oled_init(OLED_DISP_ON);
    oled_clrscr();
    oled_charMode(NORMALSIZE);


    sei(); // Povolit globální přerušení (nutné pro Timer a ADC)


    // --- Proměnné ---
    char buffer[20];
    uint16_t current_bpm = 0;
    uint16_t signal_val = 0;
    uint8_t heart_anim = 0;
    uint32_t last_draw = 0;


    // Zobrazování průměrného BPM pro lepší čitelnost
    uint16_t bpm_sum = 0;
    uint8_t bpm_count = 0;
    uint16_t display_bpm = 0;

    while (1)
    {
        // 2. Čtení a zpracování
        signal_val = pulse_read_filtered();
        uint16_t new_bpm = pulse_process_for_bpm(signal_val);

        if (new_bpm > 0) {
            // Průměrování posledních 3 tepů pro stabilitu čísla na displeji
            bpm_sum += new_bpm;
            bpm_count++;
            if (bpm_count >= 3) {
                display_bpm = bpm_sum / 3;
                bpm_sum = 0;
                bpm_count = 0;
            }
            // Okamžitá reakce pro animaci
            heart_anim = 1;
        }

        // 3. Vykreslování (každých 100ms)
        if (millis() - last_draw > 100)
        {
            last_draw = millis();
            oled_clrscr();
            
            oled_gotoxy(0, 0);
            oled_puts("--- EKG Monitor ---");

            oled_gotoxy(0, 2);
            if (display_bpm > 0) {
                sprintf(buffer, "BPM: %d", display_bpm);
            } else {
                sprintf(buffer, "BPM: --");
            }
            oled_puts(buffer);

            // Výpis RAW a PRAHU pro diagnostiku
            // V pulse.c jsme threshold udělali static, takže ho nevidíme přímo.
            // Pro ladění můžete v pulse.c přidat funkci `uint16_t get_threshold()`
            // Zde jen vypíšeme RAW hodnotu
            oled_gotoxy(0, 4);
            sprintf(buffer, "Raw: %d", signal_val);
            oled_puts(buffer);
            
            if (heart_anim) {
                oled_gotoxy(80, 2);
                oled_puts("<3"); 
                heart_anim = 0;
            }
            
            #ifdef GRAPHICMODE
            oled_display();
            #endif
        }
        _delay_ms(2); 
    }
    return 0;
}
