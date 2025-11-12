# DE2-Project-EKG
  **Vysoké učení technické v Brně, Fakulta elektrotechniky a komunikačních technologií, Ústav radioelektroniky, 2025/2026**  

---

# Řídicí systém pro ultrazvukové senzory parkovacího asistenta


## 👥 Členové týmu

 - Tomáš Běčák - Odpovědný za Github, schéma a display_control
 - Mykhailo Krasichkov - Odpovědný za echo_detect, trig_pulse a zapojení na desce
 - Daniel Kroužil - Odpovědný za Github, controller

## 📝 Popis projektu

Tento projekt realizuje měření vzdálenosti pomocí dvou ultrazvukových senzorů HS-SR04, řízených FPGA. Systém umožňuje:
 - **Měření vzdálenosti:**

 - **Zobrazení:**

 - **Signalizace:**

 
## 🔌 Hardware

Použité komponenty
 - Deska Arduino UNO Digital R3
 - EKG monitor AD8232
 - LCD display 
 - Rezistory 2,2k a 4,7k, nepájivé pole, přepojovací vodiče

## 🎚️ Zapojení 

| Pin       | Komponenta     | Funkce                                                          |
|-----------|----------------|-----------------------------------------------------------------|
| JA0       | Levý senzor    | Trigger                                                         |
| JC0       | Levý senzor    | Echo                                                            |
| JD0       | Pravý senzor   | Trigger                                                         |
| JB0       | Pravý senzor   | Echo                                                            |
| SW[8:0]   | Přepínače      | Nastavení prahové hodnoty (0–511 cm)                            |
| BTNU      | Tlačítko       | Reset                                                           |
| BTNC      | Tlačítko       | Zbrazení vzdálenosti na osmimístném sedmisegmentovém displeji   |
| BTND      | Tlačítko       | Zobrazit práhové hodnoty (0-511 cm)                             |

## 🛠️ Hardware design


## ⚙️ Funkce systému



## 🔍 Jak to funguje uvnitř?

📂 **Hlavní soubory**
 - [top_level.vhd](project_files/top_level.vhd) – Tento hlavní 'top' modul propojuje všechny komponenty.


 - [echo_receiver.vhd](project_files/echo_receiver.vhd) – Tento modul slouží k měření vzdálenosti na základě doby trvání signálu ```echo_in```. Po obdržení impulsu se 

## Video ukázka měření


---
---
