# Etapy E2–E7 — zakres prac

**Stan na: 2026-11-15** · Uzupełnienie tabeli etapów z PLAN.md §7: pakiety
prac, kolejność, badania i kryteria każdego etapu. Procedurę E1 opisuje
[E1_URUCHOMIENIE.md](E1_URUCHOMIENIE.md).

Zasady wspólne dla wszystkich etapów:

- każdy obraz wgrany na sprzęt trafia przed wgraniem do `releases/`
  z `MANIFEST.txt`; po E1 wgrywanie wyłącznie przez bootloader CAN;
- każda zmiana `device.yaml` — `build_revision` + 1, generacja, walidator
  ekosystemu; zdolność deklarowana dopiero z implementacją;
- wyniki pomiarów w `docs/pomiary/E<n>.md`; decyzje — ADR.

## E2 — Warstwa wspólna

Przeniesienie modułów firmware węzła MPCC i dopasowanie do sondy.

| Moduł (MPCC) | Zakres w sondzie |
|---|---|
| `wsc_sched` | bez zmian — scheduler kooperacyjny |
| `wsc_can` | logika nadawania jak w MPCC (`send_wait`, `TX_DROPPED`); filtry sprzętowe `NODE` ∈ {własny, 0xFF} (ADR 0002), przeładowanie po zmianie `NODE_ID`; **`TX_FRAMES` liczone przy zakończeniu nadania z `TXOK` = 1** (przerwanie CAN1_TX), a nie przy zleceniu — dopuszczone przez CanBusCommon ADR 0008 p. 4, daje licznik rzeczywiście nadanych ramek przy włączonej retransmisji |
| `wsc_clock` | LSE bypass z MFP, autotrim MSI, praca awaryjna na samym MSI |
| `wsc_i2c`, `wsc_mcp79410` | bez zmian; dodatkowo odczyt/zapis SRAM (dawka skumulowana, E5) |
| `wsc_nvm_flash`, `wsc_config` | strona NVM `0x0803F800`; tablica parametrów z `protocol_gen` |
| `wsc_safety`, `wsc_selftest`, `wsc_watchdog` | bez zmian; maska POST zgodna z `BOOT.POST_MASK` w `device.yaml` |
| `wsc_adc` | skan na żądanie z DMA Normal zamiast ciągłego (ADR 0002); kanały IN10–IN12, VREFINT, TEMP |
| `wsc_status`, `wsc_app` | dyspozytor komend i zdarzeń, liczniki, wykrywanie konfliktu adresu |
| `wsc_oled`, `wsc_ui`, `wsc_font`, `wsc_pcf8574`, `wsc_afe`, `wsc_sensors`, `drv_sensors` | **nie przenoszone** — sprzęt MPCC |

Elementy nowe względem MPCC (rewizja 2.1):

- odbiorca wspólnej osi czasu poziomu 1 (PLAN.md §4): reguła wyboru źródła,
  `UNIX` + `MS` odniesione do licznika milisekund, zapis sekund MCP79410 na
  pełnej sekundzie, `TIME_SOURCE`/`TIME_AGE_S` w 13-bajtowym
  `GET_STATUS_EXT`, `HEARTBEAT.FLAGS.TIME_SYNCED`, `GET_TIME`/`SET_TIME`
  binarnie;
- kanały i czujniki sondy: `SUPPLY_VOLTAGE`, `SUPPLY_CURRENT` (impulsowe
  włączanie INA186), `MCU_TEMP`, `VDDA`; kanały radiometryczne z `VALID` = 0.

Kolejność: scheduler, zegary, CAN i protokół uniwersalny → NVM i parametry →
bezpieczeństwo, autotest, IWDG → ADC i kanały → czas.

Badanie: `canconf` 1.1.0 poziom `full` na magistrali z MPSWP i MPCC,
w tym grupa `SYN` z wzorcem czasu symulowanym przez host.

Przegląd ręczny `CONFORMANCE.md` (CanBusCommon) — m.in. `TransmitFifoPriority`
i automatyczna retransmisja (ADR 0008) w `main.c` obu projektów.

Kryterium: 0 FAIL, lista kontrolna przeglądu ręcznego odhaczona, obraz
`app-v0.1` w `releases/`.

## E3 — Tor zliczania

| Pakiet | Zakres |
|---|---|
| Polaryzacja komparatora | COMP1 Output Polarity = **Inverted** (ustawione w `.ioc`): spoczynek niski, impuls wysoki. Blanking wymusza stan niski, więc przy polaryzacji odwrotnej nie tworzy zboczy, a LPTIM liczy zbocze narastające na początku impulsu |
| Licznik 32-bitowy | LPTIM1 ARR 0xFFFF, przerwanie ARRM zwiększa starsze słowo; odczyt spójny (podwójny odczyt CNT, zgodnie z RM0394) |
| Próg | `COMP_THRESHOLD` (mV) → DAC1 względem zmierzonego VDDA |
| CPM | okno przesuwne z sekundowych przyrostów; `AVG_MODE` stałe/adaptacyjne, `AVG_WINDOW_S` |
| Korekcja czasu martwego | n = m / (1 − m·τ), τ = `CAL_DEAD_TIME_US` |
| `READ_RAW_COUNTS` | zliczenia i czas od startu z podstawy LSE |
| `RADIOMETRY` | deklaracja zdolności w `device.yaml` |

Badanie generatorem impulsów na węźle SIGNAL (amplituda 3,3 V → 1,4 V, czas
impulsu 50–200 µs):

1. częstotliwości 0,1 Hz – 10 kHz: zgodność liczby zliczeń z zadaną;
2. próg: przemiatanie `COMP_THRESHOLD`, wyznaczenie okna poprawnej detekcji;
3. impulsy krótsze niż filtr LPTIM (< 2 µs) — odrzucane;
4. odczyt `READ_RAW_COUNTS` przez host — czestość zgodna z generatorem.

Kryterium: błąd < 0,1 % do 10 kCPS (bez korekcji czasu martwego),
telemetria rejestrowana przez CanSensorLoggerPi bez zmian w rejestratorze.

## E4 — Generator HV

Wymaga transformatora T1. Mechanizmy bezpieczeństwa — ADR 0003.

**Bezpieczeństwo pracy:** C15 (100 nF/1 kV) rozładowuje się przez dzielnik
1 GΩ ze stałą czasową ok. 100 s. Przed dotknięciem sekcji HV — rozładowanie
przez rezystor 10 MΩ/1 kV na izolowanym uchwycie i pomiar sondą HV.

| Krok | Zakres |
|---|---|
| 1. Transformator | pomiar Lp (oczek. 75 µH), przekładni (1:30), indukcyjności rozproszenia; sprawdzenie kropek |
| 2. Pętla otwarta | tuba odłączona, obciążenie zastępcze; pojedyncze serie N impulsów o rosnącym czasie przewodzenia; przyrost HV na serię, Vds na Q1 (clamp ok. 20 V), prąd szczytowy (nasycenie rdzenia) |
| 3. Stałe ADR 0003 | limit wkompilowany, budżet impulsów, N i tolerancja przyrostu, granice czasu przewodzenia — uzupełnienie ADR 0003; parametr czasu przewodzenia dopisany do `device.yaml` |
| 4. Pętla zamknięta | bang-bang z odczytem HV, łagodny start, `HV_SETPOINT`, `HV_MIN`, `HV_MAX`, `GET_HV_STATUS` |
| 5. Badania zabezpieczeń | przerwany dzielnik (symulowany odłączeniem R11 przez przełącznik — odczyt zero) → `NO_RISE`; zwarcie wyjścia przez 10 MΩ → `NO_RISE`/`UNDERVOLTAGE`; nastawa ponad limit → odrzucona; zawieszenie pętli → IWDG, bramka niska |
| 6. Blanking | HV włączone, tuba odłączona, próg roboczy: zero zliczeń przez 10 min. Sprawdzenie zakłóceń przy wyłączeniu klucza (PLAN.md §9) |
| 7. Jakość HV | tętnienie, stabilność w funkcji Vin 7–32 V i obciążenia (symulowana częstość zliczeń) |
| 8. Pobór | prąd węzła w funkcji częstości zliczeń |

Kryterium: HV dla STS-5 stabilne w ± 1 %, wszystkie badania kroku 5 i 6
zaliczone.

## E5 — Tuba i dozymetria

| Pakiet | Zakres |
|---|---|
| Plateau | STS-5 z R12 = 4,7 MΩ: HV 340–480 V co 10 V, źródło kontrolne albo tło z odpowiednio długim czasem zliczania (błąd statystyczny ≤ 3 %); narzędzie hosta z E6 albo skrypt na `tools/protocol_gen.py` |
| Nastawa | początek plateau + ok. 1/3 jego długości; zapis `HV_SETPOINT`, `HV_MIN`, `HV_MAX` |
| Próg | przemiatanie `COMP_THRESHOLD` przy nastawie roboczej — najniższy próg bez zliczeń szumu |
| Czas martwy | pomiar oscyloskopem (odstęp impulsów przy dużej częstości) albo metodą dwóch źródeł; zapis `CAL_DEAD_TIME_US` |
| Moc dawki | `DOSE_RATE` = CPM_kor × `CAL_CPM_TO_USVH` (wartość tymczasowa do czasu kalibracji) |
| Dawka skumulowana | całkowanie w SRAM MCP79410 (podtrzymanie bateryjne), utrwalanie we flashu co ustalony okres; `RESET_DOSE` |
| Alarmy | `DOSE_ALARM` z histerezą `ALARM_HYST`, progi `ALARM_*` |
| DOI-80 | powtórzenie plateau po zmianie R12 na 5,1 MΩ i `TUBE_TYPE` = 1 (jeśli tuba dostępna) |

Kryterium: parametry zapisane z `CAL_LOCK` = 1; kalibracja dozymetryczna
pozostaje poza planem.

## E6 — Hosty

- **CanSensorHub**: moduł urządzenia (komendy `RESET_DOSE`, `READ_RAW_COUNTS`,
  `GET_HV_STATUS`, zdarzenia `DOSE_ALARM`, `HV_FAULT`, parametry);
  warstwa uniwersalna z `mpcan`.
- **Narzędzie charakterystyki plateau**: przemiatanie HV z zapisem CSV
  i wykresem; wykorzystuje `READ_RAW_COUNTS`.
- **CanSensorLoggerPi**: dedykowany widok (moc dawki, CPM, dawka, HV);
  zapis działa bez zmian od E3 dzięki rejestrom wspólnym.

## E7 — Odbiór

1. Praca ciągła 24 h na wspólnej magistrali: liczniki błędów, restartów
   i `HV_FAULT` po dobie; stabilność HV i CPM tła.
2. `canconf` `full` na obrazie końcowym — 0 FAIL.
3. Wersja `app-v1.0` (`fw_version_major` = 1) w `releases/`.
4. Ewidencja: wpis egzemplarza w `node_addresses.yaml` (sekcja
   `assignments`: adres, UID, wersja), status typu w `device_types.yaml`
   z `planned` na aktywny — zgłoszenie do skrzynki
   `../CanBusCommon/.local/zgloszenia/`; dopisanie wykonuje sesja
   CanBusCommon.
5. Aktualizacja README i PLAN.md.
