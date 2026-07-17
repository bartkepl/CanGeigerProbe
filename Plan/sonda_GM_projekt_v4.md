# Sonda promieniowania z licznikiem Geigera-Müllera — dokumentacja projektowa

**Wersja:** 4.0 (konsolidacja pełna — sprzęt gotowy do KiCad)
**Data:** 2026-07-04
**Autor:** Bartek
**Status:** Faza schematu — kompletny projekt sprzętowy przed layoutem PCB

---

## Spis treści

1. Cel projektu i inspiracje
2. Założenia projektowe
3. Kompatybilność lamp G-M i dobór rezystora gaszącego
4. Architektura systemu
5. Zasilanie: buck LT8606 + LDO
6. Referencja ADC
7. Generator HV — flyback sterowany z MCU
8. Transformator T1 (P18/11)
9. Strona pierwotna flybacka i snubber
10. Dzielnik HV i ochrona wejścia ADC
11. Detektor G-M i shaper impulsów (robust)
12. Transceiver CAN
13. Przypisanie pinów MCU
14. Zabezpieczenia EMC (ESD/Surge/Burst)
15. Budżet mocy
16. Konstrukcja mechaniczna i złącze M12
17. Elementy krytyczne i dostępność TME
18. Kompletna lista BOM
19. Roadmap prac
20. Referencje techniczne

---

## 1. Cel projektu i inspiracje

Sonda promieniowania jonizującego typu polowego, kompatybilna z wieloma typami miniaturowych lamp G-M. Bazowo **DOI-80**, z pełną wymiennością na lampy radzieckie (**SBM-20**, **STS-5**, **SBM-19**), chińskie (**J305**, **M4011**) i zachodnie (**LND712**).

Konstrukcja inspirowana sondami Polon-Alfa (**DPO-G**, **ZR-1**): wąska sonda cylindryczna z jednym złączem, komunikująca się cyfrowo z jednostką nadrzędną. Kluczowe wymagania:

- **Micropower** — pobór idle rzędu setek µA, zasilanie z długiej linii przemysłowej
- **Komunikacja CAN 2.0B** z wybudzaniem na aktywność magistrali (wake-up on bus)
- **Programowalne HV 350–550 V** — jedna sonda, wiele typów lamp
- **Obudowa aluminiowa** — ekranowanie EMI, priorytet na odporność conducted
- **Odporność EMC klasy przemysłowej** — ESD, Burst, Surge (IEC 61000-4-2/-4/-5)

---

## 2. Założenia projektowe

### 2.1 Parametry funkcjonalne

| Parametr | Wartość | Uwaga |
|----------|---------|-------|
| Detektor bazowy | DOI-80 (~400 V) | Miniatura szklana halogenowa |
| Detektory kompatybilne | SBM-20, STS-5, SBM-19, J305, M4011, LND712 | Zmiana programowa HV + zwora R_anode |
| Zakres HV | 350–550 V programowalnie | Ustawiany przez CAN, regulowany software'owo |
| Wielkości mierzone | CPS/CPM, moc dawki, dawka skumulowana | + alarmy progowe |
| Interfejs | CAN 2.0B (extended ID 29-bit) | 125k / 250k / 500k / 1 Mbit |
| Zasilanie | Vin = 7–32 V DC | Wspólna masa (bez izolacji galwanicznej) |
| Pobór idle | ~200 µA @ 12 V | CAN Standby, MCU Stop 2, flyback burst |
| Pobór aktywny | ~3.7 mA @ 12 V | Chwilowo, obsługa ramki CAN |
| Ochrona EMC | IEC 61000-4-2/-4/-5 | ESD ±8 kV, Burst ±2 kV, Surge ±2 kV |
| Złącze | M12 5-pin A-coded | Standard CANopen CiA 303-1 |
| Obudowa | Aluminiowa cylindryczna, Ø wewn. 25 mm | IP67 |

### 2.2 Filozofia projektowa

1. **Micropower z podwójnym wybudzaniem** — MCU w Stop 2 (~1.4 µA), transceiver CAN w Standby (~10 µA); wybudzanie z RTC (regulacja HV, liczniki) i z EXTI (aktywność CAN).
2. **Programowalny HV** — setpoint zmienny przez CAN bez zmian sprzętowych; zwora R_anode dobierana pod lampę.
3. **Dwustopniowe zasilanie** — buck Vin→5V zasila moc (flyback, CAN), LDO 5V→3.3V zasila tylko MCU.
4. **Flyback sterowany deterministycznie z MCU** — TIM2 generuje burst, ADC mierzy HV, firmware zamyka pętlę bang-bang. Brak dedykowanego kontrolera → micropower.
5. **Transformator off-board** — kubełek P18/11, nawinięty ręcznie, mocowany śrubą.
6. **Detektor robust** — dzielnik pojemnościowy o kontrolowanej amplitudzie, rezystor gaszący specyficzny dla lampy, ochrona wejścia komparatora.
7. **Ochrona EMC kaskadowa** — coarse (GDT/duży TVS) → fine (TVS precyzyjny), priorytet conducted immunity.

---

## 3. Kompatybilność lamp G-M i dobór rezystora gaszącego

### 3.1 Fizyka i sprostowanie napięć

Ważna korekta powszechnego nieporozumienia: **SBM-20 i STS-5 pracują przy ~400 V, NIE 490 V.** Ich plateau to 350–475 V, punkt pracy 400 V — są **drop-in kompatybilne z DOI-80**. Realnie dopiero LND712 wymaga ~500 V.

### 3.2 Tabela lamp i wartości katalogowe

Wartości R1 (rezystor gaszący/anodowy) i R2 (obciążenie) wg polskiego podręcznika metrologii promieniowania (autorytatywne źródło dla lamp DOI/BOI/GOI):

| Lampa | HV nom. | Plateau | R1 (quench) | R2 (obciążenie) | Układ |
|-------|---------|---------|-------------|-----------------|-------|
| BOI | ~400 V | — | 10 MΩ | 1 MΩ | a |
| DOI-30 | ~400 V | — | 2.2 MΩ | 47 kΩ | b |
| **DOI-80** | **~400 V** | **380–430 V** | **5.1 MΩ** | **100 kΩ** | **b** |
| DOI-50 | ~400 V | — | 2.2 MΩ | 56 kΩ (kompensowany, R1C1=R2C2) | c |
| GOI-20, BOH-20 | ~400 V | — | 10 MΩ | 220 kΩ | a |
| SBM-20 | 400 V | 350–475 V | 4.7 MΩ | — | anode readout |
| STS-5 | 400 V | 350–475 V | 4.7 MΩ | — | anode readout |
| LND712 | 500 V | 450–650 V | 10 MΩ | — | anode readout |

### 3.3 Rezystor gaszący — dlaczego jest krytyczny

R_anode to **rezystor gaszący (quench)**, nie dowolna wartość. Prosty pasywny układ gaszący to rezystor anodowy o dużej wartości w głównym obwodzie wyładowania — zwiększa stałą czasową prądu odtwarzania, przedłużając okres obniżonego napięcia, co wspomaga dejonizację i gasi wyładowanie. Skutki doboru:

- **Za mały R_anode** → lampa może nie gasić (ciągłe wyładowanie → degradacja/uszkodzenie)
- **Za duży R_anode** → długi czas martwy → niski maksymalny count rate

### 3.4 Zwora R_anode na PCB (R13)

Trzy pola lutownicze — lutujesz jeden rezystor wg lampy:

```
+HV ──┬── R13a (4.7 MΩ HV 1206) ──┬─── ANODA   (SBM-20/STS-5/J305/M4011)
      ├── R13b (5.1 MΩ HV 1206) ──┤            (DOI-80/SBM-19)
      └── R13c (10  MΩ HV 1206) ──┘            (LND712)
```

Rezystory: Vishay CRHV1206 lub Ohmite HVC1206 (working voltage ≥1 kV). Domyślnie **R13b = 5.1 MΩ** (DOI-80).

### 3.5 Kalibracja czułości

`CAL_CPS_PER_USV` (impulsy/s przy 1 µSv/h) dobierana empirycznie ze wzorcem Cs-137:

| Lampa | CPS/(µSv/h) |
|-------|-------------|
| DOI-80 | ~0.5–1 |
| SBM-20 / STS-5 | ~2.5 (150 CPM/µSv/h) |
| LND712 | ~0.3–0.5 |
| J305 | ~2.0 |

---

## 4. Architektura systemu

```
┌──────────────────────────────────────────────────────────────────────┐
│                        Sonda G-M (obudowa Al)                          │
│                                                                        │
│  ┌────────┐  ┌───────────────┐  ┌─────────┐                           │
│  │M12 pin1│─▶│ Ochrona Vin    │─▶│ LT8606  │──▶ +5V ──┬──────┐        │
│  │Vin 7-32│  │ GDT+TVS+ferryt │  │ buck→5V │          │      │        │
│  └────────┘  └───────────────┘  └─────────┘          ▼      ▼        │
│                                              ┌──────────┐ ┌─────────┐  │
│                                              │LDO 3.3V  │ │ATA6561  │  │
│                                              │TPS7A0230 │ │CAN xcvr │  │
│                                              └────┬─────┘ └───┬─────┘  │
│                                                   ▼           ▼CANH/L  │
│                                          ┌────────────────┐  M12 3,4  │
│                                          │ STM32L432KCU6  │◀─STB──     │
│                                          │  VDD/VDDA=3.3V │◀─RXD wake─ │
│                                          │  bxCAN, LPTIM, │            │
│                                          │  COMP1, TIM2,  │            │
│                                          │  ADC, DAC1     │            │
│                                          └──┬──────┬──────┘            │
│                                   TIM2→GATE │      │ ADC←HV            │
│                                             ▼      │                   │
│                                     ┌────────────┐ │                   │
│                                     │ Flyback    │ │                   │
│                                     │ Q1+snubber │ │                   │
│                                     │ T1 P18/11  │ │                   │
│                                     │ 5V→350-550V│ │                   │
│                                     └─────┬──────┘ │                   │
│                                       +HV ▼        │                   │
│                                     ┌────────────┐ │                   │
│                                     │Dzielnik HV │─┘ (100pF+BAV199)   │
│                                     │ 1GΩ:1MΩ    │                    │
│                                     └─────┬──────┘                    │
│                                       +HV ▼                            │
│                                     ┌────────────┐                     │
│                                     │Lampa G-M   │                     │
│                                     │R13 quench  │                     │
│                                     │+ dziel.poj.│──▶ COMP1 ──▶ LPTIM │
│                                     └────────────┘                     │
│  M12 pin2 GND ────────                                                 │
│  M12 pin5 Earth ──── obudowa Al + C 1nF/2kV do GND                     │
└──────────────────────────────────────────────────────────────────────┘
```

Kluczowa cecha: **wspólna masa** (GND_PWR, GND_HV, GND_sygnałowa łączone w star ground). Brak izolacji galwanicznej — ochronę zapewniają TVS/GDT.

---

## 5. Zasilanie: buck LT8606 + LDO

### 5.1 Topologia dwustopniowa

```
Vin (7-32V) → LT8606 buck → +5V ─┬─→ flyback (Q1, T1)
                                  ├─→ ATA6561 (VCC=5V)
                                  └─→ TPS7A0230 LDO → +3.3V → MCU (VDD, VDDA)
```

**Uzasadnienie:** buck do 5V ma łatwiejszy stosunek konwersji niż do 3.3V (lepsza sprawność); flyback z 5V ma 2.3× więcej energii na puls niż z 3.3V; CAN transceiver ma natywne VCC=5V (bez dedykowanego LDO 5V); MCU dostaje czyste 3.3V z low-Iq LDO.

### 5.2 Buck: LT8606 (jedyny element spoza TME)

**LT8606EMSE** (Analog Devices), MSOP-10E. Zamawiany z Mouser/DigiKey/LCSC — jedyny komponent spoza TME, wybór świadomy dla najlepszego micropower.

| Parametr | Wartość | Znaczenie |
|----------|---------|-----------|
| Vin | 3.0–42 V | Margines dla przepięć w polu (Vin abs. 42V) |
| Iout | 750 mA | Nadmiarowo |
| **Iq (Burst Mode)** | **2.5 µA** | Najniższy z rozważanych — klucz micropower |
| f_sw | 200 kHz–2.2 MHz | Konfigurowana R_T |
| Obudowa | MSOP-10E | Z exposed pad |

**Konfiguracja pod 5V @ 2 MHz:**

| Ref | Wartość | Rola |
|-----|---------|------|
| U2 | LT8606EMSE | Buck |
| C4 | 10µF/50V X7R 1210 | Input bulk |
| C5 | 100nF/50V X7R 0603 | Input HF |
| L3 | **LQH32PN6R8NN0L** (6.8µH) | Cewka główna (patrz 5.3) |
| R_fb_H | 1M 1% 0603 | Dzielnik FB |
| R_fb_L | 240k 1% 0603 | Dzielnik FB (Vout=4.99V) |
| C_ff | 22pF NP0 0402 | Feedforward |
| R_T | 24k9 1% 0603 | f_sw ~2 MHz |
| C_BST | 100nF/16V X7R 0402 | Bootstrap |
| R_uv_H / R_uv_L | 1M / 137k 0603 | UVLO start ~6.5V |
| C_OUT1, C_OUT2 | 22µF/16V X7R 1206 | Output ×2 |

**Wyliczenie dzielnika FB** (V_FB = 0.97 V dla LT8606):
```
Vout = 0.97 × (1 + R_fb_H / R_fb_L) = 0.97 × (1 + 1M/240k) = 0.97 × 5.167 = 5.01 V ✓
```

**Wyliczenie cewki** (przy 2 MHz, ΔI_L = 20% × Iout):
```
L_min = (Vin_max - Vout) × Vout / (Vin_max × f_sw × ΔI_L)
      = (42 - 5) × 5 / (42 × 2e6 × 0.15) = 14.7 µH teoret.
Dobór 6.8µH: przy Burst Mode prąd jest ograniczany wewnętrznie, mniejsza L
              daje lepszą odpowiedź i mniejszą obudowę. OK dla tej mocy.
```

### 5.3 Cewka L3: LQH32PN6R8NN0L

**Murata LQH32PN0-series (1210)** — power inductor, wire-wound, **magnetycznie ekranowany**, przeznaczony do dławików w obwodach zasilania DC. To właściwy typ (NIE seria filtrująca/sygnałowa).

| Parametr | Wartość |
|----------|---------|
| Indukcyjność | 6.8 µH ±30% (sufiks N) |
| Obudowa | 1210 (3.2 × 2.5 × 1.55 mm) |
| Isat | ~850–900 mA |
| DCR | ~250 mΩ |
| Konstrukcja | Wire-wound, ekranowany |

**Weryfikacja:** Isat ~850 mA daje komfortowy margines nawet przy chwilowym zwarciu wyjścia (current limit LT8606 ~700 mA — cewka nie nasyci się). Tolerancja ±30% jest bez znaczenia dla Burst Mode. Zasada doboru: "power inductor / wirewound / shielded" = OK dla SMPS; "multilayer / RF / EMI suppression" = NIE.

### 5.4 LDO 5V → 3.3V: TPS7A0230

**TPS7A0230DBVR** (TI), SOT-23-5. Zasila VDD i VDDA MCU.

| Parametr | Wartość |
|----------|---------|
| Vin | 1.7–6.0 V |
| Vout | 3.3 V fixed |
| **Iq** | **25 nA** |
| Iout max | 200 mA |
| Dropout | 60 mV @ 1 mA |

| Ref | Wartość | Rola |
|-----|---------|------|
| U5 | TPS7A0230DBVR | LDO |
| C_LDO_in | 1µF/10V X7R 0603 | Wejście |
| C_LDO_out | 1µF/10V X7R 0603 | Wyjście |

Straty przy 1 mA (MCU średnio): 1 mA × 1.7 V = 1.7 mW — marginalne. Alternatywa (gdyby TPS niedostępny): MCP1700-3302 (Iq 1.6 µA).

---

## 6. Referencja ADC

**Rozstrzygnięcie:** w obudowie UFQFPN32 **VREF+ jest wewnętrznie związane z VDDA**. Nie ma osobnego pinu referencji. Rozważany LM4040 (shunt reference) **nie pasuje** — to nie LDO, nie udźwignie zasilania VDDA (MCU pobiera 2–5 mA impulsowo), a i tak nie ma gdzie go podłączyć.

**Przyjęte rozwiązanie:**
- VDDA zasilane z LDO TPS7A0230 (stabilne 3.3 V)
- **VREFINT** (wewnętrzna referencja 1.212 V, factory-calibrated w ROM) do programowej korekcji
- Firmware mierzy VREFINT, wylicza rzeczywiste VDDA, koryguje pomiar HV
- Dokładność ~0.5–1% — wystarczająca dla lampy GM

Filtracja VDDA: ferryt L4 (BLM18PG471) między VDD a VDDA + 100nF + 1µF.

---

## 7. Generator HV — flyback sterowany z MCU

### 7.1 Topologia

**Flyback DCM (Discontinuous Conduction Mode), single-switch, non-izolowany, sterowany burst-by-burst z MCU.** Bez dedykowanego kontrolera PWM — timer TIM2 generuje impulsy bramki, ADC mierzy HV przez dzielnik, firmware zamyka pętlę bang-bang.

### 7.2 Cykl pracy

1. **MOSFET ON** (t_on = 7 µs): TIM2 podaje impuls na bramkę Q1, prąd w Lp rośnie liniowo do I_peak ≈ 467 mA, energia magazynowana w rdzeniu (8.18 µJ). Dioda D6 zatkana.
2. **MOSFET OFF**: pole magnetyczne odwraca napięcie na wtórnym, D6 przewodzi, energia przelewa się do C8/C9 (ładuje HV). Prąd wtórnego opada do zera.
3. **Faza martwa**: prąd = 0 (stąd "discontinuous"), czekanie do następnego cyklu.
4. **Regulacja** (co 1 s przy wybudzeniu RTC): ADC mierzy HV; jeśli HV < setpoint → MCU wysyła paczkę (burst) impulsów; jeśli HV ≥ setpoint → nic (HV opada powoli przez dzielnik i lampę).

### 7.3 Zalety topologii dla micropower

- Flyback "śpi" — budzi się ~30×/s na kilka µs zamiast pracować ciągle jak kontroler IC (oszczędność mA→µA)
- Setpoint programowalny przez CAN (400 V DOI-80, 500 V LND712) bez zmian sprzętu
- MCU zna rzeczywiste HV → diagnostyka, wykrywanie awarii, raport przez CAN
- Minimalny BOM (brak kontrolera)

### 7.4 Blanking komparatora

Podczas burst flybacka przełączanie generuje zakłócenia, które mogłyby dać fałszywe zliczenia GM. Rozwiązanie: **blanking komparatora COMP1 na 50 µs** podczas każdego impulsu flybacka (realizacja programowa). Koszt: ~5% martwego czasu przy burst 1000 Hz — akceptowalne.

---

## 8. Transformator T1 (P18/11)

### 8.1 Rdzeń i karkas

| Element | Wybór | Uwaga |
|---------|-------|-------|
| Rdzeń | **P18/11-3F3** (Ferroxcube), ungapped | Materiał 3F3 (100–500 kHz), otwór centralny ø3 mm. Ekwiwalent: B65651D0000R048 (EPCOS N48) |
| Karkas | **B65652B0000T001** (EPCOS/TDK) | 1-sekcyjny, PET, klasa F |
| Szczelina | **Własna 0.1 mm — Kapton 100 µm między połówkami (symetryczna)** | AL efektywne ~270 nH/N² |

Parametry rdzenia P18/11: Ae = 43.3 mm², le = 25.8 mm, Ve = 1120 mm³, Bs ≈ 440 mT (100°C).

### 8.2 Uzwojenia

| Uzwojenie | Zwoje | Drut | Konstrukcja |
|-----------|-------|------|-------------|
| Pierwotne (P1–P2) | **17 zw** | **0.30 mm em CuL kl.2** | 1 warstwa na dnie karkasu |
| Wtórne (S1–S2) | **510 zw** | **0.10 mm em CuL kl.2** | ~8 warstw, Kapton 25 µm między warstwami |

### 8.3 Parametry elektryczne

```
AL (g=0.1mm) = µ₀ × Ae / g_eff = 4π×10⁻⁷ × 43.3e-6 / 0.2e-3 = 272 nH/N²
Np = √(Lp / AL) = √(75000 / 272) = 17 zw
Ns = n × Np = 30 × 17 = 510 zw
Lp = 75 µH,  Ls = 67.5 mH,  n = 1:30
I_peak (Vin=5V, t_on=7µs) = 5 × 7e-6 / 75e-6 = 467 mA
Energia/puls = 0.5 × 75e-6 × 0.467² = 8.18 µJ
B_peak = Lp × I_peak / (Np × Ae) = 75e-6 × 0.467 / (17 × 43.3e-6) = 47.6 mT
         → 10× margines do Bs=440 mT
Vds Q1 max = 5 + 550/30 + ringing ≈ 25 V  (OK dla DMN3404L 30V)
Vrev D6 = 550 + 30×5 = 700 V → potrzeba diody ≥1000 V (STTH112 1200V)
```

### 8.4 Mocowanie śrubą M2

Rdzeń ma centralny otwór ø3 mm (bez gwintu). Mocowanie śrubą zamiast klipsy:

```
        nakrętka M2 nylock A2 + podkładka nylonowa
              │
   ┌──────────┴──────────┐  ← górna połówka P18/11
   │   karkas + zwoje     │
   └──────────┬──────────┘  ← dolna połówka
              │
        podkładka nylonowa
              │
        śruba M2×16 A2 (imbus DIN 912)
```

| Element | Specyfikacja | Uwaga |
|---------|-------------|-------|
| Śruba | M2 × 16 mm, **stal nierdzewna A2** | DIN 912 imbus. NIGDY zwykła stal (ferromagnetyczna!) |
| Nakrętka | M2 nylock A2 | DIN 985, samohamowna |
| Podkładki | 2× M2 nylonowa | **Kluczowe — chronią kruchy ferryt** przed pęknięciem |
| Klej | Epoksyd na gwincie po dostrojeniu | Zabezpieczenie ostateczne |

**Krytyczne uwagi:** śruba musi być niemagnetyczna (A2 lub mosiądz) — zwykła stal zaburzyłaby pole magnetyczne rdzenia. Podkładki nylonowe obowiązkowe — ferryt ceramiczny pęka pod naciskiem punktowym metalu. Moment ~0.2–0.3 Nm, docisk 20–60 N wg datasheetu TDK dla stabilnego AL. Po dostrojeniu (kontrola Lp=75µH mostkiem LCR) — zaklejenie.

### 8.5 Procedura nawijania

1. Pierwotne: 17 zw 0.30 mm na dnie karkasu, 1 warstwa równomiernie
2. Izolacja: 2× Kapton 50 µm
3. Wtórne: 510 zw 0.10 mm, warstwami po ~60 zw, Kapton 25 µm między warstwami
4. Zewnętrznie: 2–3× Kapton
5. Szczelina: Kapton 100 µm w obu połówkach (środkowa nóżka + pierścień)
6. Montaż śrubą, kontrola Lp, zaklejenie epoksydem
7. Testy: Lp ≈ 75 µH ±10%, Ls ≈ 67 mH, leakage < 1.5 µH, izolacja P/S > 1 GΩ, hi-pot 1 kV/60 s

### 8.6 Długość drutu

- Pierwotne (17 zw, ALT ~32 mm): 17 × 32 = 544 mm + zapas = **~75 cm**
- Wtórne (510 zw, ALT ~35 mm śr.): 510 × 35 = 17850 mm + zapas = **~20 m**
- Zakup: rolki 25 g każdego drutu (wystarczą na 10+ prototypów)

---

## 9. Strona pierwotna flybacka i snubber

### 9.1 Połączenia

```
+5V ─┬─ C6 (10µF) ─ GND_PWR       (bulk lokalny, tuż przy T1)
     └─ T1.P1 (kropka)
T1.P2 ─ DRAIN
Q1 (DMN3404L): D→DRAIN, G→GATE_INT, S→SOURCE
R7 (22Ω) : GATE_INT — MCU_PB10 (TIM2_CH3)
R12 (100k): GATE_INT — GND_PWR   (pull-down, trzyma Q1 zatkany w Stop/reset)
R8 (2.2Ω): SOURCE — GND_PWR      (sense, opcjonalny — lub zwora)
```

### 9.2 Snubber RCD (clamp do +5V)

Snubber chroni Q1 przed przepięciem od indukcyjności rozproszenia (Lleak ~1 µH). Wybór: **RCD clamp podłączony do +5V** (nie do GND).

```
DRAIN → D_sn (anoda→DRAIN, katoda→CLAMP)
CLAMP → R_sn (100Ω) ∥ C_sn (1nF) → +5V
```

| Ref | Wartość | Rola |
|-----|---------|------|
| D_sn | **BAV21WS-DIO** (Diotec, SOD-323F) | Dioda clamp: 250V, 200mA, trr<50ns, ultrafast |
| R_sn | 100Ω 0603 | Rozładowanie C_sn do 5V |
| C_sn | 1nF/50V NP0 0603 | Magazyn energii leakage |

**Dlaczego clamp do +5V, nie GND:**
- **Recykling energii** leakage do szyny 5V zamiast strat w cieple
- **Niższe Vds:** `Vds = Vin + V_clamp_cap ≈ 5 + 15 = 20 V` (bezpieczne dla DMN3404L 30V max)
- **Cichy EMC** (miękkie przycinanie) — kluczowe przy obudowie Al, gdzie conducted immunity jest priorytetem

**Dioda D_sn:** BAV21WS-DIO wystarcza z ogromnym zapasem — prąd przez diodę w szczycie ~50–100 mA << IFSM 2 A. Musi być fast/ultrafast (nie 1N400x). Sufiks -DIO to producent Diotec; -RHG to ta sama dioda innego producenta — bierzemy tańszą -DIO.

**Dostrajanie:** obserwuj dren Q1 na oscyloskopie podczas wyłączania. Dobry przebieg: plateau ~18–22 V, spike < 25 V. Jeśli spike > 28 V → zmniejsz R_sn lub zwiększ C_sn. Jeśli dzwonienie HF → dodaj opcjonalny RC (10Ω + 220pF) dren-GND.

### 9.3 Zasady layoutu strony pierwotnej

1. **Pętla mocy minimalna**: C6 → Lp → Q1 → (R8) → GND → C6. Ta pętla przewodzi impulsy 467 mA z szybkimi zboczami — minimalizuj jej powierzchnię.
2. **Snubber tuż przy drenie** — każdy mm ścieżki to indukcyjność niwecząca działanie.
3. **Gate loop krótki** — R7 blisko bramki, ścieżka od MCU krótka.
4. **GND_PWR jako polygon**, łączona z GND_HV i GND cyfrową w jednym punkcie (star ground).
5. **Trafo off-board** — przewody P1/P2 krótkie (4–6 cm), skręcone jeśli możliwe.

---

## 10. Dzielnik HV i ochrona wejścia ADC

### 10.1 Struktura dzielnika

```
+HV → R9 (500MΩ) → R10 (500MΩ) → HV_meas → R11 (1MΩ) → GND
                                     │
                                     ├─ C16 (100pF NP0) → GND
                                     ├─ D3 (BAV199) klamry → 3.3V / GND
                                     └─ ADC1 (PA5)
```

Dzielnik /1001. Przy HV=550 V: HV_meas = 0.549 V. Prąd dzielnika ~0.55 µA (micropower).

### 10.2 Rezystory

| Ref | Wartość | Uwaga |
|-----|---------|-------|
| R9, R10 | **HVC1206T5005JET** (500MΩ, 5%) | 2× w szereg = 1GΩ. Working voltage 1 kV, 2× szereg → ~275 V/szt margines |
| R11 | 1MΩ 0603, 25V | Dolna gałąź. W normalnej pracy 0.55 V (margines 45×) |
| C16 | **100pF NP0 0603** | Antyalias (τ = 1GΩ ∥... × 100pF ≈ 100 ms). NIE 10pF |

**Tolerancja 5% R9/R10 jest OK** — usuwana kalibracją HV (którą i tak wykonujesz ze wzorcem). TCR ~100 ppm/°C daje dryf ~0.4% przy ΔT=40°C → ±1.6 V @ 400 V, nieistotne dla plateau lampy GM. Dwa rezystory z tej samej partii dryfują zgodnie (tracking). Stabilność długoterminowa 0.25–0.5%/rok — poniżej progu istotności.

**R11 na 25 V jest OK** — w normalnej pracy 0.55 V. Ryzyko tylko przy kaskadowej awarii górnej gałęzi (bardzo mało prawdopodobnej — rezystory HV padają w przerwę, nie zwarcie).

### 10.3 Ochrona wejścia ADC — klamra BAV199

**Kluczowa ochrona:** para diod low-leakage **BAV199** klamruje HV_meas do szyn 3.3V i GND. Chroni wejście ADC STM32 przed zniszczeniem w razie awarii dzielnika (setki woltów do pinu MCU).

**Dlaczego BAV199, nie Zener 3V ani Schottky:**
- **Zener 3V** ma miękkie kolano (soft knee) — upływ dziesiątki–setki nA już przy 0.5–1 V, porównywalny z prądem dzielnika (550 nA) → zakłóca i destabilizuje pomiar. ODRZUCONY.
- **BAT54 Schottky** — upływ 1–2 µA, za duży. ODRZUCONY.
- **BAV199** — upływ <5 nA, specjalnie do ochrony wysokoimpedancyjnych wejść. WYBRANY.

**Pinout BAV199** (dual series, diody połączone szeregowo): Pin1 = A1, Pin2 = K2, Pin3 = wspólny (K1, A2). Podłączenie:
```
Pin 2 → +3.3V   (górna dioda: anoda=Pin3/HV_meas, katoda=Pin2 → odprowadza >3.3V do szyny)
Pin 3 → HV_meas (wspólny punkt środkowy)
Pin 1 → GND     (dolna dioda: anoda=Pin1/GND, katoda=Pin3 → chroni przy napięciu ujemnym)
```

### 10.4 Opcja: lepsze wykorzystanie FSR ADC

Obecnie HV_meas = 0.55 V wykorzystuje ~17% zakresu 3.3 V. Opcjonalnie można podnieść do ~1.1 V przez **R11 = 2 MΩ (metoda B)** — NIE przez usunięcie górnego 500 MΩ (metoda A), która podwoiłaby prąd dzielnika i dała 549 V na jednym rezystorze.

Metoda B: dzielnik /501, HV_meas = 1.1 V, prąd dzielnika bez zmian (~0.55 µA), margines górnych rezystorów zachowany. Wymaga C16 ≥ 100 pF (lepiej 1 nF) i długiego sample time ADC (640 cykli) bo impedancja źródła rośnie do 2 MΩ.

**Werdykt:** opcjonalna optymalizacja "nice to have". Obecny wariant (0.55 V + oversampling 16× → efektywne 14-bit → ~0.15% rozdzielczości HV) jest funkcjonalnie wystarczający dla lampy GM. Metoda B warta rozważenia tylko dla wersji spektrometrycznej.

---

## 11. Detektor G-M i shaper impulsów (robust)

### 11.1 Topologia — odczyt z anody

**Odczyt z anody, katoda na masie** (walidowane przez notę ADI CN-0536: katoda uziemiona minimalizuje podatność na zakłócenia). Gdy zajdzie zdarzenie, anoda jest chwilowo ściągana ku masie, tworząc ujemny skok ~400 V.

### 11.2 Schemat wzmocniony

```
+HV ─→ R13 (quench, zwora 5.1M dla DOI-80) ─┬─ ANODA
       Ca (3.3pF/1kV) równolegle do R13     │
                                     KATODA ─┴─ GND
ANODA ─→ C_top (4.7pF/1kV) ─→ SIGNAL
SIGNAL ─→ C_bot (1nF) ─→ GND               (dzielnik pojemnościowy ~213×)
SIGNAL ─→ R_bias (1M) ─→ +3.3V             (bias ~3.0V)
SIGNAL ─→ R_s (1k) ─→ COMP_IN
COMP_IN ─→ BAV99 klamry → 3.3V / GND       (backup)
COMP_IN ─→ COMP1+ (PA1)
DAC1_OUT1 ─→ COMP1− (próg ~2.0V)
```

### 11.3 Elementy i uzasadnienie

| Ref | Wartość | Rola |
|-----|---------|------|
| R13 | Zwora 4.7M/5.1M/10M HV 1206 | **Rezystor gaszący specyficzny dla lampy** (patrz §3) |
| Ca | 3.3pF/1kV NP0 1206 | Na R13: definiuje ładunek impulsu (C_tuby ~1pF), przyspiesza gaszenie |
| C_top | 4.7pF/1kV NP0 1206 | Górny człon dzielnika pojemnościowego (strona HV) |
| C_bot | 1nF/50V NP0 0603 | Dolny człon — ustala stosunek |
| R_bias | 1M 0603 | Bias węzła do ~3.0 V |
| R_s | 1k 0603 | Szereg — ochrona pinu MCU, ogranicza prąd transientu |
| D (klamry) | BAV99 (SOT-23) | Backup: klamry do 3.3V/GND |

### 11.4 Zasada działania

- **Spoczynek:** SIGNAL = bias ~3.0 V (przez R_bias). Anoda ≈ +HV (brak prądu przez lampę).
- **Wyładowanie:** lampa przewodzi, anoda spada o ~400 V. Dzielnik pojemnościowy C_top/C_bot przenosi **kontrolowany** ujemny skok: `ΔV = 400 × C_top/(C_top+C_bot) = 400 × 4.7/1004.7 ≈ 1.9 V`. SIGNAL spada z 3.0 V do ~1.1 V, przekracza próg 2.0 V → COMP1 przełącza → LPTIM zlicza.
- **Powrót:** R_bias ładuje węzeł z powrotem do 3.0 V (τ = R_bias × C_bot ≈ 1 ms). Uwaga: przy bardzo wysokich count rate (>1000 CPS) rozważ mniejsze R_bias — ale czas martwy tuby ~100 µs i tak ogranicza.

### 11.5 Dlaczego dzielnik pojemnościowy (nie pojedynczy kondensator + klamry)

Dzielnik pojemnościowy **kontroluje amplitudę** stosunkiem pojemności (jak w CN-0536: C28/C26 tworzą dzielnik biasowany do referencji, komparator przełącza przy przekroczeniu progu). Zalety vs pojedynczy kondensator obcinany klamrami:
- **Powtarzalna wysokość impulsu** (niezależna od zmiennego napięcia tuby)
- **Mniejszy stres na klamrach** (backup, nie główny limiter)
- **Czysty sygnał** — impuls nie railuje, ląduje w zakresie komparatora

### 11.6 Kondensator Ca (definicja ładunku + gaszenie)

Ca (3.3pF/1kV) równolegle do R13: gdy lampa wyzwala lawinę, Ca ładuje się szybko, obniżając napięcie na tubie i przyspieszając gaszenie. Ponieważ własna pojemność tuby ~1 pF, Ca definiuje powtarzalny ładunek impulsu. Musi być ≥1 kV (jedna noga na anodzie/HV).

### 11.7 Próg COMP1

Ustawiany programowo przez DAC1_OUT1 → wewnętrzne wejście INM komparatora. Typowo ~2.0 V (poniżej baseline 3.0 V, powyżej dna impulsu 1.1 V). Regulowany software'owo pod konkretną lampę i poziom szumu.

### 11.8 Opcja: bufor tranzystorowy

Dla zintegrowanej sondy (tuba + elektronika w jednej obudowie Al, krótka ścieżka) **komparator wewnętrzny wystarcza** — jak w CN-0536. Bufor tranzystorowy (MMBT3904 w emiterze wspólnym) między SIGNAL a MCU warto dodać tylko gdyby: (a) surowy impuls był wyprowadzany kablem do osobnej elektroniki, lub (b) obserwowano fałszywe zliczenia od zakłóceń. Dodaje izolację i drive kosztem elementów i poboru. Dla tej sondy — zbędny.

### 11.9 Opcja: dzielnik kompensowany (config c)

Dla maksymalnej wierności kształtu impulsu (jak zalecenie producenta dla DOI-50) można zastosować dzielnik kompensowany częstotliwościowo: kondensatory bocznikujące R1 i R2 tak, by R1·C1 = R2·C2 (jak sonda oscyloskopowa). Dla samego zliczania (nie analizy kształtu) to przerost — komparator wykrywa tylko przekroczenie progu. Opcja na przyszłość, gdyby analizować kształt.

---

## 12. Transceiver CAN

### 12.1 Wybór: ATA6561 (zamiast TJA1042)

**Problem dostępności:** TJA1042T/3 w TME tylko "na zamówienie specjalne" (0 na stanie, MOQ 2500). Zamiennik: **ATA6561-GAQW-N** (Microchip, SO-8) — dostępny w TME, funkcjonalnie równoważny.

| Parametr | ATA6561 |
|----------|---------|
| Zgodność | ISO 11898-2/-5, CAN FD ready 5 Mbit/s |
| VCC | 5 V (natywnie z bucka) |
| VIO | 3.3 V (bezpośrednie interfejsowanie z MCU) |
| Tryby | Normal / Standby (bez Silent) |
| Pobór Standby | ~10 µA (wake-up on bus) |
| Pobór Normal | ~5 mA (chwilowo) |

### 12.2 Pinout i podłączenie

```
Pin 1: TXD  ← MCU PA12 (CAN1_TX)
Pin 2: GND
Pin 3: VCC  ← +5V
Pin 4: RXD  → MCU PA11 (CAN1_RX) + PA15 (EXTI15 wake)
Pin 5: VIO  ← +3.3V
Pin 6: CANL ↔ magistrala
Pin 7: CANH ↔ magistrala
Pin 8: STBY ← MCU PA7 (standby/normal)
```

**Uwaga:** pinout podobny do TJA1042T/3, ale zweryfikuj footprint przy projektowaniu (STBY na pin 8, VIO na pin 5). bxCAN **nie działa w Stop 2** — wybudzanie MCU przez EXTI15 podpięte do RXD, potem inicjalizacja CAN.

| Ref | Wartość | Rola |
|-----|---------|------|
| U1 | ATA6561-GAQW-N (SO-8) | Transceiver |
| C_CAN | 100nF/50V X7R 0402 | VCC decoupling |
| C_VIO | 100nF/16V X7R 0402 | VIO decoupling |
| R5 + JP1 | 120Ω 0805 + jumper | Terminacja (skrajne węzły) |
| C_split1, C_split2 | 4.7nF/50V X7R 0805 (opcj.) | Split termination (CM immunity) |

---

## 13. Przypisanie pinów MCU (STM32L432KCU6, UFQFPN32)

| Pin | Funkcja | Sygnał |
|-----|---------|--------|
| PA1 | COMP1_INP | Impuls z G-M (po shaper) |
| PA2 | LPUART1_TX | Debug UART (opcjonalny) |
| PA3 | LPUART1_RX | Debug UART (opcjonalny) |
| PA4 | DAC1_OUT1 | Próg COMP1 (wewnętrzny) |
| PA5 | ADC1_IN10 | HV_meas (dzielnik HV) |
| PA6 | ADC1_IN11 | VIN_SENSE (dzielnik Vin) |
| PA7 | GPIO | STBY → ATA6561 (standby/normal) |
| PA8 | GPIO | ALARM_OUT (open-drain, opcjonalny) |
| PA9 | GPIO | LED_STATUS (opcjonalny) |
| PA11 | CAN1_RX (AF9) | CAN receive |
| PA12 | CAN1_TX (AF9) | CAN transmit |
| PA13/PA14 | SWDIO/SWCLK | Programator |
| PA15 | GPIO / EXTI15 | Wake-up z RXD transceivera |
| PB0 | GPIO | LED_HV (opcjonalny) |
| PB3 | COMP1_OUT | Wewnętrznie do LPTIM1 (zliczanie) |
| PB4 | GPIO | EN_HV (włączenie flybacka) |
| PB5 | GPIO | PG (Power Good z bucka) |
| PB6 | I2C1_SCL | Rezerwa |
| PB7 | I2C1_SDA | Rezerwa |
| PB10 | TIM2_CH3 | GATE_FB → bramka Q1 |

Zasilanie: VDD, VDDA ← +3.3V z LDO; ferryt L4 (BLM18PG471) między VDD a VDDA; VREF+ wewnętrznie związane z VDDA. NRST z 10k pull-up + 100nF. BOOT0 z 10k pull-down.

---

## 14. Zabezpieczenia EMC (ESD / Surge / Burst)

### 14.1 Normy i poziomy docelowe

Sonda polowa musi przejść testy odporności wg EN 61326-1 / EN 61000-6-2:

| Norma | Test | Poziom (przemysł) | Charakterystyka |
|-------|------|-------------------|-----------------|
| IEC 61000-4-2 | ESD | ±8 kV kontakt, ±15 kV powietrze | 1 ns rise, niska energia |
| IEC 61000-4-4 | EFT/Burst | ±2 kV (zasilanie), ±1 kV (sygnał) | 5/50 ns pakiety |
| IEC 61000-4-5 | Surge | ±1 kV linia-linia, ±2 kV linia-ziemia | 1.2/50 µs, **wysoka energia** |

**Rozróżnienie energii:** ESD/EFT — krótkie, niska energia (TVS radzą sobie same). Surge — 3–4 rzędy wielkości więcej energii → wymaga GDT/dużych TVS.

### 14.2 Filozofia — kaskada coarse → fine

Obudowa aluminiowa ekranuje radiated, więc **priorytet to conducted immunity**. Strategia kaskadowa:

```
Linia → [Stopień 1: GDT/duży TVS] → [impedancja: ferryt] → [Stopień 2: TVS fine] → obwód
         (coarse, wysoka energia)     (rozprzęgnięcie)        (niska clamp)
```

Pierwszy stopień przejmuje energię surge, element pośredni (ferryt) wytwarza spadek pozwalający pierwszemu zadziałać, drugi ustala niskie napięcie clamp chroniące krzem.

### 14.3 Ochrona wejścia Vin

```
M12.1 → GDT1 → F1 → D1(TVS coarse) → L1(ferryt) → D2(Schottky) → D3(TVS fine) → C_bulk → LT8606
```

| Ref | Wartość | Rola | Norma |
|-----|---------|------|-------|
| GDT1 | Bourns 2038-15-SM 90V | Coarse surge linia-ziemia | 4-5 |
| F1 | PTC 0.5A 60V (MF-MSMF050) | Zwarciowa | — |
| D1 | SMBJ33CA (bidir.) | Coarse TVS | 4-5 |
| L1 | BLM31PG601 (600Ω) | Rozprzęgnięcie, EFT | 4-4 |
| D2 | PMEG6030EP | Odwr. polaryzacja | — |
| D3 | SMBJ36A | Fine TVS | 4-2 |
| C_bulk | 10µF/50V + 100nF | Absorpcja EFT | 4-4 |

Dobór: dla ±2 kV linia-ziemia (Rźr 12Ω → ~167 A) GDT 90V przejmuje energię (Ipp >1 kA), SMBJ33 (18A @ 8/20µs) obsługuje resztki po ferrycie. Ferryt szeregowy kluczowy — bez niego TVS się przeciąży.

### 14.4 Ochrona interfejsu CAN

```
M12.3 (CANH) → CDSOT23-T24CAN → L2(CMC) → [R 10Ω opcj.] → ATA6561.CANH
M12.4 (CANL) → CDSOT23-T24CAN → L2      → [           ] → ATA6561.CANL
```

**TVS: CDSOT23-T24CAN (Bourns)** — dedykowany dla CAN, dostępny w TME (1375 szt).

| Parametr | Wartość |
|----------|---------|
| Working voltage | 24 V (CAN + margines miswiring) |
| Clamping | 40 V @ Ipp |
| Ipp (8/20µs) | 8 A |
| Zgodność | IEC 61000-4-2 (±30 kV), -4-4, -4-5 |

| Ref | Wartość | Rola |
|-----|---------|------|
| D4 | CDSOT23-T24CAN | TVS CAN |
| L2 | Würth 744232091 | CMC |
| R1, R2 | 10Ω 0805 (opcj.) | Pulse-proof do transceivera |
| GDT2 (opcj.) | GDT 90V | Coarse surge dla ostrych warunków |

ATA6561 ma wbudowaną ochronę ±8 kV IEC ESD i ±58 V bus fault, ale surge wymaga zewnętrznych elementów.

### 14.5 Ochrona Earth/Shield (pin 5)

```
M12.5 (Earth) → obudowa Al (bezpośrednio)
              → C_shield 1nF/2kV (Y-cap) → GND_sygnałowa (AC shield tie)
```

Obudowa połączona z Earth, masa sygnałowa tylko przez kondensator (nie DC) — unika pętli ziemi, zapewnia drogę AC dla CM i ESD.

### 14.6 Tłumienie conducted emission (priorytet)

Radiated ekranowane obudową, ale conducted wraca linią. Źródła: LT8606 @ 2 MHz i flyback burst. Filtr: ferryt L1 + kondensatory X 4.7µF na Vin. Pętla pierwotna flybacka mała (mniej emisji). Ewentualnie mały CMC na Vin/GND jeśli CM problematyczny.

### 14.7 Zasady layoutu EMC

1. Kolejność: coarse (GDT) najbliżej złącza, fine (TVS) najbliżej obwodu. Nigdy odwrotnie.
2. "Brudna masa" ochronna (TVS/GDT) połączona z obudową w jednym punkcie, oddzielona od masy cyfrowej.
3. Krótkie ścieżki do TVS (indukcyjność dodaje się do clamp: L×di/dt).
4. Prąd surge poza obszarem cyfrowym/analogowym.
5. Guard ring wokół sekcji wejściowej, połączony z obudową.

---

## 15. Budżet mocy

### 15.1 Tryb IDLE (Vin = 12 V)

| Blok | Pobór z 12V |
|------|-------------|
| STM32L432 Stop 2 + LPTIM + RTC | ~1.4 µA |
| Komparator wewnętrzny | ~0.7 µA |
| LDO TPS7A0230 Iq | ~15 nA |
| ATA6561 Standby | ~10 µA |
| Dzielnik HV (1GΩ @ 400V) | ~0.4 µA |
| Flyback burst (~30 Hz) | ~35 µA |
| Iq bucka LT8606 (Burst Mode) | ~2.5 µA |
| **RAZEM IDLE** | **~50–200 µA** |

Idle ~2.4 mW @ 12 V — najlepszy budżet dzięki LT8606 (Iq 2.5 µA). Dominują flyback i ATA6561 standby.

### 15.2 Tryb ACTIVE

| Blok | Pobór z 12V |
|------|-------------|
| STM32L432 Run @ 16 MHz | ~1.5 mA |
| ATA6561 Normal | ~2 mA |
| Reszta | ~250 µA |
| **RAZEM** | **~3.7 mA** |

Średnio (1 ramka/s, 20 ms aktywności): ~250–310 µA.

### 15.3 Energetyka HV (Vin flyback = 5 V)

```
Energia/puls = 8.18 µJ (2.3× więcej niż z 3.3V → niższy burst rate)
Burst rate typ. ~30 Hz @ 400V, ~60 Hz @ 550V
Moc HV dostarczana: 250–500 µW
Prąd z 5V (średnio): ~50 µA
```

---

## 16. Konstrukcja mechaniczna i złącze M12

### 16.1 Złącze M12 5-pin A-coded

Standard przemysłowy (CANopen CiA 303-1, IEC 61076-2-101). IP67, kable patch dostępne.

```
Pin 1: +Vin (7-32V)
Pin 2: GND
Pin 3: CAN_H
Pin 4: CAN_L
Pin 5: Shield / Earth / obudowa
```

Panel-mount żeński: Binder 09-3441-77-05 / TE / Amphenol. Zakres -40..+85°C.

### 16.2 Obudowa aluminiowa

- Cylindryczna, Ø wewn. 25 mm (mieści trafo P18/11 ~Ø18 mm z ~3.5 mm marginesem)
- Połączona z Earth (pin 5) — ekran EMI + droga ESD
- Uszczelnienie IP67

### 16.3 Rozmieszczenie na PCB — strefowanie

```
┌────────────────────────────────────────────────────────┐
│ [M12] [Ochrona Vin+CAN] [LT8606] [ATA6561] [C_out 5V]  │
│ [TPS7A0230 LDO]  [STM32L432KCU6 + decoupling]          │
│ [Trafo T1 off-board — pinheader / przewody 4-6cm]      │
│ [Flyback Q1, snubber, D6, filtry HV]                   │
│ [Dzielnik HV — creepage 5mm od cyfrowej]               │
│ [Lampa G-M + zwora R_anode + shaper]                   │
└────────────────────────────────────────────────────────┘
```

Zasady: strefa HV oddzielona kanałami (creepage 5 mm dla 550V + margines); star ground blisko C9; ekran/shield osobna warstwa pod HV; odległość dzielnika HV od cyfrowych ≥3 mm.

---

## 17. Elementy krytyczne i dostępność TME

| Element | Status TME | Rozwiązanie |
|---------|-----------|-------------|
| **LT8606EMSE** | Niedostępny | **Mouser/DigiKey/LCSC** — świadomy wybór, jedyny spoza TME |
| TJA1042T/3 | Special order MOQ 2500 | **Zamiana na ATA6561-GAQW-N** (TME) |
| **ATA6561-GAQW-N** | Dostępny ✓ | OK |
| **P18/11-3F3 / B65651D0000R048** | Dostępne ✓ | OK |
| **B65652B0000T001** (karkas) | Dostępny ✓ | OK |
| **STTH112** | Dostępny ✓ | OK |
| **CDSOT23-T24CAN** | Dostępny (1375 szt) ✓ | OK |
| **LQH32PN6R8NN0L** | Do weryfikacji | Alt: wariant M (±20%) |
| HVC1206T5005JET | Do weryfikacji (lead time) | Alt: Vishay CRHV1206 |
| BAV199 / BAV99 | Popularne ✓ | OK |
| BAV21WS-DIO | Dostępny ✓ | OK |
| TPS7A0230DBVR | Do weryfikacji | Alt: MCP1700-3302 |
| STM32L432KCU6 | Zwykle dostępny | Weryfikować stan |
| DMN3404L | Popularny | Weryfikować |
| Śruba M2 A2 + nakrętka + podkładki nylon | Dostępne ✓ | Kategoria złączki |

---

## 18. Kompletna lista BOM

### 18.1 Zasilanie wejściowe i ochrona EMC

| Ref | Wartość | Obudowa |
|-----|---------|---------|
| GDT1 | Bourns 2038-15-SM 90V | SMD |
| F1 | PTC 0.5A 60V (MF-MSMF050) | 1812 |
| D1 | SMBJ33CA | SMB |
| L1 | BLM31PG601SN1L | 1206 |
| D2 | PMEG6030EP | SOD-128 |
| D3 | SMBJ36A | SMB |
| C1 | 100nF/100V X7R | 0805 |
| C2, C3 | 4.7µF/50V X7R | 1210 |

### 18.2 Buck LT8606 (spoza TME)

| Ref | Wartość | Obudowa |
|-----|---------|---------|
| U2 | LT8606EMSE | MSOP-10E |
| C4 | 10µF/50V X7R | 1210 |
| C5 | 100nF/50V X7R | 0603 |
| L3 | LQH32PN6R8NN0L (6.8µH) | 1210 |
| R_fb_H | 1M 1% | 0603 |
| R_fb_L | 240k 1% | 0603 |
| C_ff | 22pF NP0 | 0402 |
| R_T | 24k9 1% | 0603 |
| C_BST | 100nF/16V X7R | 0402 |
| R_uv_H / R_uv_L | 1M / 137k | 0603 |
| C_OUT1, C_OUT2 | 22µF/16V X7R | 1206 |

### 18.3 LDO 3.3V

| Ref | Wartość | Obudowa |
|-----|---------|---------|
| U5 | TPS7A0230DBVR | SOT-23-5 |
| C_LDO_in, C_LDO_out | 1µF/10V X7R | 0603 |

### 18.4 CAN

| Ref | Wartość | Obudowa |
|-----|---------|---------|
| U1 | ATA6561-GAQW-N | SO-8 |
| D4 | CDSOT23-T24CAN | SOT-23 |
| L2 | Würth 744232091 | CMC |
| C_CAN | 100nF/50V X7R | 0402 |
| C_VIO | 100nF/16V X7R | 0402 |
| R5 | 120Ω | 0805 |
| JP1 | Jumper 1x2 | — |
| C_split1, C_split2 | 4.7nF/50V X7R (opcj.) | 0805 |
| R1, R2 | 10Ω (opcj.) | 0805 |

### 18.5 MCU

| Ref | Wartość | Obudowa |
|-----|---------|---------|
| U3 | STM32L432KCU6 | UFQFPN32 |
| C13-C17 | 100nF/16V X7R | 0402 |
| C_bulk_vdd | 10µF/10V X7R | 0805 |
| L4 | BLM18PG471SN1D | 0603 |
| R_NRST | 10k | 0603 |
| C_NRST | 100nF | 0402 |
| R_BOOT | 10k | 0603 |
| J2 | Header 1x6 SWD | — |

### 18.6 Flyback + snubber

| Ref | Wartość | Obudowa |
|-----|---------|---------|
| T1 | Trafo P18/11 własny (17:510) | off-board |
| Q1 | DMN3404L | SOT-23 |
| R7 | 22Ω | 0402 |
| R12 | 100k | 0402 |
| R8 | 2.2Ω (opcj. sense) | 0805 |
| C6 | 10µF/16V X7R | 0805 |
| D_sn | BAV21WS-DIO | SOD-323F |
| R_sn | 100Ω | 0603 |
| C_sn | 1nF/50V NP0 | 0603 |
| D6 | STTH112 (1200V) | SMA |
| C8 | 2.2nF/1kV NP0 | 1210 |
| C9, C9b | 100nF/1kV X7R | 1210 |

### 18.7 Dzielnik HV + ochrona ADC

| Ref | Wartość | Obudowa |
|-----|---------|---------|
| R9, R10 | HVC1206T5005JET (500MΩ 5%) | 1206 |
| R11 | 1M 0603 25V | 0603 |
| C16 | 100pF NP0 | 0603 |
| D3 | BAV199 (low-leakage dual) | SOT-23 |

### 18.8 Detektor G-M i shaper

| Ref | Wartość | Obudowa |
|-----|---------|---------|
| V1 | Lampa G-M (wymienna) | — |
| R13a/b/c | 4.7M / 5.1M / 10M HV (zwora) | 1206 |
| Ca | 3.3pF/1kV NP0 | 1206 |
| C_top | 4.7pF/1kV NP0 | 1206 |
| C_bot | 1nF/50V NP0 | 0603 |
| R_bias | 1M | 0603 |
| R_s | 1k | 0603 |
| D7 (klamry) | BAV99 | SOT-23 |

### 18.9 Złącza i trafo — mechanika

| Ref | Wartość | Uwaga |
|-----|---------|-------|
| J1 | M12 5-pin A-coded panel | Binder 09-3441-77-05 |
| J2 | Header 1x6 SWD | — |
| C_shield | 1nF/2kV Y-cap | Shield tie |
| Rdzeń | P18/11-3F3 | Ferroxcube (lub B65651D0000R048) |
| Karkas | B65652B0000T001 | EPCOS/TDK |
| Śruba | M2×16 A2 DIN 912 | Nierdzewna niemagnetyczna |
| Nakrętka | M2 nylock A2 DIN 985 | — |
| Podkładki | 2× M2 nylon | Chronią ferryt |
| Drut 0.30mm em CuL kl.2 | rolka 25g | ~1 m potrzeba |
| Drut 0.10mm em CuL kl.2 | rolka 25g | ~20 m potrzeba |
| Kapton 25µm | rolka 6mm×33m | Izolacja warstwowa |
| Kapton 100µm | wycinek | Szczelina |
| Epoksyd | kropla | Zaklejenie po dostrojeniu |

**Szacunkowy koszt v4.0:** ~160–200 zł (w tym LT8606 ~25 zł spoza TME, GDT ~5 zł, reszta TME).

---

## 19. Roadmap prac

**Faza 1 — Projektowa** ✅ (zakończona)
- [x] Topologia, dobór komponentów, weryfikacja dostępności TME
- [x] Obliczenia trafo, budżet mocy, EMC, detektor robust
- [ ] Schemat elektryczny w KiCad
- [ ] BOM z linkami (TME + Mouser dla LT8606)

**Faza 2 — Prototyp trafa**
- [ ] Zakup P18/11-3F3 + B65652 + śruba M2 A2 + druty + Kapton
- [ ] Nawinięcie 17:510, szczelina Kapton 100µm, montaż śrubą
- [ ] Kontrola Lp=75µH, zaklejenie
- [ ] Testy L, izolacja, leakage, hi-pot 1kV

**Faza 3 — PCB**
- [ ] Layout ze strefami HV/analog/cyfrowe + brudna masa EMC
- [ ] Sekcja zwory R_anode
- [ ] Off-board trafo
- [ ] Produkcja (JLCPCB) + montaż sekcyjny

**Faza 4 — Firmware** (osobny etap — poza tym dokumentem)
- [ ] Bring-up, LPTIM+COMP zliczanie w Stop 2, TIM2+pętla HV
- [ ] bxCAN + wake-up, protokół, persystencja, alarmy

**Faza 5 — Kalibracja i EMC**
- [ ] Kalibracja HV setpoint per lampa
- [ ] Kalibracja CPS→µSv/h ze wzorcem Cs-137
- [ ] Testy EMC: ESD, Burst, Surge (IEC 61000-4-2/-4/-5)
- [ ] Testy CAN na długiej linii (100 m+)

**Faza 6 — Mechanika**
- [ ] Obudowa Al, montaż M12, testy IP67

---

## 20. Referencje techniczne

- Ferroxcube: P18/11, materiał 3F3; EPCOS/TDK: B65651 (rdzeń), B65652 (karkas)
- STM32L432: RM0394, DS11453; bxCAN: AN2606, AN5028
- LT8606: Analog Devices datasheet
- ATA6561: Microchip DS20005991
- TPS7A0230: TI SBVS314
- Murata LQH32PN: datasheet serii
- STTH112: ST datasheet
- BAV199: Nexperia (low-leakage double diode); BAV21WS: Diotec
- CDSOT23-T24CAN: Bourns datasheet
- Analog Devices **CN-0536**: Geiger Counter Circuit (odczyt z anody, dzielnik pojemnościowy, komparator)
- Nuts & Volts: Pocket Geiger Unit (kondensator anodowy, definicja ładunku)
- Polski podręcznik metrologii promieniowania: układy pracy liczników DOI/BOI/GOI (rys. 5.36)
- IEC 61000-4-2 (ESD), -4-4 (EFT/Burst), -4-5 (Surge); EN 61326-1, EN 61000-6-2
- CAN 2.0B: ISO 11898-1/-2; CANopen: CiA 301, CiA 303-1; M12: IEC 61076-2-101
- Lampy G-M: datasheets producentów (Polon-Alfa, Rosenergoatom, LND)

---

*Koniec dokumentu v4.0 — kompletny projekt sprzętowy, gotowy do implementacji w KiCad. Firmware i mapa rejestrów CAN w osobnym opracowaniu.*
