<!-- prevent jekyll yaml parsing -->

  **Vysoké učení technické v Brně, Fakulta elektrotechniky a komunikačních technologií, Ústav radioelektroniky, 2025/2026**  

---

# Systém pro zobrazení EKG a PPG signálu a měření tepové frekvence. 

---

# 📚 Obsah

1. [Systém pro zobrazení EKG a PPG signálu a měření tepové frekvence](#systém-pro-zobrazení-ekg-a-ppg-signálu-a-měření-tepové-frekvence)
2. [Členové týmu](#členové-týmu)
3. [Popis projektu](#popis-projektu)
4. [Výpočet BPM](#výpočet-tepové-frekvence-bpm)
5. [Funkční bloky](#funkční-bloky-systému)
6. [Hardware](#hardware)
7. [Zapojení](#zapojení)
8. [Hardware design](#hardware-design)
9. [Funkce systému](#funkce-systému)
10. [Jak to funguje uvnitř](#jak-to-funguje-uvnitř)
11. [Hlavní soubory](#hlavní-soubory)
12. [Video ukázka měření](#video-ukázka-měření)
13. [Poznámky](#poznámky)

---

## 👥 Členové týmu

- **Tomáš Běčák** – Odpovědný za GitHub, implementace AD8232 driveru, spoluautor `main.c`  
- **Mykhailo Krasichkov** – Spoluautor posteru, spoluautor PPG driveru, spoluautor `main.c`  
- **Daniel Kroužil** – Spoluautor posteru, spoluautor PPG driveru, GitHub spoluadministrace, spoluautor `main.c`

## 📝 Popis projektu

Projekt realizuje přenosný **EKG/PPG monitor** založený na mikrokontroléru **ATmega328P (Arduino UNO)**.  
Systém umožňuje:

- snímat EKG signál modulem **AD8232**
- snímat optický PPG signál senzorem **HW-827**
- zobrazit waveform v reálném čase na **OLED SH1106 (128×64)**
- počítat tepovou frekvenci (BPM) pro oba režimy
- přepínat EKG ↔ PPG tlačítkem
- detekovat odlepení elektrod (LO+/LO–)

Firmware je napsaný **v čistém C**, bez Arduino frameworku.  
Veškeré vykreslování probíhá přes I2C/TWI knihovnu.

## 🧮 Výpočet tepové frekvence (BPM)

Systém počítá BPM zvlášť pro EKG i PPG.

### EKG (AD8232)
- detekce R-špiček pomocí jednoduchého adaptivního prahu  
- ukládá časy posledních detekcí  
- BPM se počítá:  

$BPM = \frac{60000}{\Delta t_{RR}}$

kde $\Delta t_{RR}$ je rozdíl dvou R-peak časů v milisekundách.

### PPG (HW-827)
- signál je filtrován (EMA + noise reduction)  
- pro zvýšení efektivní vzorkovací frekvence se měří více vzorků mezi refreshi displeje  
- adaptivní detekce peaků  
- z IBI (inter-beat interval) se počítá BPM podobně jako u EKG

---

## ⚙️ Funkční bloky systému

| Blok | Funkce |
|------|---------|
| **AD8232** | EKG snímač, analogová filtrace, LO+/LO– detekce |
| **PPG senzor (HW-827)** | Optické snímání průtoku krve, filtr + BPM algoritmus |
| **ATmega328P** | ADC převod, řízení režimů, výpočet BPM, filtrace |
| **OLED SH1106** | Kreslení waveformu a BPM v reálném čase |
| **Tlačítko** | Přepínání EKG ↔ PPG režimu |
| **Napájení** | 5 V (Arduino), 3.3 V pro AD8232 |

---

## 🔌 Hardware

Použité komponenty
- Arduino UNO (ATmega328P)
- AD8232 EKG modul
- HW-827 PPG senzor
- OLED 128×64 SH1106 (I2C)
- Tlačítko přepínání režimu

---

## 🎚️ Zapojení 

| Arduino | AD8232 |
|------|---------| 
| 3V3 | 3.3V | 
| GND | GND | 
| A0 | OUTPUT | 
| D2 | LO- | 
| D3 | LO+ | 

| Arduino | SH1106 | 
|------|---------| 
| 5V | VCC | 
| GND | GND | 
| A5 | SCK | 
| A4 | SDA |

| Arduino | HW-827/PPG senzor | 
|------|---------| 
| GND | GND | 
| 5V | VCC | 
| A1 | Signal |

| Arduino | Mikrospínač | 
|------|---------| 
| GND | Pravá horní nožička a levá horní nožička | 
| D6 | Pravá dolní nožička a levá dolní nožička | 

---

## 🛠️ Hardware design

<img src="images/zapojeni.jpg" alt="HW-827" width="500">

Obr. 1 Propojení

<img src="images/HW-827.jpg" alt="HW-827 (Pulse Sensor)" width="45%">

Obr. 2 Propojení HW-827 s piny desky Arduino UNO *(zdroj: [DevXplained](https://devxplained.eu/en/blog/heart-rate-module-part-1))*

<img src="images/AD8232.jpeg" alt="AD8232 (EKG modul)" width="45%">

Obr. 3 Propojení AD8232 s piny desky Arduino UNO *(zdroj: [Microcontrollers Lab](https://microcontrollerslab.com/ad8232-ecg-module-pinout-interfacing-with-arduino-applications-features/))*

---

## ⚙️ Funkce systému

- **EKG režim**
  - ADC čtení z A0
  - kontrola odlepení elektrod přes LO+/LO–
  - filtrování + adaptivní baseline
  - detekce R-špiček → výpočet BPM

- **PPG režim**
  - čtení z A1
  - digitální filtrace + oversampling
  - detekce pulsů → výpočet BPM
  - zvětšený vertikální zoom pro lepší čitelnost

- **OLED SH1106**
  - vykreslení waveformu (lineární spojnice)
  - zobrazení BPM
  - blikající srdce jako indikátor detekce beatů
  - úvodní animace „EKG / PPG Monitor“

---

## 🔍 Jak to funguje uvnitř?

### Hlavní program (`main.c`)
- inicializace všech modulů  
- přepínání režimů  
- řízení ADC  
- vykreslování na OLED  
- volání BPM algoritmů  

### EKG driver (`ad8232.c`)
- LO+ / LO– logika  
- čtení ADC0  

### BPM logika (`bpm.c`)
- detekce R-peaks  
- výpočet BPM  

### PPG driver (`ppg_sensor.c`)
- filtrace  
- detekce pulsů  
- BPM z IBI  

### OLED driver (`oled.c`)
- grafické primitivy  
- frame buffer  
- rendering  

### TWI (`twi.c`)
- I2C transport pro OLED
 

---

## 📂 **Hlavní soubory**
```
/..................................................Kořenový adresář projektu
├── include/.......................................Hlavičkové soubory projektu
│   └── timer.h....................................Prototypy časovače, systémová timebase
├── lib/...........................................Knihovny
│   ├── oled/......................................Ovladač OLED displeje SH1106
│   │   ├── oled.c.................................
│   │   ├── oled.h.................................
│   │   └── font.h.................................
│   ├── twi/.......................................I2C/TWI master ovladač pro AVR
│   │   ├── twi.c..................................
│   │   └── twi.h..................................
│   ├── uart/......................................UART ovladač (Peter Fleury)
│   │   ├── uart.c.................................
│   │   └── uart.h.................................
│   ├── bpm/.......................................Výpočet BPM + detekce špiček
│   │   ├── bpm.c..................................
│   │   └── bpm.h..................................
│   ├── ad8232/....................................Ovladač EKG modulu AD8232
│   │   ├── ad8232.c...............................
│   │   └── ad8232.h...............................
│   ├── ppg_sensor/................................Ovladač PPG senzoru + BPM algoritmus
│   │   ├── ppg_sensor.c...........................
│   │   └── ppg_sensor.h...........................
├── src/...........................................Zdrojové kódy
│   └── main.c.....................................Hlavní řídicí logika systému (EKG/PPG, OLED, BPM)
└── README.md......................................Dokumentace projektu
```

### Klíčové vlastnosti firmware
- **Timer0** generuje millis() přes přerušení  
- **X-SCALE** (1–4) umožňuje roztáhnout waveform v ose X  
- **Bez Arduino frameworku** – čisté AVR C  
- **Bez dynamické paměti**, OLED používá statický buffer  
- **Debounce tlačítka softwarem**  
- **Bezpečné přepínání ADC kanálů mezi EKG a PPG**

---

## Video ukázka měření

---

## Prototyp:
<video src="videos/Prototype.mp4" controls width="500"></video>

---

Poznámky:
- nepoužívá `arduino.h`
- nepoužívá Arduino knihovny
- projekt je plně v C (AVR-GCC)
