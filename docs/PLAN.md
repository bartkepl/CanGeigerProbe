# Plan realizacji oprogramowania

**Stan na: 2026-11-15** · Dotyczy: firmware aplikacji i bootloadera sondy,
integracja z magistralą CanBusCommon (profil 2).

Dokument określa zakres, założenia i kolejność prac nad oprogramowaniem
sondy. Decyzje projektowe podjęte w trakcie realizacji zapisywane są jako
ADR w [`docs/adr/`](adr/README.md).

## 1. Zakres

Sonda jest węzłem magistrali zgodnym z profilem 2 standardu
[CanBusCommon](https://github.com/bartkepl/CanBusCommon), budowanym od początku
na warstwie wspólnej:

- podmoduł `common/`, opis urządzenia `protocol/device.yaml`, kod protokołu
  generowany (`common/protocol/generate.py`), zgodność sprawdzana walidatorem
  ekosystemu;
- wspólny bootloader CAN w wersji wire 1.1, w odmianie bez transportu UART
  ([ADR 0001](adr/0001-bootloader-bez-uart.md));
- aplikacja oparta na sprawdzonym zestawie modułów firmware węzła MPCC
  (scheduler kooperacyjny, warstwa CAN z oczekiwaniem na miejsce w kolejce,
  NVM we flashu, warstwa bezpieczeństwa i autotestu, IWDG, sterownik
  MCP79410, tor ADC z filtrem IQM i przeliczeniem ratiometrycznym względem
  VREFINT);
- odbiór warstwy wspólnej testerem zgodności `canconf`.

Numery z rejestrów wspólnych są już przydzielone i nie wymagają rewizji:

| Element | Wartość |
|---|---|
| `DEVICE_TYPE` | `0x0004` (CanGeigerProbe) |
| Zakres adresów | `0x30`–`0x3F`, domyślny `0x30` |
| Adres bootloadera | `0x30` |
| Czujniki | `GM_TUBE` (`0x30`), `HV_MONITOR` (`0x31`), `SUPPLY_MON` (`0x11`) |
| Wielkości | `DOSE_RATE` `0x40`, `CPM` `0x41`, `DOSE_ACC` `0x42`, `TUBE_HV` `0x43`, `SUPPLY_VOLTAGE` `0x12`, `SUPPLY_CURRENT` `0x13` |

CanBusCommon rozróżnia rdzeń standardu od rejestrów. Rdzeń — układ
identyfikatora, warstwa uniwersalna komend, zdarzeń i parametrów, kontrakt
bootloadera — jest zamrożony i nie jest zmieniany na potrzeby węzła. Rejestry
są rozszerzalne przez dopisanie: nowy węzeł dopisuje brakujące wielkości
i typy czujników przed implementacją (`common/docs/integracja.md` §1), a numer
raz nadany nie zmienia znaczenia. Wszystko, co specyficzne dla sondy, opisuje
`device.yaml` w zakresach device-specific.
| Zdolność | `RADIOMETRY` (bit 18) |

## 2. Założenia

| Założenie | Konsekwencja |
|---|---|
| Sonda pracuje na **wspólnej magistrali** z innymi węzłami (MPSWP, MPCC) | Węzeł nasłuchuje stale; transceiver pozostaje w trybie normalnym. Tryb Stop 2 i Standby transceivera nie są stosowane: ruch innych węzłów budziłby sondę na bieżąco, a węzeł uśpiony gubiłby zapytania hostów (bxCAN nie pracuje w Stop 2, pierwsza ramka po wybudzeniu przepada). |
| Pobór prądu ma być **możliwie najniższy** w trybie stałego nasłuchu | Strategia z sekcji 5; pobór mierzony własnym torem INA186. |
| Magistrala 500 kbit/s | Jak pozostałe węzły. Zmiana prędkości jest sprawą przyszłej wersji standardu. |
| Tuba bazowa **STS-5**, alternatywnie DOI-80 | Nastawa HV ok. 400 V; R12 = 4,7 MΩ (STS-5) lub 5,1 MΩ (DOI-80). |
| Kalibracja dozymetryczna odłożona | Do czasu kalibracji publikowane są surowe zliczenia; `DOSE_RATE` liczona ze współczynnikiem tymczasowym. |
| Transformator flybacka niedostępny w chwili uruchamiania | Generator HV realizowany jest po uruchomieniu komunikacji i toru zliczania (etap E4). |
| Jeden egzemplarz | Bez ewidencji egzemplarzy poza wpisem przy odbiorze. |

## 3. Mapa sprzętowa MCU

Przypisanie pinów odczytane z płytki (`pcb/geiger_probe.kicad_pcb`, układ U1).
Plan v5 i README podają dla `CAN_STBY` pin PA7 — stan nieaktualny.

| Pin | Sygnał | Funkcja |
|---|---|---|
| PA1 | `COMP_GM_IN` | COMP1 INP — impulsy z detektora G-M |
| PA2 | `FBACK_CTRL` | TIM2_CH3 — bramka klucza flybacka; TIM2_OC3 jest zarazem źródłem blankingu COMP1 |
| PA5 | `HV_meas` | ADC1 IN10 — dzielnik HV /1001 |
| PA6 | `VIN_SENSE` | ADC1 IN11 — dzielnik Vin 11:1 |
| PA7 | `I_SENS` | ADC1 IN12 — wyjście INA186 (G = 200, shunt 0,5 Ω) |
| PB0 | `EN_I_SENS` | wyjście — włączenie INA186 |
| PA9 | `CAN_STBY` | wyjście — ATA6561 STBY, stan niski = tryb normalny |
| PA10 | `CAN_R_CTRL` | wyjście — terminacja split (PhotoMOS), stan wysoki = załączona |
| PA11 / PA12 | `CAN_RX` / `CAN_TX` | bxCAN |
| PA15 | `CAN_RX_IT` | wejście połączone z `CAN_RX` przez 10 Ω (rezerwa na wybudzanie) |
| PB6 / PB7 | I2C1 SCL / SDA | MCP79410 |
| PC14 | `CLK` | wyjście MFP MCP79410 (32,768 kHz) — LSE w trybie bypass |
| PA13 / PA14 / PB3 | SWD + SWO | Tag-Connect |
| PA0, PA3, PA4, PA8, PB1, PB4, PB5, PC15 | — | niepodłączone |

Płytka nie ma wolnego USART: PA2/PA3 zajmuje flyback, PA9/PA10 sterowanie
transceiverem, PB6/PB7 magistrala I2C.

## 4. Opis urządzenia — zarys `device.yaml`

Zarys do uszczegółowienia w etapie E0; numery parametrów i komend
własnych nadaje `device.yaml`, numery wielkości i czujników — wyłącznie
rejestry wspólne.

**Kanały telemetrii:** `CPM`, `DOSE_RATE`, `DOSE_ACC`, `TUBE_HV`, `MCU_TEMP`,
`VDDA`, `SUPPLY_VOLTAGE` (VIN_SENSE), `SUPPLY_CURRENT` (INA186).

**Czujniki:** `GM_TUBE`, `HV_MONITOR`, `SUPPLY_MON`, `MCU`.

**Parametry własne:**

| Grupa | Parametry |
|---|---|
| Tuba i HV | typ tuby, nastawa HV, dolny i górny limit HV, włączenie HV |
| Detekcja | próg komparatora (DAC1) |
| Pomiar | tryb uśredniania (stałe okno / adaptacyjne), długość okna |
| Alarmy | progi mocy dawki, próg dawki, histereza |
| Kalibracja (`CAL_LOCK`) | współczynnik CPM → µSv/h, czas martwy tuby, wzmocnienie i offset toru HV, toru VIN i toru prądu, czas przewodzenia klucza flybacka (w granicach wkompilowanych) |

**Komendy własne:** `RESET_DOSE` (z bajtem zabezpieczającym),
`READ_RAW_COUNTS` (surowa liczba zliczeń i czas pomiaru z podstawy LSE —
dla metrologii po stronie hosta), `GET_HV_STATUS` (liczba impulsów
flybacka, wypełnienie, przyczyna ostatniego błędu).

**Zdarzenia własne:** `DOSE_ALARM` (poziom, kierunek przejścia),
`HV_FAULT` (przyczyna).

**Dawka skumulowana** przechowywana jest w podtrzymywanej bateryjnie pamięci
SRAM układu MCP79410 i okresowo utrwalana we flashu — zapis przeżywa zanik
zasilania bez zużywania flasha przy każdej aktualizacji.

**Wymagania warstwy wspólnej** (`common/docs/integracja.md` §6), każde
odpowiadające usterce wykrytej przy migracji wcześniejszych węzłów: ramka
identyfikacyjna 28 B z jawnie wyzerowanym buforem; bajty zabezpieczające
`RESET`, `ENTER_BOOTLOADER`, `LOAD_DEFAULTS`; egzekwowanie `CAL_LOCK`;
nadawanie segmentów z oczekiwaniem na miejsce w kolejce; `TransmitFifoPriority`
włączone w `.ioc` i w kodzie; licznik sekwencji telemetrii indeksowany pozycją
kanału; brak odpowiedzi na rozgłoszeniowe `SET_TIME_SYNC`; bitmapa
`CAPABILITIES` zgodna z implementacją.

## 5. Strategia poboru prądu

Węzeł pracuje w trybie Run/Sleep ze stałym nasłuchem magistrali. Pobór
ogranicza się środkami, które nie naruszają dostępności węzła:

| Odbiornik | Środek |
|---|---|
| Rdzeń | MSI 4 MHz z autotrimem do LSE, zakres napięcia Range 2, `WFI` między zdarzeniami |
| ADC | pojedyncze skany wyzwalane programowo co okres pomiaru, ADC wyłączany między skanami; zegar ADC z SYSCLK (bez PLLSAI1) |
| INA186 | włączany na czas pomiaru (`EN_I_SENS` + ustalenie filtra RC τ = 100 µs, ok. 0,5 ms) |
| Wyjście MFP | podciągnięcie R18 10 kΩ pobiera ok. 165 µA (wyjście open-drain, wypełnienie 50 %); przewidziana zmiana na 47–100 kΩ po sprawdzeniu zboczy na PC14 i stabilności LSE |
| Flyback | serie impulsów wyłącznie na żądanie pętli regulacji HV |
| DAC1 | kanał wewnętrzny bez bufora; tryb sample-and-hold jako późniejsza optymalizacja |
| Piny wolne | stan analogowy |
| Pomiar VBAT | nieużywany (mostek pomiarowy wyłączony) |

Największym odbiornikiem pozostaje transceiver w trybie normalnym. Budżet
weryfikowany jest pomiarem własnym węzła w etapie E1 i po każdym etapie
zmieniającym pracę peryferiów.

## 6. Bezpieczeństwo generatora HV

Pętla bang-bang reguluje HV na podstawie odczytu dzielnika. Przerwa w gałęzi
R9/R10 daje odczyt bliski zera, na który pętla odpowiada ciągłym
dostarczaniem energii — napięcie rośnie bez ograniczenia. Wymagania:

- **twardy limit HV wkompilowany w obraz**, niezależny od parametrów;
- **budżet impulsów** na jednostkę czasu, przekroczenie wyłącza generator;
- **kontrola wiarygodności**: każda seria impulsów musi podnieść odczyt HV
  o wartość oczekiwaną z energii serii; brak przyrostu po N seriach wyłącza
  generator i zgłasza `HV_FAULT`;
- **łagodny start** po włączeniu HV;
- HV wyłączone w bootloaderze (PA2 w stanie resetu, R7 trzyma bramkę nisko),
  przy `ENTER_BOOTLOADER`, przy `HV_FAULT` i po każdym resecie do czasu
  odczytu konfiguracji;
- czas przewodzenia klucza w granicach wkompilowanych (nasycenie rdzenia,
  prąd szczytowy klucza).

Uzasadnienie i parametry zostaną zapisane w osobnym ADR przed etapem E4.

## 7. Etapy

| Etap | Zakres | Wymaga | Kryterium zakończenia |
|---|---|---|---|
| **E0** Specyfikacja | podmoduł `common/`; `device.yaml`; konfiguracje CubeMX aplikacji i bootloadera (sekcja 8); szkielet `docs/`; ADR-y: strategia poboru prądu, bezpieczeństwo HV | — | walidator ekosystemu bez błędów, `generate.py --check` aktualne |
| **E1** Uruchomienie sprzętu | szyny zasilania i pobór, SWD, LSE z MFP i autotrim MSI, MCP79410, tory ADC (VIN, I_SENS, VREFINT, temperatura), CAN i terminacja, bootloader i skok do aplikacji, próby wartości R18 | zmontowana płytka | ramki na magistrali, aplikacja wgrana przez bootloader CAN |
| **E2** Warstwa wspólna | moduły firmware z MPCC: protokół, NVM, bezpieczeństwo i autotest, IWDG, czas i `SET_TIME_SYNC`, liczniki, wykrywanie konfliktu adresu | jw. | `canconf` poziom `full`: 0 FAIL; obraz `app-v0.1` w `releases/` |
| **E3** Tor zliczania | COMP1 → LPTIM1, próg z DAC1, licznik 32-bitowy, CPM, `READ_RAW_COUNTS`; badanie generatorem impulsów podanym na `COMP_GM` | generator impulsów | zgodność z zadaną częstotliwością do ok. 10 kCPS; telemetria rejestrowana przez CanSensorLoggerPi bez zmian w rejestratorze |
| **E4** Generator HV | seria w pętli otwartej, pętla zamknięta, łagodny start, limity, `HV_FAULT`; badanie przerwanego dzielnika; badanie blankingu (HV włączone, tuba odłączona — zero zliczeń); tętnienie HV; pobór w funkcji częstości zliczeń | transformator T1 | HV STS-5 stabilne w ±1 %, wszystkie badania bezpieczeństwa zaliczone |
| **E5** Tuba i dozymetria | charakterystyka plateau STS-5, wybór nastawy HV i progu, czas martwy, korekcja czasu martwego, uśrednianie adaptacyjne, `DOSE_RATE`, `DOSE_ACC`, alarmy; współczynnik tymczasowy | tuba | parametry zapisane z `CAL_LOCK` |
| **E6** Hosty | moduł urządzenia w CanSensorHub, narzędzie charakterystyki plateau, dedykowany widok w CanSensorLoggerPi | — | — |
| **E7** Odbiór | praca ciągła 24 h, liczniki po dobie, wersja 1.0, wpis egzemplarza w `node_addresses.yaml` | — | — |

Etap E3 nie zależy od transformatora: tor zliczania badany jest sygnałem
z generatora. Kalibracja dozymetryczna (współczynnik µSv/h względem źródła
lub przyrządu wzorcowego) pozostaje poza zakresem planu do czasu ustalenia
metody.

Każdy obraz wgrany na sprzęt trafia przed wgraniem do `releases/` wraz
z `MANIFEST.txt`, wg zasad wspólnych dla rodziny węzłów.

## 8. Konfiguracja CubeMX

Wartości bazowe odpowiadają sprawdzonej konfiguracji MPCC; odstępstwa
opisane są w kolumnie uwag.

### 8.1 Aplikacja — `firmware/CanGeigerProbe/CanGeigerProbe.ioc`

| Blok | Ustawienie | Uwagi |
|---|---|---|
| RCC | MSI 4 MHz (Range 6) jako SYSCLK; AHB, APB1, APB2 /1 | |
| RCC | Power Regulator Voltage Scale: Range 2 | |
| RCC | LSE nie włączany w CubeMX | bypass z MFP włącza kod, z powrotem do samego MSI przy braku sygnału (wzorzec MPCC) |
| RCC | HSI, PLL, PLLSAI1 wyłączone; zegar ADC: SYSCLK; zegar LPTIM1: PCLK | odstępstwo od MPCC (PLLSAI1) — pobór prądu |
| RTC, IWDG | nie konfigurowane w CubeMX | IWDG konfigurowany w kodzie; czas z MCP79410 |
| CAN1 | Prescaler 1, BS1 6 TQ, BS2 1 TQ, SJW 1 → 500 kbit/s, punkt próbkowania 87,5 % | |
| CAN1 | Transmit FIFO Priority: ENABLE | wymaganie warstwy wspólnej |
| CAN1 | NVIC: CAN1_RX0, CAN1_TX | |
| GPIO | PA9 `CAN_STBY` output, stan początkowy Low | |
| GPIO | PA10 `CAN_R_CTRL` output, Low | terminację ustawia kod wg parametru |
| GPIO | PB0 `EN_I_SENS` output, Low | |
| GPIO | PA15 `CAN_RX_IT` input, bez podciągania, bez EXTI | |
| GPIO | PA2: pull-down | |
| Code Generator | „Set all free pins as analog” | |
| ADC1 | IN10, IN11, IN12, VREFINT, TEMPSENSOR; bez VBAT | |
| ADC1 | scan, Continuous: DISABLE, wyzwalanie programowe, DMA Normal | odstępstwo od MPCC (ciągły, Circular) — pobór prądu |
| ADC1 | oversampling 16×, przesunięcie 2 bity | wzorzec MPCC (pełna skala 16380) |
| ADC1 | czas próbkowania: IN11, VREFINT, TEMP — 640,5; IN10, IN12 — 247,5 | |
| DMA | ADC1 → DMA1 Ch1, halfword; NVIC DMA1_Ch1 | |
| COMP1 | INP = PA1; INM = DAC1 kanał 1 | kanał 2 ma wyjście na PA5 (pomiar HV) |
| COMP1 | Ultra Low Power, histereza Medium, bez okna, bez EXTI | |
| COMP1 | Blanking: TIM2_OC3 | |
| COMP1 | bez wyjścia na pin | COMP1_OUT jest alternatywą PB0 (`EN_I_SENS`) |
| DAC1 | wyłącznie kanał 1, „connected to on-chip peripherals”, bufor wyłączony | kanał 2 usunięty |
| LPTIM1 | clock source internal (PCLK), counter source External, Input1 = COMP1 output, filtr 8 taktów, ARR 0xFFFF | |
| LPTIM1 | NVIC: LPTIM1 global | rozszerzenie licznika do 32 bitów |
| TIM2 | CH3 PWM Generation, prescaler 0, preload ARR i CCR, polaryzacja High | okres i szerokość impulsu ustawia kod; Pulse = 0 |
| TIM2 | DMA: TIM2_UP → DMA1 Ch2, Memory → Peripheral, Normal, word; NVIC DMA1_Ch2 | seria N impulsów sprzętowo, bez przerwań |
| I2C1 | Standard 100 kHz; DMA RX → Ch7, TX → Ch6; NVIC I2C1 EV/ER, DMA1 Ch6/Ch7 | wzorzec MPCC |
| NVIC | PriorityGroup 4, SysTick priorytet 15 | wzorzec MPCC |
| SYS | Debug: Serial Wire (SWO dopuszczalne) | |
| Project | STM32CubeIDE, pakiet FW_L4 jak w MPCC, kod inicjalizacji w `main.c` | |

Po wygenerowaniu, poza CubeMX: skrypt linkera — `FLASH ORIGIN = 0x08004000,
LENGTH = 0x3B800` i sekcja `.noinit (NOLOAD)`; `SCB->VTOR = 0x08004000`
w `USER CODE 1` funkcji `main()`.

### 8.2 Bootloader — `firmware/CanGeigerProbe_BL/CanGeigerProbe_BL.ioc`

Nowy projekt CubeMX, nie kopia `MPCC_BL`: konfiguracja MPCC zawiera USART1
na PB6/PB7 (tu I2C) i piny nieobecne na tej płytce.

| Blok | Ustawienie |
|---|---|
| RCC | MSI 4 MHz jako SYSCLK |
| CAN1 | jak w aplikacji: 500 kbit/s, BS1 6, BS2 1; NVIC CAN1_RX0, CAN1_TX |
| GPIO | PA9 output Low, etykieta **`CAN_S`** (nazwa oczekiwana przez kod wspólny; stan niski = tryb normalny także w ATA6561) |
| GPIO | PA10 output, etykieta **`CAN_R_CTRL`** |
| Pozostałe | bez USART, ADC, TIM2; PA2 w stanie resetu |

Po wygenerowaniu, poza CubeMX: folder połączony `Core/Common` →
`common/bootloader`; `bl_board.h` z szablonu (`BL_NODE_ID 0x30`,
`BL_DEVICE_TYPE 0x0004`, `BL_VER_MINOR 1`); skrypt linkera `FLASH LENGTH = 16K`;
wywołanie `bl_core_run()` w `USER CODE 2`; odmiana bez UART wg
[ADR 0001](adr/0001-bootloader-bez-uart.md).

## 9. Zagadnienia otwarte

- **Skuteczność blankingu TIM2_OC3** — obejmuje wyłącznie czas przewodzenia
  klucza; zakłócenia przy wyłączeniu klucza wymagają sprawdzenia w E4.
