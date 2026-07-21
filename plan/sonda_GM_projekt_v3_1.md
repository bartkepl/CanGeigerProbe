# Sonda promieniowania z licznikiem G-M — dokumentacja projektowa

**Wersja:** 3.1 (finalna faza koncepcyjna, z analizą EMC i dostępności)
**Data:** 2026-07-02
**Autor:** Bartek
**Status:** Projekt w fazie schematu — gotowy do implementacji w KiCad

**Zmiany kluczowe vs v3.0:**
- Buck: MP2459 → **LT8606** (finalnie, zamawiany spoza TME — jedyny taki element)
- Rdzeń: → **P18/11-3F3 (Ferroxcube), ungapped** + karkas **B65652B0000T001** (EPCOS/TDK)
- Mocowanie trafa: klipsa → **śruba M2 A2 + podkładki nylonowe + zaklejenie**
- Druty: **0.30 mm** (pierwotne) i **0.10 mm** (wtórne)
- Referencja ADC: rozstrzygnięcie kwestii LM4040 — **VDDA z LDO + VREFINT** (LM4040 nie pasuje do UFQFPN32)
- CAN transceiver: **ATA6561** (zamiast TJA1042 — problem dostępności w TME)
- **Nowy rozdział 11: Zabezpieczenia EMC (ESD/Surge/Burst)** wg IEC 61000-4-2/-4/-5
- Lista elementów krytycznych niedostępnych w TME

---

## 1. Cel projektu i inspiracje

Sonda promieniowania jonizującego typu polowego, kompatybilna z wieloma typami miniaturowych lamp G-M — bazowo **DOI-80**, z wymiennością na lampy radzieckie (**SBM-20**, **STS-5**, **SBM-19**), chińskie (**J305**, **M4011**) i zachodnie (**LND712**).

Inspiracja: rozwiązania Polon-Alfa (**DPO-G**, **ZR-1**) — wąska sonda cylindryczna z jednym złączem, komunikacja cyfrowa z jednostką nadrzędną przez CAN 2.0B.

Wymagania: **micropower design**, praca polowa, zasilanie z długiej linii przemysłowej, komunikacja CAN 2.0B z wake-up on bus activity, **obudowa aluminiowa** (ekranowanie EMI).

---

## 2. Założenia projektowe

### 2.1 Założenia funkcjonalne

| Parametr | Wartość | Uwaga |
|----------|---------|-------|
| Detektor bazowy | DOI-80 (400V) | Miniatura szklana halogenowa |
| Detektory kompatybilne | SBM-20, STS-5, SBM-19, J305, M4011, LND712 | Zmiana programowa |
| Zakres HV | 350–550 V programowalnie | Ustawiany przez CAN |
| Zakres pomiarowy | CPS/CPM, dawka chwilowa, dawka skumulowana | + alarmy progowe |
| Interfejs | CAN 2.0B (extended ID 29-bit) | 125k / 250k / 500k / 1 Mbit |
| Zasilanie | Vin = 7–32 V DC | Z długiej linii, wspólna masa |
| Pobór typ. (idle) | ~200 µA @ 12V | CAN Standby, MCU Stop 2, LT8606 |
| Pobór typ. (aktywny) | ~3 mA @ 12V | Chwilowo, obsługa ramki CAN |
| Izolacja | Brak (wspólna masa) | Ochrona TVS/GDT |
| Ochrona EMC | IEC 61000-4-2/-4/-5 | ESD, Burst, Surge |
| Złącze | M12 5-pin, A-coded | Standard CAN przemysłowy |
| Obudowa | Aluminiowa cylindryczna | Ekranowanie EMI, IP67 |

### 2.2 Filozofia projektowa

1. **Micropower z wybudzaniem** — MCU w Stop 2, transceiver CAN w Standby, wake-up na aktywność bus
2. **Programowalny HV** — jedna sonda, wiele lamp; setpoint zmienialny przez CAN
3. **Dwustopniowe zasilanie** — buck do 5V dla mocy, LDO 5V→3.3V dla MCU
4. **Deterministyczna regulacja HV** — burst-by-burst z pętlą ADC
5. **Off-board transformator HV** — kubełek P18/11, nawinięty ręcznie, mocowany śrubą
6. **Blanking impulsów** w firmware podczas burst flybacka
7. **Persystencja** liczników w rotacyjnym buforze flash
8. **Ochrona EMC klasy przemysłowej** — odporność na ESD/Surge/Burst z długiej linii polowej
9. **Obudowa aluminiowa** — radiated emission drugorzędne, priorytet na conducted immunity

---

## 3. Kompatybilność lamp G-M

### 3.1 Tabela lamp

| Lampa | HV nom. | Plateau | R_anode | Uwagi |
|-------|---------|---------|---------|-------|
| **DOI-80** | 400 V | 380-430 V | 5-10 MΩ | Bazowa |
| SBM-20 | 400 V | 350-475 V | 4.7 MΩ | Klasyk radziecki |
| STS-5 | 400 V | 350-475 V | 4.7 MΩ | Drop-in SBM-20 |
| SBM-19 | 390 V | 350-450 V | 5-10 MΩ | Większa czułość |
| J305 | 380 V | 350-450 V | 4.7-10 MΩ | Chińska SBM-20 |
| M4011 | 380 V | 350-450 V | 4.7 MΩ | Chińska mała |
| LND712 | 500 V | 450-650 V | 10 MΩ | Zachodnia α+β+γ |

### 3.2 Zwora R_anode na PCB

Pole lutownicze 3-pozycyjne — lutujesz jeden rezystor pod używaną lampę:
```
+HV ──┬── R13a (4.7 MΩ) ──┬─── ANODE (SBM-20/STS-5/J305/M4011)
      ├── R13b (5.1 MΩ) ──┤    (DOI-80/SBM-19)
      └── R13c (10  MΩ) ──┘    (LND712)
```

### 3.3 Kalibracja czułości

`CAL_CPS_PER_USV`: DOI-80 ~0.5-1, SBM-20/STS-5 ~2.5, LND712 ~0.3-0.5, J305 ~2.0. Kalibracja ze wzorcem Cs-137.

---

## 4. Architektura systemu

```
┌────────────────────────────────────────────────────────────────────┐
│                          Sonda G-M (obudowa Al)                     │
│                                                                     │
│  ┌────────┐  ┌──────────────┐  ┌────────┐                          │
│  │M12 pin1│─▶│ Ochrona Vin  │─▶│ LT8606 │──▶ +5V rail              │
│  │Vin 7-32│  │ GDT+TVS+CLC  │  │  → 5V  │      │                   │
│  └────────┘  └──────────────┘  └────────┘      │                   │
│                                                 ├────────┐          │
│                                                 ▼        ▼          │
│                                        ┌──────────┐ ┌──────────┐   │
│                                        │LDO 3.3V  │ │CAN xcvr  │   │
│                                        │TPS7A0230 │ │ATA6561   │   │
│                                        │(dla MCU) │ │(VCC=5V)  │   │
│                                        └────┬─────┘ └───┬──────┘   │
│                                             ▼           ▼ CANH/L   │
│                                     ┌───────────────┐   → M12 3,4  │
│                                     │ STM32L432KCU6 │◀──STB──      │
│                                     │  Vdd=3.3V     │      ▲       │
│                                     │  VDDA=VREF+   │◀─RXD─┘       │
│                                     │               │  wake        │
│                                     │  bxCAN, LPTIM, COMP1, TIM2,  │
│                                     │  ADC (VREFINT calib), DAC    │
│                                     └──┬────────┬──────────────────┤
│                                        │        │                  │
│                                        │        ▼                  │
│                                        │  ┌─────────────┐          │
│                                        │  │ Flyback HV  │          │
│                                        │  │ Q1+T1 P18/11│          │
│                                        │  │ 5V→350-550V │          │
│                                        │  └──────┬──────┘          │
│                                        │         ▼                 │
│                                        │  ┌─────────────┐          │
│                                        │  │Dzielnik HV  │          │
│                                        │  │→ADC 100pF   │          │
│                                        │  └──────┬──────┘          │
│                                        │         ▼                 │
│                                        │  ┌─────────────┐          │
│                                        │  │ Lampa G-M   │          │
│                                        │  │ R_anode zw. │          │
│                                        └─▶│ shaper→COMP1│          │
│                                           └─────────────┘          │
│  M12 pin2 (GND) ────────────────────                               │
│  M12 pin5 (Earth) ──── obudowa Al + C 1nF/2kV do GND               │
└────────────────────────────────────────────────────────────────────┘
```

---

## 5. Szczegóły konstrukcji — zasilanie

### 5.1 Buck 7-32V → 5V: LT8606

**Wybór IC:** **LT8606** (Analog Devices) — **zamawiany spoza TME (jedyny taki element)**

Parametry:
- Vin: 3.0–42 V (Vin abs. max 42V — pełny margines dla przepięć w polu)
- Iout: 750 mA
- **Iq: 2.5 µA** w Burst Mode (najniższy z rozważanych — kluczowe dla micropower)
- Częstotliwość: 200 kHz – 2.2 MHz (konfigurowana R_T)
- Obudowa: MSOP-10E (z exposed pad)

**Konfiguracja pod 5V @ 2 MHz:**

| Ref | Wartość | Rola |
|-----|---------|------|
| U2 | LT8606EMSE | Buck controller |
| C4 | 10µF/50V X7R 1210 | Input bulk |
| C5 | 100nF/50V X7R 0603 | Input HF |
| L3 | **6.8µH** (patrz niżej) | Cewka główna |
| R_fb_H | 1M 1% 0603 | Dzielnik FB (Vout=5V) |
| R_fb_L | 232k 1% 0603 | Dzielnik FB |
| C_ff | 22pF NP0 0402 | Feedforward |
| C_OUT1, 2 | 22µF/16V X7R 1206 ×2 | Output caps |
| R_T | 24k9 1% 0603 | f_sw ~2 MHz |
| C_BST | 100nF/16V X7R 0402 | Bootstrap |
| R_uv_H, R_uv_L | 1M / 137k 0603 | UVLO ~6.5V start |

**Cewka L3 przy 2 MHz — obudowa 1008 możliwa:**
```
L_min = (Vin_max - Vout) × Vout / (Vin_max × f_sw × ΔI_L)
      = (42 - 5) × 5 / (42 × 2e6 × 0.2) = 11 µH → wybór 6.8µH z zapasem Burst
```
Dzięki 2 MHz można użyć małej cewki **Murata LQH3NPN6R8 (1008, 360mA Isat)** lub **Coilcraft XGL4020-682**. Uwaga: to power inductor (wirewound/shielded), NIE multilayer ferrite chip.

**Wyliczenie dzielnika FB (V_FB = 0.97V dla LT8606):**
```
Vout = 0.97 × (1 + R_H/R_L) = 0.97 × (1 + 1M/232k) = 0.97 × 5.31 = 5.15 V
Korekta: R_L = 240k → Vout = 4.99V ✓
```

### 5.2 LDO 5V → 3.3V dla MCU: TPS7A0230

| Ref | Wartość | Rola |
|-----|---------|------|
| U5 | TPS7A0230DBVR (SOT-23-5) | LDO, Iq 25 nA, dropout 60mV |
| C_LDO_in | 1µF/10V X7R 0603 | Wejście |
| C_LDO_out | 1µF/10V X7R 0603 | Wyjście |

Zasila **VDD i VDDA** MCU. Straty przy 1 mA: 1.7 mW — marginalne.

### 5.3 Referencja ADC — rozstrzygnięcie kwestii LM4040

**Analiza:** rozważano LM4040AIM3-3.0 jako zewnętrzną referencję dla VDDA. **Nie pasuje do UFQFPN32**, z następujących powodów:

1. **LM4040 to shunt reference** (równoległa, jak precyzyjna dioda Zenera) — potrzebuje rezystora szeregowego i "przepala" nadmiar prądu. **Nie jest LDO** — nie udźwignie zasilania VDDA MCU (który pobiera 2-5 mA impulsowo).
2. **W UFQFPN32 VREF+ jest wewnętrznie związane z VDDA** — nie ma osobnego pinu VREF+, do którego można podłączyć referencję. Napięcie referencyjne ADC = napięcie zasilania analogowego.

**Przyjęte rozwiązanie:**
- **VDDA zasilane z LDO TPS7A0230** (stabilne 3.3V)
- **VREFINT** (wewnętrzna referencja 1.212V, factory-calibrated, zapisana w ROM) używana do **programowej korekcji**
- Firmware mierzy VREFINT, wylicza rzeczywiste VDDA, koryguje pomiar HV
- **Dokładność: ~0.5-1%** — w zupełności wystarczy dla pomiaru HV

LM4040 miałby sens tylko w większej obudowie (LQFP48/64) z osobnym pinem VREF+. Dla UFQFPN32 odpada architektonicznie.

---

## 6. Szczegóły konstrukcji — HV i detektor

### 6.1 Transformator T1 — P18/11-3F3

**Rdzeń:** **P18/11-3F3** (Ferroxcube), ungapped, dostępny w TME.
**Karkas:** **B65652B0000T001** (EPCOS/TDK), 1-sekcyjny, PET, dostępny w TME.

| Parametr | Wartość |
|----------|---------|
| Materiał | 3F3 (100-500 kHz, MnZn) |
| Ae | 43.3 mm² |
| le | 25.8 mm |
| Ve | 1120 mm³ |
| AL ungapped | ~2850 nH/N² |
| **Szczelina własna** | **Kapton 100 µm między połówkami (symetryczna)** |
| **AL efektywne (g=0.1mm)** | **~270 nH/N²** |

**Uzwojenia:**

| Uzwojenie | Zwoje | Drut | Warstwy |
|-----------|-------|------|---------|
| Pierwotne (P1-P2) | **17 zw** | **0.30 mm em CuL** | 1 warstwa |
| Wtórne (S1-S2) | **510 zw** | **0.10 mm em CuL** | ~8 warstw, Kapton między |

**Parametry elektryczne:**
```
Lp = 75 µH  (17² × 270 nH/N²)
Ls = 67.5 mH
n = 1:30
I_peak (Vin=5V, t_on=7µs) = 467 mA
Energia na puls = 8.18 µJ
B_peak = 75e-6 × 0.467 / (17 × 43.3e-6) = 47.9 mT  (10× margines do Bs=440mT)
Burst rate typ. ~30 Hz dla 250 µW
Vds Q1 max = 5 + 550/30 + ringing ≈ 25V  (OK dla 30V MOSFET)
Vrev diody = 550 + 30×5 = 700V → potrzeba diody ≥1000V
```

**Mocowanie rdzenia — śruba M2 (rdzeń ma otwór ø3 mm):**

Rdzeń P18/11-3F3 ma centralny otwór ø3 mm (bez gwintu). Mocowanie:
```
        nakrętka M2 nylock A2
              + podkładka nylonowa
                   │
        ┌──────────┴──────────┐  ← górna połówka
        │  P18/11 + karkas    │
        └──────────┬──────────┘  ← dolna połówka
                   │
             podkładka nylonowa
                   │
            śruba M2×16 A2 (imbus DIN 912)
```

| Element | Specyfikacja | Uwaga |
|---------|-------------|-------|
| Śruba | M2 × 16 mm, **stal nierdzewna A2** (niemagnetyczna!) | DIN 912 imbus |
| Nakrętka | M2 nylock A2 | DIN 985, samohamowna |
| Podkładki | 2× M2 nylonowa | **Kluczowe — chronią kruchy ferryt** |
| Klej | Kropla epoksydu na gwincie po dostrojeniu | Zabezpieczenie ostateczne |

**KRYTYCZNE:**
- **Stal A2 (nierdzewna), NIGDY zwykła ocynkowana** — zwykła stal jest ferromagnetyczna i zaburzy pole magnetyczne rdzenia
- **Podkładki nylonowe obowiązkowe** — ferryt jest ceramiczny i kruchy, metal dokręcony bezpośrednio powoduje mikropęknięcia
- Moment dokręcania ~0.2-0.3 Nm (mocno palcami, bez forsowania)
- Docisk 20-60 N wg datasheetu TDK dla stabilnego AL
- Po dostrojeniu (kontrola Lp=75µH mostkiem LCR): **zaklejenie epoksydem** dla trwałości

**Procedura nawijania:**
1. Pierwotne: 17 zw 0.30 mm na dnie karkasu, 1 warstwa równomiernie
2. Izolacja: 2× Kapton 50 µm
3. Wtórne: 510 zw 0.10 mm, warstwami po ~60 zw, Kapton 25 µm między warstwami
4. Zewnętrznie: 2-3× Kapton
5. Szczelina: Kapton 100 µm w obu połówkach (środkowa nóżka + pierścień)
6. Montaż śrubą, kontrola Lp, zaklejenie
7. Testy: Lp≈75µH, Ls≈67mH, leakage<1.5µH, izolacja P/S >1GΩ, hi-pot 1kV/60s

**Długość drutu:**
- Pierwotne (17 zw, ALT~32mm): 17×32 = 544mm + zapas = **~75 cm**
- Wtórne (510 zw, ALT~35mm śr.): 510×35 = 17850mm + zapas = **~20 m**

### 6.2 Flyback — MOSFET, dioda, snubber

| Ref | Wartość | Rola |
|-----|---------|------|
| Q1 | DMN3404L (SOT-23) | MOSFET 30V, Rds 116mΩ, ID 4A |
| R7 | 22Ω 0402 | Gate resistor |
| R12 | 100k 0402 | Gate pull-down |
| C6 | 10µF/16V X7R 0805 | Bulk 5V flyback |
| C7 | 470pF/50V NP0 0603 | Snubber HF |
| D5 | SMAJ22A (SMA) | TVS clamp drain |
| **D6** | **STTH112 (SMA, 1200V)** | Dioda HV ultrafast — dostępna w TME |
| C8 | 2.2nF/1kV NP0 1210 | Filtr peak |
| C9, C9b | 100nF/1kV X7R 1210 ×2 | Reservoir HV |

### 6.3 Dzielnik HV — C=100pF + oversampling

```
+HV → R9(500MΩ) → R10(500MΩ) → R11(1MΩ) → HV_ADC → ADC1
                                     ↓ C10 100pF
                                    GND
```

| Ref | Wartość | Rola |
|-----|---------|------|
| R9, R10 | 500MΩ 1% Ohmite HVC1206 | 2× szereg = 1GΩ |
| R11 | 1M 0.1% 0805 | Bottom |
| C10 | **100pF NP0 0603** | Antyalias (τ=100ms), NIE 10nF |

Skala: HV/1001. Przy 550V → 549mV. Oversampling ADC 16× → ~14-bit efektywne, precyzja ~0.3%.

### 6.4 Detektor G-M i shaper

| Ref | Wartość | Rola |
|-----|---------|------|
| V1 | Lampa G-M | Wymienna |
| R13a/b/c | 4.7M / 5.1M / 10M HV 1206 | Zwora (jeden lutowany) |
| C11 | 22pF/1kV NP0 1206 | Sprzęg AC |
| R14 | 1M 0603 | Limit prądu |
| R_pu | 1M 0603 | Bias do Vcc/2 |
| D7, D8 | BAV99 (SOT-23) | Klamry |
| C12 | 10pF NP0 0402 | Filtr HF (opcj.) |

Próg COMP1 przez DAC1_OUT1 (wewn.), ~1.0-1.2V.

### 6.5 MCU — STM32L432KCU6

UFQFPN32, bxCAN, Stop 2 ~1.4µA. VDD+VDDA z LDO. VREF+ wewn. związane z VDDA.

Mapowanie: PA1=COMP1_INP, PA4=DAC1, PA5=ADC(HV), PA6=ADC(Vin), PA7=STB, PA11=CAN_RX, PA12=CAN_TX, PA15=EXTI wake, PB3=COMP1_OUT→LPTIM1, PB4=EN_HV, PB10=TIM2_CH3(GATE).

---

## 7. Transceiver CAN — ATA6561 (zmiana z TJA1042)

**Problem dostępności:** TJA1042T/3 w TME jest tylko "na zamówienie specjalne" (0 na stanie, multiplicity 2500). **Zamiennik: ATA6561 (Microchip)** — dostępny w TME, funkcjonalnie równoważny.

**ATA6561-GAQW-N** (SO-8) lub **ATA6561-GBQW-N** (VDFN8):
- Zgodny ISO 11898-2/-5, CAN FD ready 5 Mbit/s
- VCC = 5V, **VIO = 3.3V** (bezpośrednie interfejsowanie z MCU)
- Standby mode z wake-up
- Pin 5 = STBY (nie SPLIT/VIO jak w TJA) — **uwaga: inny pinout niż TJA1042!**

**Pinout ATA6561:**
```
Pin 1: TXD  ← MCU PA12 (CAN_TX)
Pin 2: GND
Pin 3: VCC  ← +5V
Pin 4: RXD  → MCU PA11 (CAN_RX) + PA15 (EXTI wake)
Pin 5: VIO  ← +3.3V (poziom logiczny)
Pin 6: CANL ↔ magistrala
Pin 7: CANH ↔ magistrala
Pin 8: STBY ← MCU PA7 (standby/normal)
```

**Uwaga:** ATA6561 ma STBY na pin 8 i VIO na pin 5 — układ podobny do TJA1042T/3, ale sprawdź dokładnie datasheet przy projektowaniu footprintu. ATA6561 nie ma trybu Silent (tylko Normal/Standby) — dla naszej aplikacji to wystarcza.

Pobór: Standby ~10 µA, Normal ~5 mA (chwilowo).

---

## 8. Kluczowe wyliczenia — budżet mocy

### 8.1 Tryb IDLE (Vin=12V)

| Blok | Pobór z 12V |
|------|-------------|
| STM32L432 Stop 2 + LPTIM + RTC | ~1.4 µA |
| Komparator wewnętrzny | ~0.7 µA |
| LDO TPS7A0230 Iq | ~15 nA |
| ATA6561 Standby | ~10 µA (z 5V) |
| Dzielnik HV (1GΩ @ 400V) | ~0.4 µA |
| Flyback burst (~30 Hz) | ~35 µA |
| Iq bucka LT8606 | ~2.5 µA |
| **RAZEM IDLE** | **~50-200 µA** |

Przy LT8606 (Iq 2.5µA) budżet idle jest **najlepszy ze wszystkich wersji** — ~2.4 mW @ 12V. Dominuje flyback i ATA6561 standby.

### 8.2 Tryb ACTIVE

| Blok | Pobór z 12V |
|------|-------------|
| STM32L432 Run @ 16 MHz | ~1.5 mA |
| ATA6561 Normal | ~2 mA |
| Reszta | ~250 µA |
| **RAZEM** | **~3.7 mA** |

Średnio (1 ramka/s, 20ms): ~250 µA.

### 8.3 Porównanie wersji

| Wersja | Buck | Iq buck | Idle @12V |
|--------|------|---------|-----------|
| v3.0 | MP2459 | 190 µA | ~240 µA |
| **v3.1** | **LT8606** | **2.5 µA** | **~50-200 µA** |

LT8606 poprawia idle o ~40-190 µA względem MP2459. Koszt: zamawianie spoza TME.

---

## 9. Mapa rejestrów CAN

Protokół własny, CAN 2.0B extended ID (29-bit): `[MFR=0xD0][NODE_ID][FUNC 5b][REG_ADDR 8b]`.

Kody FUNC: 0x01 READ, 0x02 READ_RESP, 0x03 WRITE, 0x04 WRITE_ACK, 0x06 ALARM, 0x07 PING, 0x08 PING_RESP, 0x09 HEARTBEAT.

Kluczowe rejestry (pełna mapa w Appendix A):

| ADR | Nazwa | Typ | RW |
|-----|-------|-----|----|
| 0x0000 | DEVICE_ID (0xD080) | U16 | R |
| 0x0005 | STATUS | U16 | R |
| 0x0010 | CPS | U16 | R |
| 0x0013 | CPM | U32 | R |
| 0x0017 | DOSE_RATE_NSVH | U32 | R |
| 0x0019 | DOSE_ACCUM_NSV | U32 | R |
| 0x0020 | HV_MEASURED | U16 | R |
| 0x0021 | HV_SETPOINT | U16 | RW |
| 0x0031 | CAL_CPS_PER_USV | U16 | RW |
| 0x0037 | TUBE_TYPE | U16 | RW |
| 0x0045 | CAN_BITRATE | U16 | RW |
| 0x0046 | CAN_NODE_ID | U16 | RW |

---

## 10. Architektura firmware

Event-driven, Stop 2, wybudzanie z RTC (1s) i EXTI15 (CAN wake). Regulacja HV bang-bang z adaptive gain. Blanking komparatora 50µs podczas burst. Persystencja w rotacyjnym buforze flash (8×64B, ~10 lat). Szczegóły jak w v3.0 (bez zmian).

---

## 11. Zabezpieczenia EMC — ESD / Surge / Burst (NOWY ROZDZIAŁ)

### 11.1 Normy i poziomy docelowe

Sonda polowa na długiej linii przemysłowej musi przejść testy odporności wg **EN 61326-1** (aparatura pomiarowa) lub **EN 61000-6-2** (odporność, środowisko przemysłowe). Kluczowe testy:

| Norma | Test | Poziom docelowy (przemysł) | Charakterystyka |
|-------|------|---------------------------|-----------------|
| **IEC 61000-4-2** | ESD | ±8 kV kontakt, ±15 kV powietrze (L4) | 1 ns rise, ~30 ns, niska energia |
| **IEC 61000-4-4** | EFT/Burst | ±2 kV (zasilanie), ±1 kV (sygnał) (L3-4) | 5/50 ns, pakiety 15ms/300ms |
| **IEC 61000-4-5** | Surge | ±1 kV (linia-linia), ±2 kV (linia-ziemia) (L2-3) | 1.2/50 µs, **wysoka energia** |

**Ważne rozróżnienie energii:**
- ESD i EFT: krótkie, niska energia — TVS radzą sobie same
- **Surge: 3-4 rzędy wielkości więcej energii** — wymaga elementów wysokoenergetycznych (GDT, większe TVS, MOV)

### 11.2 Filozofia ochrony wielostopniowej

Ponieważ obudowa jest **aluminiowa** (radiated emission drugorzędne), priorytet to **conducted immunity**. Strategia kaskadowa "coarse → fine":

```
Linia → [Stopień 1: GDT/duży TVS] → [impedancja: ferryt/R] → [Stopień 2: TVS precyzyjny] → obwód
         (coarse, wysoka energia)      (rozprzęgnięcie)         (fine, niska clamp)
```

Zasada: pierwszy stopień przejmuje energię (surge), drugi ustala niskie napięcie clamp (chroni krzem). Element pośredni (ferryt/rezystor) wytwarza spadek napięcia który pozwala pierwszemu stopniowi zadziałać zanim drugi się przeciąży.

### 11.3 Ochrona wejścia zasilania (Vin, pin 1)

Kaskada od złącza:

```
M12.1 → [GDT] → [F1 PTC] → [D1 TVS coarse] → [L1 ferryt] → [D2 Schottky] → [D3 TVS fine] → [C bulk] → LT8606
   │                                                                                                        
  GND ←─[GDT]                                                                                              
```

| Ref | Wartość | Rola | Norma |
|-----|---------|------|-------|
| **GDT1** | Bourns 2-el. GDT 90V (np. 2038-15-SM-RPLF) | Coarse surge, linia-ziemia | 4-5 |
| F1 | PTC 0.5A 60V (MF-MSMF050) | Ochrona zwarciowa | — |
| D1 | **SMBJ33CA** (bidirectional) | Coarse TVS, surge | 4-5 |
| L1 | BLM31PG601 (ferryt 600Ω) lub 10µH | Rozprzęgnięcie, EFT | 4-4 |
| D2 | PMEG6030EP | Odwr. polaryzacja | — |
| D3 | **SMBJ36A** | Fine TVS clamp | 4-2 |
| C_bulk | 10µF/50V X7R + 100nF | Absorpcja EFT resztek | 4-4 |

**Dobór surge:** dla ±2 kV linia-ziemia z rezystancją źródła 12Ω (IEC 61000-4-5), prąd szczytowy ~167 A. GDT 90V przejmuje większość energii (Ipp GDT >1 kA), SMBJ33 (Ppk 600W, ~18A @ 8/20µs) obsługuje resztki po ferrycie. Ferryt/rezystor szeregowy jest kluczowy — bez niego TVS SMBJ przeciąży się.

**Uwaga o GDT:** GDT ma czas zadziałania ~µs (wolniejszy niż TVS ns), dlatego para GDT (coarse, wolny, wysokoenergetyczny) + TVS (fine, szybki) jest komplementarna. GDT sam nie chroni przed ESD/EFT (za wolny), TVS sam nie przetrwa dużego surge (za mała energia).

### 11.4 Ochrona interfejsu CAN (CAN_H, CAN_L, piny 3-4)

```
M12.3 (CANH) → [GDT lub CDSOT23-T24CAN] → [L2 CMC] → [R 10Ω opcj.] → ATA6561.CANH
M12.4 (CANL) → [                        ] → [       ] → [           ] → ATA6561.CANL
```

**Rekomendowany TVS: CDSOT23-T24CAN (Bourns)** — dedykowany dla CAN, **dostępny w TME (1375 szt, ~0.35 USD)**.

| Parametr CDSOT23-T24CAN | Wartość |
|------------------------|---------|
| Working voltage | 24 V (obejmuje CAN + margines miswiring 24V) |
| Breakdown | 26.2-32 V |
| Clamping | 40 V @ Ipp |
| Ipp (8/20µs) | 8 A |
| Zgodność | IEC 61000-4-2 (±30kV), 61000-4-4, 61000-4-5 (500V) |
| Pojemność | Niska (kompatybilna z 1 Mbit CAN) |

| Ref | Wartość | Rola | Norma |
|-----|---------|------|-------|
| **D4** | **CDSOT23-T24CAN** | TVS dedykowany CAN | 4-2, 4-4, 4-5 |
| L2 | Würth 744232091 (CMC 51µH-100µH) | Common-mode choke, EFT/CM | 4-4 |
| R1, R2 | 10Ω 0805 (opcjonalne) | Pulse-proof, ogranicza prąd do xcvr | 4-5 |
| GDT2 (opcj.) | GDT 90V linia-ziemia | Coarse surge dla ostrych warunków | 4-5 |

**Dla surowszych warunków (surge >500V):** dodać GDT linia-ziemia przed CDSOT23-T24CAN. Wtedy: GDT (coarse) → CMC (impedancja) → CDSOT23 (fine). Sam ATA6561 ma wbudowaną ochronę ±8kV IEC ESD i ±58V bus fault, ale surge wymaga zewnętrznych elementów.

**Terminacja i split:** 120Ω terminacja (przez JP1) na skrajnych węzłach. Split termination (2×60Ω + C 4.7nF do GND) poprawia CM immunity — zalecane dla sondy końcowej.

### 11.5 Ochrona Earth/Shield (pin 5)

```
M12.5 (Earth) → obudowa aluminiowa (bezpośrednio)
              → C_shield 1nF/2kV → GND_sygnałowa (AC shield tie)
              → GDT3 (opcj.) → GND (drenaż ESD do obudowy)
```

| Ref | Wartość | Rola |
|-----|---------|------|
| C_shield | 1nF/2kV ceramic (Y-cap class) | AC coupling shield↔GND, unika pętli ziemi |
| GDT3 (opcj.) | GDT 90V | Drenaż ESD obudowa↔GND |

**Zasada:** obudowa aluminiowa połączona z pin 5 (Earth), a masa sygnałowa tylko przez kondensator (nie DC) — unika pętli ziemi między sondą a masterem, ale zapewnia drogę AC dla zakłóceń CM i ESD.

### 11.6 Uwagi layoutowe dla EMC

1. **Kolejność elementów ochronnych** — coarse (GDT) najbliżej złącza, fine (TVS) najbliżej chronionego obwodu. Nigdy odwrotnie.
2. **Masa ochronna** — TVS i GDT do dedykowanej "brudnej masy" połączonej z obudową w jednym punkcie, oddzielonej od masy cyfrowej.
3. **Krótkie ścieżki do TVS** — indukcyjność ścieżki dodaje się do clamp voltage (L×di/dt). Ścieżki do TVS masywne i krótkie.
4. **Pętla surge poza obszarem cyfrowym** — prąd surge nie może płynąć przez masę cyfrową ani analogową.
5. **Guard ring wokół sekcji wejściowej** — pierścień masy połączony z obudową.
6. **Obudowa aluminiowa** — połączona z Earth (pin 5) i "brudną masą" w jednym punkcie. Ekran dla radiated (choć drugorzędny) i droga powrotu dla ESD.
7. **Conducted emission** (priorytet): filtr LC na wejściu Vin (ferryt L1 + C) tłumi zakłócenia z flybacka i bucka wracające do linii. LT8606 @ 2MHz i flyback burst generują zakłócenia — filtr wejściowy je zatrzymuje.

### 11.7 Tłumienie conducted emission (priorytet przy obudowie Al)

Ponieważ radiated jest ekranowane obudową, ale conducted wraca linią zasilającą:

```
LT8606 SW node ─── (źródło zakłóceń 2 MHz)
Flyback Q1 ────── (źródło zakłóceń burst)
         ↓
    [C bulk lokalny] ── tłumi u źródła
         ↓
    [L1 ferryt 600Ω] ── blokuje propagację do linii
         ↓
    [C-X 4.7µF] ── bocznik do masy
         ↓
    M12 pin 1 (czyste Vin)
```

Elementy: ferryt L1 (już w torze Vin) + kondensatory X (differential-mode) 4.7µF + ewentualnie mały CMC na Vin/GND jeśli conducted CM jest problemem. Dla flybacka: pętla pierwotna Q1-T1-C6 musi być fizycznie mała (minimalizacja powierzchni pętli = mniej emisji).

---

## 12. Konstrukcja mechaniczna

### 12.1 Złącze M12 5-pin A-coded

Pinout (zgodny CANopen CiA 303-1):
```
Pin 1: +Vin (7-32V)
Pin 2: GND
Pin 3: CAN_H
Pin 4: CAN_L
Pin 5: Shield / Earth / obudowa
```

Panel-mount żeński: Binder 09-3441-77-05, TE, Amphenol. IP67.

### 12.2 Obudowa aluminiowa

- Cylindryczna, Ø wewn. 25 mm (mieści trafo P18/11 ~Ø18 mm z ~3.5mm marginesu)
- Połączona z Earth (pin 5) — ekran EMI + droga ESD
- Uszczelnienie IP67

---

## 13. Lista elementów KRYTYCZNYCH niedostępnych w TME

Podczas weryfikacji dostępności zidentyfikowano następujące elementy krytyczne wymagające zamówienia spoza TME lub zamiany:

| Element | Status TME | Rozwiązanie |
|---------|-----------|-------------|
| **LT8606EMSE** | Niedostępny | **Zamówić z Mouser/Digi-Key/LCSC** (świadomy wybór — jedyny taki element) |
| **TJA1042T/3** | Tylko special order (MOQ 2500) | **Zamiana na ATA6561-GAQW-N** (dostępny w TME) |
| **Ohmite HVC1206 500MΩ** | Do weryfikacji | Alt: Vishay CRHV1206, Panasonic ERA — sprawdzić stan |
| **STTH112** | **Dostępny w TME** ✓ | OK |
| **CDSOT23-T24CAN** | **Dostępny w TME (1375szt)** ✓ | OK |
| **P18/11-3F3 + B65652B0000T001** | **Dostępne w TME** ✓ | OK |
| **TPS7A0230DBVR** | Do weryfikacji | Alt: MCP1700-3302 (Iq 1.6µA, gorszy ale dostępny) |

**Elementy do weryfikacji stanu magazynowego przed zamówieniem** (mogą wymagać zamiany):
- STM32L432KCU6 — zwykle dostępny, ale sprawdzić
- DMN3404L — popularny, ale zweryfikować
- Rezystory HV 500MΩ (Ohmite/Vishay) — często długi lead time

---

## 14. Kompletna lista BOM v3.1

### 14.1 Zasilanie wejściowe i ochrona EMC

| Ref | Wartość | Obudowa | Uwagi |
|-----|---------|---------|-------|
| GDT1 | Bourns 2038-15-SM 90V | SMD | Coarse surge Vin |
| F1 | PTC 0.5A 60V | 1812 | MF-MSMF050 |
| D1 | SMBJ33CA | SMB | Coarse TVS bidirectional |
| L1 | BLM31PG601SN1L | 1206 | Ferryt 600Ω |
| D2 | PMEG6030EP | SOD-128 | Schottky odwr.polar. |
| D3 | SMBJ36A | SMB | Fine TVS |
| C1 | 100nF/100V X7R | 0805 | HF |
| C2, C3 | 4.7µF/50V X7R | 1210 | Bulk ×2 |

### 14.2 Buck LT8606 (spoza TME)

| Ref | Wartość | Uwagi |
|-----|---------|-------|
| U2 | **LT8606EMSE** | MSOP-10E, **z Mouser/DigiKey** |
| C4 | 10µF/50V X7R 1210 | Input |
| C5 | 100nF/50V X7R 0603 | Input HF |
| L3 | Murata LQH3NPN6R8 (6.8µH, 1008) | Power inductor |
| R_fb_H | 1M 1% 0603 | FB |
| R_fb_L | 240k 1% 0603 | FB |
| C_ff | 22pF NP0 0402 | Feedforward |
| R_T | 24k9 1% 0603 | 2 MHz |
| C_BST | 100nF/16V X7R 0402 | Bootstrap |
| R_uv_H/L | 1M / 137k 0603 | UVLO ~6.5V |
| C_OUT1,2 | 22µF/16V X7R 1206 | ×2 |

### 14.3 LDO 3.3V

| Ref | Wartość | Uwagi |
|-----|---------|-------|
| U5 | TPS7A0230DBVR | SOT-23-5, Iq 25nA (alt: MCP1700-3302) |
| C_LDO_in/out | 1µF/10V X7R 0603 | ×2 |

### 14.4 CAN — ochrona i transceiver

| Ref | Wartość | Uwagi |
|-----|---------|-------|
| U1 | **ATA6561-GAQW-N** | SO-8, dostępny TME (zamiast TJA1042) |
| D4 | **CDSOT23-T24CAN** | TVS CAN, dostępny TME |
| L2 | Würth 744232091 | CMC dla CAN |
| GDT2 (opcj.) | GDT 90V | Coarse surge CAN |
| R1, R2 | 10Ω 0805 | Pulse-proof (opcj.) |
| C_CAN | 100nF/50V X7R 0402 | VCC decoupling |
| C_VIO | 100nF/16V X7R 0402 | VIO decoupling |
| R5 + JP1 | 120Ω 0805 + jumper | Terminacja |
| C_split1,2 | 4.7nF/50V X7R 0805 | Split term (opcj.) |

### 14.5 MCU

| Ref | Wartość | Uwagi |
|-----|---------|-------|
| U3 | STM32L432KCU6 | UFQFPN32, bxCAN |
| C13-C17 | 100nF/16V X7R 0402 | Decoupling |
| C16_bulk | 10µF/10V X7R 0805 | VDD bulk |
| L4 | BLM18PG471SN1D 0603 | Ferryt VDDA |
| R_NRST | 10k 0603 | Reset |
| C_NRST | 100nF 0402 | Reset filter |
| R_BOOT | 10k 0603 | BOOT0 |
| J2 | Header 1x6 | SWD |

### 14.6 Flyback HV

| Ref | Wartość | Uwagi |
|-----|---------|-------|
| T1 | Trafo P18/11 własny | 17:510 zw, 3F3 |
| Q1 | DMN3404L | SOT-23, 30V |
| R7 | 22Ω 0402 | Gate |
| R12 | 100k 0402 | Gate pull-down |
| C6 | 10µF/16V X7R 0805 | Bulk 5V |
| C7 | 470pF/50V NP0 0603 | Snubber |
| D5 | SMAJ22A SMA | TVS drain |
| D6 | **STTH112 SMA** | Dioda HV 1200V (TME) |
| C8 | 2.2nF/1kV NP0 1210 | Peak |
| C9, C9b | 100nF/1kV X7R 1210 | Reservoir ×2 |

### 14.7 Dzielnik HV + detektor

| Ref | Wartość | Uwagi |
|-----|---------|-------|
| R9, R10 | 500MΩ 1% Ohmite HVC1206 | ×2 szereg |
| R11 | 1M 0.1% 0805 | Bottom |
| C10 | 100pF NP0 0603 | Antyalias |
| V1 | Lampa G-M | Wymienna |
| R13a/b/c | 4.7M/5.1M/10M HV 1206 | Zwora |
| C11 | 22pF/1kV NP0 1206 | Sprzęg |
| R14, R_pu | 1M 0603 | ×2 |
| D7, D8 | BAV99 SOT-23 | Klamry |
| C12 | 10pF NP0 0402 | Filtr |

### 14.8 Złącza i trafo — mechanika

| Ref | Wartość | Uwagi |
|-----|---------|-------|
| J1 | M12 5-pin A-coded panel | Binder 09-3441-77-05 |
| J2 | Header 1x6 SWD | |
| C_shield | 1nF/2kV Y-cap | Shield tie |
| Rdzeń | **P18/11-3F3** | Ferroxcube, TME |
| Karkas | **B65652B0000T001** | EPCOS/TDK, TME |
| Śruba | M2×16 A2 DIN 912 | Nierdzewna niemagnetyczna |
| Nakrętka | M2 nylock A2 DIN 985 | |
| Podkładki | 2× M2 nylon | Chronią ferryt |
| Drut 0.30mm em CuL kl.2 | rolka 25g | ~1m potrzeba |
| Drut 0.10mm em CuL kl.2 | rolka 25g | ~20m potrzeba |
| Kapton 25µm | rolka 6mm×33m | Izolacja warstwowa |
| Kapton 100µm | wycinek | Szczelina |
| Epoksyd | kropla | Zaklejenie po dostrojeniu |

**Szacunkowy koszt v3.1:** ~160-200 zł (w tym LT8606 ~25 zł spoza TME, GDT ~5 zł, reszta TME).

---

## 15. Roadmap prac

**Faza 1 — Projektowa** ✅ (obecna)
- [x] Wybór topologii, LT8606, P18/11, ATA6561, EMC
- [x] Rozdział zabezpieczeń EMC
- [x] Weryfikacja dostępności TME
- [ ] Schemat KiCad
- [ ] BOM z linkami (TME + Mouser dla LT8606)

**Faza 2 — Prototyp trafa**
- [ ] Zakup P18/11-3F3 + B65652 + śruba M2 A2 + druty + Kapton
- [ ] Nawinięcie 17:510, szczelina Kapton 100µm
- [ ] Montaż śrubą, kontrola Lp=75µH, zaklejenie
- [ ] Testy L, izolacja, leakage, hi-pot

**Faza 3 — PCB**
- [ ] Layout ze strefami EMC (coarse/fine, brudna masa)
- [ ] Strefowanie HV/analog/cyfrowe
- [ ] Produkcja + montaż sekcyjny

**Faza 4 — Firmware**
- [ ] Bring-up, LPTIM+COMP, TIM2+HV, bxCAN, wake-up, protokół, persystencja, alarmy

**Faza 5 — Kalibracja i EMC**
- [ ] Kalibracja HV + CPS→µSv/h ze wzorcem
- [ ] **Testy EMC: ESD, Burst, Surge** wg IEC 61000-4-2/-4/-5
- [ ] Testy CAN na długiej linii

**Faza 6 — Mechanika**
- [ ] Obudowa Al, montaż M12, IP67

---

## 16. Referencje techniczne

- Ferroxcube: P18/11 3F3, karkas B65652
- STM32L432: RM0394, DS11453
- LT8606: Analog Devices datasheet
- ATA6561: Microchip DS20005991
- TPS7A0230: TI SBVS314
- STTH112: ST datasheet
- CDSOT23-T24CAN: Bourns datasheet
- **IEC 61000-4-2** (ESD), **-4-4** (EFT/Burst), **-4-5** (Surge)
- **EN 61326-1** (EMC aparatury pomiarowej), **EN 61000-6-2** (odporność przemysłowa)
- TI TIDUB36 (IEC ESD/EFT/Surge CAN protection design)
- Bourns AN CANbus surge protection
- CAN 2.0B: ISO 11898-1/-2, CANopen CiA 301/303-1

---

## Appendix A — Pełna mapa rejestrów CAN

| ADR | Nazwa | Typ | RW | Opis |
|-----|-------|-----|----|----- |
| 0x0000 | DEVICE_ID | U16 | R | 0xD080 |
| 0x0001 | FW_VERSION | U16 | R | major<<8\|minor |
| 0x0002 | HW_VERSION | U16 | R | |
| 0x0003-04 | UPTIME_S | U32 | R | |
| 0x0005 | STATUS | U16 | R | bit0=HV_OK...bit7=DOSE_ALARM |
| 0x0010 | CPS | U16 | R | |
| 0x0011 | CPS_AVG_10S | U16 | R | |
| 0x0012 | CPS_AVG_60S | U16 | R | |
| 0x0013-14 | CPM | U32 | R | |
| 0x0015-16 | TOTAL_COUNTS | U32 | R | |
| 0x0017-18 | DOSE_RATE_NSVH | U32 | R | nSv/h |
| 0x0019-1A | DOSE_ACCUM_NSV | U32 | R | nSv |
| 0x001B-1C | DOSE_ACCUM_HI | U32 | R | |
| 0x0020 | HV_MEASURED | U16 | R | ×10 V |
| 0x0021 | HV_SETPOINT | U16 | RW | ×10 V, 3500-5500 |
| 0x0022 | HV_BURST_RATE | U16 | R | Hz |
| 0x0023 | VIN_MV | U16 | R | mV |
| 0x0024 | TEMP_C | I16 | R | ×10 °C |
| 0x0030 | DEAD_TIME_US | U16 | RW | µs |
| 0x0031 | CAL_CPS_PER_USV | U16 | RW | |
| 0x0032-33 | ALARM_DOSE_RATE | U32 | RW | nSv/h |
| 0x0034-35 | ALARM_DOSE_ACCUM | U32 | RW | nSv |
| 0x0036 | HV_RIPPLE_LIMIT | U16 | RW | ×10 V |
| 0x0037 | TUBE_TYPE | U16 | RW | 0=DOI-80...6=LND712 |
| 0x0038 | R_ANODE_MOHM | U16 | R | ×10 |
| 0x0043 | SAVE_CONFIG | U16 | W | 0x5A5A |
| 0x0044 | RESET | U16 | W | 0xDEAD |
| 0x0045 | CAN_BITRATE | U16 | RW | 0=125k...3=1M |
| 0x0046 | CAN_NODE_ID | U16 | RW | 1-247 |
| 0x0047 | CAN_HEARTBEAT_MS | U16 | RW | 0=off |
| 0x0048 | CAN_ERROR_COUNTER | U16 | R | REC/TEC |
| 0x0049 | CAN_STATE | U16 | R | 0=Standby...3=BusOff |
| 0x0050 | DIAG_FLAGS | U16 | RW | Sticky |
| 0x0051 | HV_REG_ERRORS | U16 | R | |
| 0x0052 | RESET_COUNT | U16 | R | |
| 0x0053 | LAST_RESET_CAUSE | U16 | R | RCC->CSR |

---

*Koniec dokumentu v3.1*
