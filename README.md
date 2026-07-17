# CanGeigerProbe
## Sonda promieniowania G-M z interfejsem CAN

Polowa sonda promieniowania jonizującego z miniaturową lampą Geigera-Müllera, komunikująca się cyfrowo przez **CAN 2.0B**. Zaprojektowana jako element rozproszonego systemu pomiarowego: wąska sonda cylindryczna w obudowie aluminiowej, jedno złącze M12, micropower, odporność EMC klasy przemysłowej.

![status](https://img.shields.io/badge/status-faza%20schematu-orange)
![hw](https://img.shields.io/badge/hardware-v4.0-blue)
![mcu](https://img.shields.io/badge/MCU-STM32L432KCU6-brightgreen)
![bus](https://img.shields.io/badge/bus-CAN%202.0B-informational)

> **Status:** kompletny projekt sprzętowy przed layoutem PCB. Firmware i mapa rejestrów CAN w osobnym opracowaniu.

Inspiracja: sondy Polon-Alfa (DPO-G, ZR-1) — jedna sonda, cyfrowe wyjście do jednostki nadrzędnej, wymienność lamp.

---

## Najważniejsze cechy

- **Programowalne HV 350–550 V** — jedna sonda obsługuje wiele typów lamp, setpoint ustawiany przez CAN bez zmian sprzętowych.
- **Micropower** — pobór idle rzędu setek µA dzięki buckowi LT8606 (Iq 2.5 µA), MCU w Stop 2 i podwójnemu wybudzaniu (RTC + aktywność magistrali).
- **Flyback sterowany deterministycznie z MCU** — brak dedykowanego kontrolera PWM; TIM2 generuje burst, ADC mierzy HV, firmware zamyka pętlę bang-bang.
- **CAN 2.0B (29-bit ID)** z wybudzaniem na aktywność magistrali, 125 kbit/s – 1 Mbit/s.
- **Wymienność lamp** — zwora `R_anode` na PCB pozwala dobrać rezystor gaszący pod konkretną lampę.
- **Odporność EMC** — kaskadowa ochrona coarse→fine (GDT/TVS), testy IEC 61000-4-2/-4/-5.
- **Obudowa aluminiowa IP67**, złącze M12 5-pin A-coded (standard CANopen CiA 303-1).

---

## Specyfikacja

| Parametr | Wartość |
|----------|---------|
| Detektor bazowy | DOI-80 (~400 V) |
| Zakres HV | 350–550 V, programowalny |
| Wielkości mierzone | CPS/CPM, moc dawki, dawka skumulowana, alarmy progowe |
| Interfejs | CAN 2.0B (extended ID), 125k / 250k / 500k / 1 Mbit |
| Zasilanie | 7–32 V DC, wspólna masa (bez izolacji galwanicznej) |
| Pobór idle | ~200 µA @ 12 V |
| Pobór aktywny | ~3.7 mA @ 12 V (chwilowo) |
| Ochrona EMC | ESD ±8 kV, Burst ±2 kV, Surge ±2 kV |
| Złącze | M12 5-pin A-coded |
| Obudowa | Aluminiowa cylindryczna Ø wewn. 25 mm, IP67 |

---

## Kompatybilne lampy G-M

Zmiana lampy = zmiana programowa HV + dobór zwory `R13` (rezystor gaszący).

| Lampa | HV nom. | R_anode (quench) | Uwaga |
|-------|---------|------------------|-------|
| **DOI-80** | ~400 V | **5.1 MΩ** | domyślna |
| SBM-20 / STS-5 | 400 V | 4.7 MΩ | drop-in z DOI-80 |
| SBM-19 | ~400 V | 5.1 MΩ | |
| J305 / M4011 | ~400 V | 4.7 MΩ | lampy chińskie |
| LND712 | 500 V | 10 MΩ | wymaga wyższego HV |

Trzy pola lutownicze — lutujesz jeden rezystor HV 1206 wg używanej lampy.

---

## Architektura

```
M12 Vin (7-32V) ─▶ Ochrona (GDT+TVS+ferryt) ─▶ LT8606 buck ─▶ +5V ─┬─▶ ATA6561 (CAN)
                                                                     ├─▶ TPS7A0230 LDO ─▶ +3.3V ─▶ STM32L432
                                                                     └─▶ Flyback (Q1 + T1 P18/11) ─▶ +HV
                                                                                                        │
   Lampa G-M ◀─ R13 (quench) ◀─ dzielnik HV 1GΩ:1MΩ ◀─────────────────────────────────────────────────┘
        │
        └─▶ dzielnik pojemnościowy ─▶ COMP1 ─▶ LPTIM (zliczanie w Stop 2)
```

Kluczowa cecha: **wspólna masa** (star ground), izolacja zapewniona wyłącznie przez TVS/GDT. Flyback „śpi" i budzi się kilka razy na sekundę — stąd pobór na poziomie µA.

Pełny schemat blokowy i szczegóły w [dokumentacji projektowej](docs/).

---

## Kluczowe komponenty

| Blok | Element | Uwaga |
|------|---------|-------|
| Buck | LT8606EMSE | Iq 2.5 µA (Burst Mode) — jedyny komponent spoza TME |
| LDO 3.3 V | TPS7A0230 | Iq 25 nA |
| MCU | STM32L432KCU6 | bxCAN, LPTIM, COMP1, TIM2, ADC, DAC1 |
| CAN | ATA6561-GAQW-N | zamiennik TJA1042 dostępny w TME |
| Transformator | własny P18/11-3F3, 17:510 | nawijany ręcznie, off-board |
| Klucz flyback | DMN3404L | 30 V, snubber RCD clamp do +5 V |
| Dioda HV | STTH112 | 1200 V |
| Dzielnik HV | 2× HVC1206 500 MΩ | /1001, ochrona ADC diodami BAV199 |
| Ochrona CAN | CDSOT23-T24CAN | dedykowany TVS |

Szacunkowy koszt kompletu: **~160–200 zł**.

---

## Transformator (DIY)

Najbardziej „ręczna" część projektu — rdzeń kubełkowy **P18/11-3F3**, mocowany śrubą **M2 A2 (nierdzewna, niemagnetyczna!)** z podkładkami nylonowymi chroniącymi kruchy ferryt.

- Pierwotne: **17 zw** drut 0.30 mm CuL
- Wtórne: **510 zw** drut 0.10 mm CuL (przekładnia 1:30)
- Szczelina: Kapton 100 µm między połówkami
- Parametry docelowe: Lp ≈ 75 µH, Ls ≈ 67 mH, leakage < 1.5 µH, izolacja P/S > 1 GΩ

Procedura nawijania, obliczenia i testy (hi-pot 1 kV/60 s) — patrz dokumentacja, sekcja 8.

---

## Roadmap

- [x] **Faza 1** — projekt: topologia, dobór komponentów, obliczenia, EMC, budżet mocy
- [ ] **Faza 2** — prototyp transformatora (nawinięcie, strojenie, testy)
- [ ] **Faza 3** — schemat i layout PCB w KiCad (strefy HV / analog / cyfrowe)
- [ ] **Faza 4** — firmware (LPTIM+COMP w Stop 2, pętla HV, bxCAN + wake-up, protokół)
- [ ] **Faza 5** — kalibracja HV per lampa, kalibracja CPS→µSv/h (Cs-137), testy EMC
- [ ] **Faza 6** — mechanika: obudowa Al, montaż M12, testy IP67

---

## Struktura repozytorium

```
.
├── docs/           # pełna dokumentacja projektowa
├── plan/           # założenia i pliki projektowe
├── sim/            # symulacje LTSpice
├── pcb/            # projekt KiCad (schemat, PCB, BOM)
└── firmware/       # (wkrótce)
```

---

## Referencje

- Analog Devices **CN-0536** — Geiger Counter Circuit (odczyt z anody, dzielnik pojemnościowy, komparator)
- STM32L432: RM0394, DS11453; bxCAN: AN2606, AN5028
- Ferroxcube P18/11, materiał 3F3; EPCOS/TDK B65651/B65652
- IEC 61000-4-2/-4/-5; EN 61326-1; EN 61000-6-2
- CAN 2.0B: ISO 11898-1/-2; CANopen CiA 301, CiA 303-1; M12: IEC 61076-2-101
- Polski podręcznik metrologii promieniowania — układy pracy liczników DOI/BOI/GOI

---

## Licencja

MIT 

---

> ⚠️ **Uwaga bezpieczeństwa:** urządzenie generuje napięcie ~350–550 V. Zachowaj ostrożność przy uruchamianiu i pomiarach obwodów HV.