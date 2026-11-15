# E1 — Uruchomienie sprzętu

**Stan na: 2026-11-15** · Dotyczy: pierwszy egzemplarz płytki, bez
transformatora T1 i bez tuby G-M.

Procedura uruchomienia płytki od pierwszego zasilenia do aplikacji wgranej
przez bootloader CAN. Kolejność kroków jest celowa: każdy krok korzysta tylko
z elementów sprawdzonych wcześniej. Wyniki zapisywane są w
`docs/pomiary/E1.md` (tabela na końcu dokumentu) — wartość zmierzona, data,
przyrząd.

Kryterium zakończenia etapu (PLAN.md §7): ramki na magistrali, aplikacja
wgrana przez bootloader CAN, wyniki kroków 1–11 zapisane.

## Warunki wstępne

| Element | Stan wymagany |
|---|---|
| T1 (transformator flybacka) | **niezamontowany** — generator HV nie może wytworzyć napięcia |
| Tuba G-M | niezamontowana |
| F1 (PTC) | wartość testowa 100 mA (docelowo 500 mA) |
| Zasilacz | laboratoryjny, ograniczenie prądu 30 mA, start od 12 V |
| Przyrządy | multimetr, oscyloskop (≥ 50 MHz), ST-LINK z Tag-Connect, adapter CAN hosta (CanSensorHub / `mpcan`), generator funkcyjny |
| Firmware | `CanGeigerProbe_BL` (wydanie `bl-v1.1`) oraz obraz uruchomieniowy aplikacji `app-v0.0.x` (niżej) |

Obraz uruchomieniowy aplikacji to aplikacja z E0 rozszerzona o funkcje
diagnostyczne kroków 5–10 (odczyty ADC, I2C, zegary) dostępne przez komendy
CAN. Każdy wgrany obraz trafia przed wgraniem do `releases/` z
`MANIFEST.txt`.

## 1. Kontrola przed zasileniem

1. Oględziny: zwarcia cyną, orientacja U1, IC1, U2, IC2, IC8, U701/U702, D7.
2. Rezystancje do GND (płytka bez zasilania, multimetr):

   | Węzeł | Oczekiwane |
   |---|---|
   | Vin (J1.1) | > 1 MΩ po ustaleniu (C20 się ładuje) |
   | +5 V | > 10 kΩ |
   | +3,3 V | > 1 kΩ |
   | CANH–CANL | > 10 kΩ — rezystancja wejściowa transceivera (terminacja wyłączona: R24 trzyma Q2 zamknięty) |

3. Węzeł HV (C15) — rezystancja do GND ≈ 1 GΩ (dzielnik R9+R10+R11); niższa
   wartość oznacza upływ albo błędny element.

## 2. Pierwsze zasilenie

1. Vin = 12 V, ograniczenie 30 mA. Prąd spoczynkowy pustego MCU (pamięć flash
   czysta) zapisać jako odniesienie.
2. Napięcia: +5 V = 4,94 V ± 2 % (dzielnik FB 1M/187k), +3,3 V = 3,30 V ± 2 %.
3. Tętnienie +5 V oscyloskopem (Burst Mode LT8606) — zapisać amplitudę
   i częstotliwość paczek.
4. Zakres Vin: zmniejszać Vin do chwili zaniku +5 V i zwiększać do 32 V.
   Start i praca od 7 V wymagane (próg EN/UV — PLAN.md §2 projektu
   sprzętowego).
5. Odwrotna polaryzacja (krótko, ograniczenie 30 mA): brak prądu, brak
   uszkodzeń (D7).

## 3. SWD i opcje

1. ST-LINK przez Tag-Connect: odczyt identyfikatora (STM32L43x/44x,
   DEV_ID 0x435), rozmiar flash 256 KB.
2. Option bytes: RDP Level 0; `nBOOT0`/`nSWBOOT0` tak, by start następował
   z flash (PH3 z R17 do GND); `nRST_STOP`/`nRST_STDBY` bez zmian; IWDG
   programowy (`IWDG_SW` = 1). Zapisać odczytane wartości.
3. Masowe kasowanie.

## 4. Bootloader i magistrala

1. Wgrać `CanGeigerProbe_BL` przez SWD (od `0x08000000`).
2. Bez aplikacji bootloader pozostaje aktywny: `CAN_S` (PA9) niski, terminacja
   (PA10) wysoka. Zmierzyć CANH–CANL bez zasilania magistrali: ≈ 120 Ω
   (2 × 59 Ω + Ron PhotoMOS).
3. Podłączyć do magistrali z hostem (sama sonda + host, potem z MPSWP i MPCC).
   Host: `HELLO` na adres 0x30 — odpowiedź z `BL_VER` 1.1, `DEVICE_TYPE`
   0x0004, `NODE_ID` 0x30, `APP_VALID` = 0.
4. Oscyloskopem: poziomy CANH/CANL (recesywny ≈ 2,5 V, dominujący ≈ 3,5/1,5 V),
   czas bitu 2 µs, punkt próbkowania zgodny z innymi węzłami (brak ramek
   błędów w liczniku hosta).

## 5. Aplikacja przez bootloader CAN

1. Wgrać obraz uruchomieniowy narzędziem hosta przez bootloader (nie SWD):
   `ERASE`, `PROG_*`, `VERIFY`, `GO`.
2. Po resecie: okno „knock” 250 ms, skok do aplikacji, ramka `BOOT`
   i `HEARTBEAT` na magistrali.
3. `ENTER_BOOTLOADER` (bajt `0xB0`) z aplikacji — powrót do bootloadera
   i ponowne wgranie tego samego obrazu. Ten krok potwierdza całą drogę
   aktualizacji bez SWD.

## 6. MCP79410 i zegar LSE z wyjścia MFP

1. I2C1 (100 kHz): odpowiedź RTC 0x6F i EEPROM 0x57. Zapis i odczyt rejestru
   SRAM (0x20–0x5F).
2. Uruchomienie oscylatora RTC (`ST`), wyjście MFP 32,768 kHz (`SQWEN`,
   `SQWFS` = 32,768 kHz).
3. PC14 oscyloskopem przy R18 = 10 kΩ: zbocze narastające (open-drain +
   R18 + pojemność), wypełnienie, amplituda. Zapisać czas narastania 10–90 %.
4. LSE w trybie bypass: `LSERDY` w czasie < 1 s; przy braku sygnału aplikacja
   zostaje na samym MSI i zgłasza to w statusie.
5. Autotrim MSI (`MSIPLLEN`): odchyłka MSI względem LSE — pomiar timerem
   taktowanym MSI przez okno 1 s z LSE (oczekiwane < 0,1 %).
6. **Próby R18** (ADR 0002): 10 kΩ → 47 kΩ → 100 kΩ. Dla każdej wartości:
   czas narastania na PC14, stabilność `LSERDY` w ciągu 10 min, odchyłka MSI,
   prąd całkowity węzła. Wybrać największą wartość, przy której LSE pracuje
   stabilnie; wynik dopisać do ADR 0002.

## 7. Tory ADC

Wszystkie odczyty z oversamplingiem 16× (pełna skala 16380), przeliczone
ratiometrycznie względem VREFINT.

| Tor | Sprawdzenie | Oczekiwane |
|---|---|---|
| VREFINT → VDDA | porównanie z multimetrem na +3,3 V | ± 1 % |
| Temperatura MCU | kalibracja fabryczna TS_CAL1/TS_CAL2 | ± 3 °C względem otoczenia |
| VIN_SENSE (PA6) | Vin = 7, 12, 24, 32 V względem multimetru | błąd < 2 % przed kalibracją; wyznaczyć `CAL_VIN_C0/C1` |
| I_SENS (PA7) | `EN_I_SENS` = 0: offset; = 1: porównanie z amperomierzem szeregowym przy kilku obciążeniach | ok. 2 µA na działkę; wyznaczyć `CAL_ISENS_C0/C1` |
| I_SENS — ustalenie | odczyty co 0,1 ms po włączeniu `EN_I_SENS` | stała τ = 100 µs, ustalenie ≤ 0,5 ms (ADR 0002) |
| HV_meas (PA5) | bez T1: odczyt ≈ 0; podanie 0,1–0,5 V przez rezystor 10 kΩ na PA5 | liniowość toru; D3 nie przewodzi poniżej 3,3 V |

## 8. Magistrala z innymi węzłami

1. Sonda na magistrali z MPSWP i MPCC: brak ramek błędów, liczniki TEC/REC
   bliskie zeru po 10 min.
2. Terminacja: parametr `CAN_TERMINATION` 0/1 — CANH–CANL w obu stanach
   (zmiana o 120 Ω).
3. Filtry sprzętowe (ADR 0002): ramki innych węzłów nie wywołują przerwania
   RX (licznik przerwań w obrazie uruchomieniowym), zapytania do 0x30
   i rozgłoszenia — tak.

## 9. Tor zliczania — sprawdzenie wstępne

Pełne badanie należy do E3. Tu tylko potwierdzenie, że tor działa:

1. DAC1 kanał 1 = 2,4 V (`COMP_THRESHOLD` domyślny) (wewnętrzny — pomiar pośredni przez zachowanie
   komparatora).
2. Generator funkcyjny przez 10 kΩ na węzeł SIGNAL (za C19): prostokąt
   3,3 V → 1,4 V (amplituda impulsu z tuby), impulsy 50 µs, 10 Hz. LPTIM1 liczy 10 impulsów/s.
3. Sprawdzić polaryzację wyjścia komparatora (PLAN.md §9): stan spoczynkowy
   wyjścia COMP1 musi być niski.

## 10. Sterowanie kluczem flybacka (bez T1)

1. PA2 w spoczynku niski (pull-down + R7).
2. Seria N = 5 impulsów z DMA TIM2_UP, szerokość 5 µs: oscyloskop na bramce
   Q1 (za R6). Liczba impulsów, szerokość i powrót do zera po serii.
3. Reset w trakcie serii (IWDG albo przycisk SWD): bramka wraca do zera.

## 11. Pobór prądu

Pomiar amperomierzem zasilacza i torem INA186, Vin = 12 V i 24 V:

| Stan | Oczekiwane (ADR 0002) |
|---|---|
| Aplikacja, nasłuch, `WFI`, terminacja wyłączona | do ustalenia — wartość odniesienia dla E2–E7 |
| jw., terminacja włączona | + ok. 4 mA (LED PhotoMOS) |
| Nadawanie telemetrii co 1 s | szczyty do ok. 20 mA przy 7 V |
| Bootloader | do ustalenia |

## Zapis wyników — `docs/pomiary/E1.md`

| Krok | Wielkość | Oczekiwane | Zmierzone | Przyrząd | Data | Uwagi |
|---|---|---|---|---|---|---|
| 2.2 | +5 V | 4,94 V | | | | |
| … | | | | | | |

Rozbieżności z oczekiwaniami trafiają do PLAN.md §9 albo — gdy zmieniają
decyzję — do ADR.
