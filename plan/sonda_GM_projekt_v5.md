# Sonda promieniowania z licznikiem Geigera-Müllera — dokumentacja projektowa

**Wersja:** 5.0 (zsynchronizowana ze schematem KiCad — geiger_probe.kicad_sch)
**Data:** 2026-07-21
**Autor:** Bartek
**Status:** Schemat narysowany (6 arkuszy hierarchicznych), przed layoutem PCB

**Zmiany kluczowe vs v4.0:**
- **Korekta LT8606:** referencja FB to **0.778 V** (nie 0.970 V). Dzielnik **1M/187k → 4.94 V** — poprawny. Obudowa **DFN-8 (LT8606IDC), zawsze Burst Mode**.
- LDO 5V→3.3V: **MCP1700-3302 (SOT-89)** zamiast TPS7A0230
- Dodano **pomiar prądu Vin: INA186 A4 (G=200) + shunt 0.5 Ω**
- Dodano **zewnętrzny RTC MCP79410 + ogniwo podtrzymujące** (brak wykorzystania VBAT + źródło 32.768 kHz do kalibracji MSI)
- Terminacja CAN: **PhotoMOS GAQY212GS ×2 + BSS138 + 59 Ω** (split, przełączalna)
- Snubber: **BAV21WS + 100 Ω + 1 nF do +5V**
- Ochrona: **GDT 3-elektrodowe 3R090-5S ×2** (LCSC), podwójne ferryty (z doświadczeń burst)
- Odniesienia elementów (RefDes) zsynchronizowane z arkuszami KiCad

**Uwaga o nazwach netów:** wewnątrz arkuszy hierarchicznych sygnały mają nazwy lokalne dla czytelności (np. `GM_OUT` w hv, `COMP_GM_IN` w mcu; `V_zas` w prot, `V_IN` w pwr). Są zwarte na arkuszu głównym (root).

---

## Struktura arkuszy KiCad

| Arkusz | Plik | Zawartość |
|--------|------|-----------|
| root | geiger_probe.kicad_sch | Połączenia hierarchiczne |
| PWR | pwr.kicad_sch | Buck LT8606, LDO MCP1700, dzielnik VIN_SENSE |
| MCU | mcu.kicad_sch | STM32L432, RTC MCP79410, kwarc 32k, złącze SWD |
| HV | hv.kicad_sch | Flyback + detektor GM (trafo, Q1, snubber, dzielnik HV, shaper) |
| CAN_BUS | can_bus.kicad_sch | ATA6561, terminacja PhotoMOS |
| PROTECTION | prot.kicad_sch | Ochrona wejścia Vin i CAN, złącze M12 |

---

## 1. Cel i założenia

Sonda promieniowania jonizującego typu polowego, kompatybilna z wieloma lampami G-M (bazowo DOI-80; SBM-20, STS-5, SBM-19, J305, M4011, LND712). Micropower, zasilanie z długiej linii przemysłowej 7–32 V, komunikacja CAN 2.0B z wybudzaniem, obudowa aluminiowa (priorytet conducted immunity), odporność EMC IEC 61000-4-2/-4/-5.

| Parametr | Wartość |
|----------|---------|
| Zakres HV | 350–550 V programowalnie |
| Interfejs | CAN 2.0B (extended ID 29-bit) |
| Zasilanie | Vin = 7–32 V DC |
| Pobór idle | ~200 µA @ 12 V |
| Złącze | M12 5-pin A-coded |

---

## 2. Zasilanie — arkusz PWR

### 2.1 Buck LT8606 (Vin → 5V)

**IC1: LT8606IDC#TRPBF** — obudowa DFN-8 (2×2 mm), wersja I (-40..+125°C). Zamawiany spoza TME (Mouser/DigiKey).

| Parametr | Wartość | Uwaga |
|----------|---------|-------|
| Vin | 3.0–42 V | Margines dla przepięć |
| Iout | 350 mA (DFN) | Wystarcza (obciążenie <20 mA) |
| Iq | 2.5 µA | Micropower |
| **FB reference** | **0.778 V** | Kluczowe — patrz dzielnik |
| Tryb | **zawsze Burst Mode** (DFN, brak SYNC) | Brak spread-spectrum |

**Dzielnik FB (poprawny dla 5 V):**
```
R4 = 1 MΩ (góra), R3 = 187 kΩ (dół)
Vout = 0.778 × (1 + 1M/187k) = 0.778 × 6.348 = 4.94 V ✓
C2 = 10 pF NP0 — feedforward na R4
```

**Częstotliwość:** R1 = 18.2 kΩ → ~2 MHz (mała cewka).

**Cewka L1 = 6.8 µH:** Murata **LQH32PN6R8NN0L** (1210, Isat ~850–900 mA, wire-wound shielded power inductor). Isat z zapasem nad current limit LT8606.

| Element | Wartość | Rola |
|---------|---------|------|
| IC1 | LT8606IDC#TRPBF | Buck (spoza TME) |
| L1 | LQH32PN6R8NN0L (6.8µH) | Cewka |
| R4 / R3 | 1M / 187k 1% | Dzielnik FB → 4.94 V |
| C2 | 10p NP0 | Feedforward |
| R1 | 18k2 1% | RT (~2 MHz) |
| C4 | 100n | BST |
| C7 | 10u | Vin bulk |
| C5, C8 | 100n | Decoupling |
| C6, C3 | 1u | INTVCC / filtr |

**Do weryfikacji (ERC):** pin EN/UV — upewnić się, że próg załączenia ≤7 V (podpięty do Vin lub dzielnik ustawiony na ~6.5 V), by sonda startowała od dolnego końca zakresu.

### 2.2 LDO 5V → 3.3V (MCP1700)

**U2: MCP1700x-330xMB** — SOT-89, Iq 1.6 µA, Vin max 6 V (pokrywa 5 V), Iout 250 mA. Zasila VDD/VDDA MCU.

| Element | Wartość | Rola |
|---------|---------|------|
| U2 | MCP1700-3302E/MB | LDO 3.3 V |
| C23 | 1u | Wyjście (≥1µF wymagane dla stabilności) |
| C25 | 100n | Decoupling |
| C26 | 1u | Wejście |

Pobór idle: 1.6 µA (vs 25 nA dla TPS7A0230, ale różnica <1% budżetu — akceptowalne).

### 2.3 Dzielnik VIN_SENSE

```
V_IN → R25 (1M) → VIN_SENSE → R26 (100k) → GND      (dzielnik 11:1)
Przy 32 V: VIN_SENSE = 2.9 V (~88% FSR 3.3V)
Prąd dzielnika: 32/1.1M = 29 µA
```
Rozważyć klamrę do 3.3 V przy pinie ADC (na wypadek przepięcia >36 V).

---

## 3. Pomiar prądu Vin — INA186 (NOWE)

### 3.1 Koncepcja

Pomiar high-side prądu pobieranego przez sondę, na linii Vin za ochroną, przed buckiem. **INA186** — 40 V common-mode (pokrywa Vin 7–32 V), pin enable, zero-drift (niski offset).

### 3.2 Dobór — A4 (G=200) + shunt 0.5 Ω

FSR spójne z resztą (VIN_SENSE celuje 2.9 V) → target V_out_max ≈ 3.0 V. I_max_FS = 30 mA (zapas ~50% nad najgorszym active-CAN ~20 mA @ 7 V).

```
R_shunt = V_out_max / (I_max_FS × G) = 3.0 / (0.030 × 200) = 0.5 Ω
```

**Dlaczego A4 (G=200), nie A1 (G=25):** przy tej samej skali rozdzielczość ADC identyczna (R×G stałe), ale A4 pozwala na mały shunt 0.5 Ω (zamiast 4 Ω) — łagodny dla linii zasilania. A1 dałby 2 V spadku i 1 W przy inrush (zakłóca start bucka). A4 = 15 mV spadku, 125 mW przy inrush.

| Parametr | Wartość |
|----------|---------|
| Wzmacniacz | INA186 A4 (G=200), 40 V CM, EN |
| R_shunt | 0.5 Ω 1% (E24: 0.51 Ω), 1206 |
| Pełna skala | 30 mA (33 mA przy 3.3 V) |
| Rozdzielczość (14-bit OVS) | ~2 µA/count |
| Idle 200 µA | ~100 count (0.7%) |
| Spadek @ 30 mA | 15 mV |

### 3.3 Podłączenie

```
VIN_F → R_shunt (0.5Ω) → buck
         │        │
       IN+ ── INA186 A4 ── OUT → R_filt (1k) → ADC ← C_filt (100n) → GND
       IN-                          (τ=100µs, uśrednia tętnienie SMPS)
REF → GND        (pomiar jednokierunkowy)
EN  → GPIO       (włącz → 20µs → pomiar → wyłącz)
VS  → +3.3V
```

---

## 4. MCU i RTC — arkusz MCU

### 4.1 STM32L432KCU6 (U1)

UFQFPN32, bxCAN, Stop 2 ~1.4 µA. VDD/VDDA z LDO 3.3 V. VREF+ wewnętrznie związane z VDDA → referencja ADC przez VREFINT (korekcja programowa).

**Rozważyć:** ferryt między VDD a VDDA (czystsza referencja analogowa) — do dodania.

### 4.2 Zewnętrzny RTC MCP79410 (IC8) — NOWE

**Powód:** wybrana konfiguracja MCU nie wykorzystuje podtrzymania bateryjnego VBAT — po odłączeniu zasilania data/czas byłyby tracone. MCP79410 (RTC + EEPROM + SRAM podtrzymywane) rozwiązuje to. Dodatkowo kwarc **32.768 kHz** służy jako źródło do **kalibracji wewnętrznego MSI** STM32.

| Element | Wartość | Rola |
|---------|---------|------|
| IC8 | MCP79410-I/MS | RTC/EEPROM, I2C |
| Y1 | 32.768 kHz | Kwarc RTC + kalibracja MSI |
| C42, C43 | 6.2p | Obciążenie kwarcu |
| BT1 | MS621FE-FL11E | Ogniwo podtrzymujące (rechargeable) |
| D27 | BAT54J | Dioda-OR podtrzymania |
| R19, R20, R18 | 10k | Pull-upy I2C / sygnały |

### 4.3 Programowanie i pozostałe

J2 (TC2030 Tag-Connect) — SWD. R17/R46 10k. Kondensatory decoupling 100n na VDD.

### 4.4 Przypisanie pinów (kluczowe)

| Pin | Sygnał | Net |
|-----|--------|-----|
| PA1 | COMP1_INP | COMP_GM_IN (z detektora GM) |
| PA2 (TIM2_CH3) | Gate flybacka | FBACK_CTRL |
| PA5 | ADC HV | HV_meas |
| PA6 | ADC Vin | VIN_SENSE |
| PA11 / PA12 | CAN_RX / CAN_TX | do ATA6561 |
| PA7 | STBY | CAN_STBY |
| — | ADC prąd | (INA186 OUT) |
| — | EN INA186, CAN_R_CTRL | GPIO |

**Weryfikacja ERC:** `GM_OUT` (hv) ↔ `COMP_GM_IN` (mcu) muszą być zwarte na root; `V_zas` (prot) ↔ `V_IN` (pwr) również.

---

## 5. Generator HV i detektor — arkusz HV

### 5.1 Flyback (sterowany z MCU)

Flyback DCM, single-switch, non-izolowany, sterowany burst-by-burst z TIM2. Zasilanie z **+5V** (nie Vin). Blanking komparatora 50 µs podczas burst.

**Transformator T1 (P18/11):**
- Rdzeń P18/11-3F3 (lub B65651D0000R048), ungapped; karkas B65652B0000T001
- Szczelina Kapton 100 µm; AL ~270 nH/N²
- **17 zw (0.30 mm)** pierwotne : **510 zw (0.10 mm)** wtórne; n=1:30; Lp=75 µH
- Mocowanie: śruba M2×16 A2 + nakrętka nylock + 2× podkładka nylonowa + epoksyd
- **Weryfikacja kropek:** pierwotne kropką do +5V; wtórne tak, by D2 przewodziła w fazie off

**Netlist flybacka:**
| RefDes | Wartość | Rola |
|--------|---------|------|
| Q1 | DMN3404L | MOSFET 30 V |
| R6 | 22R | Gate resistor |
| R7 | 100k | Gate pull-down |
| D2 | STTH112 | Dioda HV 1200 V |
| C14 | 2.2n/1kV | Filtr peak |
| C15 | 100n/1kV | Reservoir HV |
| C12 | 10u | Bulk 5 V |

**Snubber RCD (do +5V):**
| RefDes | Wartość | Rola |
|--------|---------|------|
| D1 | BAV21WS | Dioda clamp (ultrafast) |
| R8 | 100R | Rozładowanie |
| C13 | 1n | Magazyn energii leakage |

Uzasadnienie clamp do +5V: recykling energii, Vds ~20 V (bezpieczne dla 30 V), cichy EMC.

### 5.2 Dzielnik HV → ADC

```
HV+ → R9 (500M/500V) → R10 (500M/500V) → HV_meas → R11 (1M) → GND
HV_meas: C16 (100p NP0) + D3 (BAV199 klamry 3.3V/GND) + ADC (PA5)
```
/1001; przy 550 V → 0.549 V. Tolerancja 5% usuwana kalibracją. BAV199 (upływ <5 nA) chroni ADC.

### 5.3 Detektor G-M i shaper (robust)

Odczyt z anody, katoda na GND. Dzielnik pojemnościowy o kontrolowanej amplitudzie.

| RefDes | Wartość | Rola |
|--------|---------|------|
| R12 | 5.1M/1kV | Rezystor gaszący (quench) — dla DOI-80 |
| C17 | 3.3p/1kV | Ca na R12: definiuje ładunek, przyspiesza gaszenie |
| C18 | 4.7p/1kV | C_top (dzielnik pojemnościowy) |
| C19 | 1n/50V | C_bot (stosunek ~213) |
| R13 | 1M | R_bias do 3.3 V |
| R14 | 1k | R_s (szereg do komparatora) |
| D4 | BAV199 | Klamry wejścia komparatora |

Impuls: anoda spada ~400 V → dzielnik przenosi ~1.9 V → SIGNAL spada poniżej progu → COMP1 (PA1) → LPTIM. Próg z DAC1.

R12 dobierany pod lampę (wartości z podręcznika): DOI-80 5.1M, SBM-20/STS-5 4.7M, LND712 10M.

---

## 6. CAN — arkusz CAN_BUS

### 6.1 Transceiver ATA6561 (IC2)

**ATA6561-GAQW-N** (SO-8), zamiennik niedostępnego TJA1042. VCC=5V, VIO=3.3V, Standby ~10 µA.

```
TXD ← PA12, RXD → PA11, STBY ← PA7, VIO ← 3.3V, VCC ← 5V
CANH/CANL → terminacja + ochrona (arkusz PROTECTION)
```
Decoupling: C32/C33 100n, C34 10u, C35 100n.

### 6.2 Terminacja split przełączalna (PhotoMOS)

Dwa PhotoMOS w szeregu z gałęziami 59 Ω, sterowane jednym pinem przez szeregowe LED-y i MOSFET.

| RefDes | Wartość | Rola |
|--------|---------|------|
| U701, U702 | GAQY212GS (AQY212GS) | Przełączniki gałęzi (bidirekcyjne, izolowane) |
| R21, R22 | 59 Ω | Split termination (2×59 + Ron ≈ 120 Ω) |
| C_split | 4.7 nF | **Środek dzielnika → GND** (zweryfikować: nF, nie pF!) |
| R23 | 560 R | Rezystor szeregowy LED |
| Q2 | BSS138 | Przełącznik LED z GPIO (N-ch, niższy Vgs(th) niż 2N7002) |
| R24 | 100k | Pull-down bramki (terminacja OFF przy resecie) |

Sterowanie: `+5V → R23 → LED_U701 → LED_U702 → Q2.drain; Q2.gate → GPIO (CAN_R_CTRL)`. LED-y w szeregu → ~4 mA na oba (2× mniej niż równolegle).

**Weryfikacja:** kondensator split-termination musi być ~4.7 **nF** (nie pF). Sprawdzić C39/C40/C41 na schemacie.

---

## 7. Ochrona wejścia — arkusz PROTECTION

### 7.1 Kaskada Vin (coarse → fine)

```
J1.1 → GD1 (GDT 3-el. → Earth) → F1 (PTC) → D5 (TVS) → FB1 (ferryt) → D7 (Schottky) → D6/D8 (TVS) → C20/C21 → V_zas
```

| RefDes | Wartość | Rola | Źródło |
|--------|---------|------|--------|
| GD1 | 3R090-5S (GDT 3-el. 90V) | Coarse surge Vin→Earth | LCSC |
| F1 | PTC 100mA | Zwarciowa (**test — docelowo 500 mA**) | — |
| D5 | SMBJ33CA | Coarse TVS | LCSC |
| FB1, FB2 | BLM31PG601SN1L | Ferryt (podwójny — z doświadczeń burst) | TME |
| D7 | PMEG6030EP | Odwr. polaryzacja | LCSC |
| D6, D8 | SMBJ36A-13-F | Fine TVS (po obu stronach D7) | LCSC |
| C20 | 10u/50V | Bulk | — |
| C21 | 1n/50V | HF | — |

### 7.2 Ochrona CAN

```
J1.3/4 → GD2 (GDT 3-el. → Earth) → D9 (CDSOT23-T24CAN → GND) → FL1 (CMC) → R15/R16 (10R) → ATA6561
```

| RefDes | Wartość | Rola |
|--------|---------|------|
| GD2 | 3R090-5S (GDT 3-el.) | Coarse surge CAN→Earth |
| D9 | CDSOT23-T24CAN | Fine TVS dedykowany CAN |
| FL1 | ACT45B-510-2P-TL003 | CMC (TDK) |
| R15, R16 | 10R Pulse-proof | Szereg do transceivera |

### 7.3 Ekran / masa

| RefDes | Wartość | Rola |
|--------|---------|------|
| J1 | Conn M12 5-pin | 1=Vin, 2=GND, 3=CAN_H, 4=CAN_L, 5=Earth |
| C22 | 1nF/2kV | Shield tie Earth↔GND (Y-cap) |

Rozdzielenie `Earth` (obudowa, GDT) i `GND` (sygnałowa) — łączone tylko przez C22.

---

## 8. Budżet mocy (Vin=12V)

| Tryb | Pobór | Uwaga |
|------|-------|-------|
| Idle | ~200 µA | MCU Stop 2, ATA6561 Standby, flyback ~30 Hz, LT8606 2.5µA, MCP1700 1.6µA |
| Active | ~3.7 mA | Obsługa ramki CAN |
| Active + CAN TX | do ~15–20 mA @ 7V | Piki transmisji (uwzględnione w skali current-sense) |

---

## 9. Elementy — dostępność (trzy źródła)

| Źródło | Elementy |
|--------|----------|
| **Mouser/DigiKey** | LT8606IDC (buck) — jedyny |
| **LCSC** | Ochrona: GD1/GD2 3R090-5S, D5 SMBJ33CA, D9 CDSOT23-T24CAN, FL1 ACT45B-510, D7 PMEG6030EP, D6/D8 SMBJ36A, C22 1nF/2kV, F1 PTC |
| **TME** | ATA6561, STM32L432, MCP1700, MCP79410, STTH112, P18/11+B65652, BAV199, BAV99, BAV21WS, DMN3404L, GAQY212GS, BSS138, INA186, HVC1206, ferryty BLM31 |

**Do weryfikacji stanu TME:** INA186 (wariant A1/A4, z EN), LQH32PN6R8NN0L, HVC1206T5005JET.

---

## 10. Lista kontrolna przed layoutem (z przeglądu schematu)

- [ ] ERC: `GM_OUT`↔`COMP_GM_IN`, `V_zas`↔`V_IN` zwarte na root
- [ ] EN/UV LT8606 — próg ≤7 V (start od dolnego zakresu)
- [ ] C_split terminacji CAN = 4.7 **nF** (nie pF)
- [ ] Kropki fazowe T1 (flyback) — poprawna faza wtórnego
- [ ] Flyback + snubber zasilane z **+5V** (nie Vin)
- [ ] Klamra na VIN_SENSE (opcja)
- [ ] Ferryt VDD/VDDA (opcja)
- [ ] F1 PTC — po testach zmienić 100 mA → 500 mA
- [ ] Filtr RC na wyjściu INA186 (1k+100n)

---

## 11. Zasady layoutu (skrót)

- Strefy: HV (creepage ≥3–5 mm wokół +HV/ANODE), analog (dzielniki, INA186), cyfrowa (MCU, CAN)
- Pętla mocy flybacka minimalna; snubber tuż przy drenie Q1
- Trafo off-board (przewody 4–6 cm, skręcone)
- Star ground: GND_PWR/GND_HV/GND_sygnałowa w jednym punkcie
- Brudna masa EMC (TVS/GDT) → Earth/obudowa, oddzielona od masy cyfrowej
- Coarse (GDT) przy złączu, fine (TVS) przy obwodzie
- Shunt current-sense: krótkie, symetryczne ścieżki IN+/IN−; filtr RC blisko ADC

---

## 12. Roadmap

**Faza 1 — Projektowa** (w toku)
- [x] Schemat KiCad (6 arkuszy)
- [ ] ERC + poprawki z listy kontrolnej (§10)
- [ ] BOM z linkami (3 źródła)

**Faza 2 — Trafo:** zakup P18/11, nawinięcie 17:510, montaż śrubą, testy L/izolacja/hi-pot

**Faza 3 — PCB:** layout stref, produkcja JLCPCB, montaż sekcyjny

**Faza 4 — Firmware:** bring-up, LPTIM+COMP, TIM2+HV, bxCAN+wake, RTC/MSI cal, current-sense, protokół, persystencja

**Faza 5 — Kalibracja/EMC:** HV + CPS→µSv/h (Cs-137), testy ESD/Burst/Surge, CAN długa linia, dobór F1 PTC po pomiarze inrush

**Faza 6 — Mechanika:** obudowa Al, M12, IP67

---

## 13. Referencje

LT8606 (AD, Vref 0.778V, DFN Burst Mode) · MCP1700, MCP79410 (Microchip) · INA186 (TI, 40V, EN, zero-drift) · ATA6561 (Microchip) · STM32L432 RM0394 · P18/11 3F3 (Ferroxcube), B65652 (EPCOS) · STTH112, BAV199, BAV21WS, DMN3404L · GAQY212GS (Panasonic), BSS138 · CDSOT23-T24CAN (Bourns), 3R090-5S GDT, ACT45B-510 CMC (TDK) · CN-0536 (AD, detektor GM) · podręcznik metrologii promieniowania (układy DOI/BOI/GOI) · IEC 61000-4-2/-4/-5, EN 61326-1 · CAN ISO 11898, CANopen CiA 303-1 · M12 IEC 61076-2-101

---

*Koniec dokumentu v5.0 — zsynchronizowany ze schematem KiCad. Firmware i mapa rejestrów CAN w osobnym opracowaniu.*
