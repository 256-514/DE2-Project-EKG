  **Vysoké učení technické v Brně, Fakulta elektrotechniky a komunikačních technologií, Ústav radioelektroniky, 2025/2026**  

---

# Systém pro zobrazení EKG signálu a měření tepové frekvence. 


## 👥 Členové týmu

 - Tomáš Běčák - Odpovědný za Github
 - Mykhailo Krasichkov - Odpovědný za Github
 - Daniel Kroužil - Odpovědný za Github

## 📝 Popis projektu

Tento projekt realizuje přenosný **EKG monitor**, který snímá a zobrazuje elektrickou aktivitu srdce pomocí modulu **AD8232** a mikrokontroléru **ATmega328P (Arduino UNO)**.
Systém kromě vizualizace EKG signálu také automaticky vypočítává tepovou frekvenci. Výpočet probíhá na mikrokontroléru ATmega328P.

## 🧮 Výpočet tepové frekvence (BPM)

Systém v reálném čase vypočítává tepovou frekvenci na základě detekce R-špiček v EKG signálu.

### Postup:
1. Při detekci R-špičky mikrokontrolér změří čas od předchozí R-špičky.  
2. Tento interval (RR interval) se použije pro výpočet tepové frekvence:
   

$BPM = \frac{60000}{\Delta t_{RR}}$

kde $ \Delta t_{RR} $ je rozdíl dvou R-peak časů v milisekundách.

## ⚙️ Funkční bloky systému

| Blok | Funkce |
|------|---------|
| **AD8232** | Zesílení a analogová filtrace EKG, detekce odlepené elektrody |
| **ATmega328P** | ADC převod, EMA filtr, detekce R-špiček, výpočet BPM |
| **LCD PB200-142 (HD44780)** | Zobrazení EKG waveformu a BPM |
| **LED indikace** | Zelená – měření, Červená – stop/chyba |
| **Tlačítko** | Start/Stop měření |
| **Napájení** | 5 V přes USB, AD8232 z 3.3 V |

## 🔌 Hardware

Použité komponenty
 - Deska Arduino UNO Digital R3
 - EKG monitor AD8232
 - I2C OLED display 128x64, driver SH1106
 - LCD display Digilent PmodCLP

## 🎚️ Zapojení 


## 🛠️ Hardware design


## ⚙️ Funkce systému
 - **Měření signálu EKG:**
 
 - **Zesílení signálu mV --> V**

 - **Vzorkování, hledáni R špiček**

 - **Zobrazení signálu a tepové frekvence:**


## 🔍 Jak to funguje uvnitř?


📂 **Hlavní soubory**
```
/..................................................Kořenový adresář projektu
├── .vscode........................................
├── include........................................
│   │   └── timer.h................................
├── lib............................................Knihovny
│   ├── oled.......................................
│   │   ├── oled.c
│   │   ├── oled.h
│   │   └── font.h
│   ├── uart.......................................
│   │   ├── uart.c.................................
│   │   └── uart.h.................................
│   ├── twi........................................
│   │   ├── twi.c..................................
│   │   └── twi.h..................................
├── src........................................ 
│   └── main.c.................................
├── test....................................... 
│   ├──
│   ├──
├── platformio.ini.............................
│   └──
└── build......................................
```
## Video ukázka měření


---
---
Poznámky:
žádná knihovna arduiono.h
ne arduino framework
zdrojáky C a hlavičkové soubory
