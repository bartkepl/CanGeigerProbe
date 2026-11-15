# ADR 0002 — Strategia poboru prądu przy stałym nasłuchu magistrali

**Stan:** przyjęta; wartość R18 i budżet do potwierdzenia pomiarem w E1
**Data:** 2026-11-15
**Dotyczy:** firmware aplikacji, konfiguracja CubeMX (`CanGeigerProbe.ioc`),
R18, filtry CAN

## Kontekst

Projekt sprzętowy zakładał pracę mikrokontrolera w trybie Stop 2
z wybudzaniem przez RTC i aktywność magistrali. Sonda pracuje jednak na
**wspólnej magistrali** z węzłami MPSWP i MPCC, które nadają telemetrię
cyklicznie. Z tego wynika:

- bxCAN nie pracuje w Stop 2. Pierwsza ramka po wybudzeniu przez zbocze na
  `CAN_RX` przepada, więc węzeł uśpiony gubi zapytania hostów i zdarzenia
  synchronizacji czasu;
- ruch innych węzłów budziłby sondę na bieżąco, a koszt każdego wybudzenia
  (start zegara, ponowna inicjalizacja CAN) przewyższa zysk z uśpienia;
- transceiver ATA6561 musi pozostać w trybie normalnym, bo tryb standby nie
  odbiera ramek do kontrolera.

Pobór ma być mimo to możliwie niski. Wymaganie nie jest twarde („miło by
było”), ale ma pierwszeństwo przed wygodą implementacji wszędzie tam, gdzie
nie narusza dostępności węzła.

## Rozważane warianty

| Wariant | Ocena |
|---|---|
| A. Stop 2, wybudzanie zboczem `CAN_RX` (PA15) i RTC | odrzucony — gubienie pierwszej ramki, ciągłe wybudzenia przez ruch innych węzłów |
| B. Run/Sleep z `WFI`, peryferia w trybach oszczędnych, filtry sprzętowe CAN | przyjęty |
| C. Run bez `WFI` | odrzucony — bez korzyści |

## Decyzja

Węzeł pracuje w trybie Run/Sleep ze stałym nasłuchem:

1. **Zegar rdzenia:** MSI 4 MHz dostrojony do LSE (bypass z wyjścia MFP
   MCP79410), regulator w zakresie Range 2, `WFI` między zdarzeniami
   (parametr uniwersalny `LOW_POWER` = 1). PLL i PLLSAI1 nieużywane, zegar ADC
   z SYSCLK.
2. **Filtry CAN:** dwa filtry 32-bitowe z maską `0x00FF0000` przyjmują
   wyłącznie ramki z polem `NODE` równym własnemu adresowi albo `0xFF`.
   Obejmuje to zapytania do węzła, rozgłoszenia (`SET_TIME_SYNC`, wykrywanie)
   i ramki innego węzła nadającego z tym samym adresem (wykrywanie konfliktu).
   Telemetria pozostałych węzłów nie budzi rdzenia. Filtry przeładowywane są
   po zmianie `NODE_ID`.
3. **ADC:** pojedyncze skany wyzwalane programowo co `MEASURE_PERIOD`, DMA
   w trybie Normal, ADC wyłączany między skanami (odstępstwo od MPCC:
   konwersja ciągła z DMA Circular).
4. **INA186:** włączany (`EN_I_SENS`) tylko na czas pomiaru, z odczekaniem
   ok. 0,5 ms na ustalenie filtra RC.
5. **Zliczanie impulsów:** sprzętowe (COMP1 → LPTIM1), bez przerwania na
   każdy impuls; przerwanie tylko przy przepełnieniu licznika 16-bitowego.
6. **DAC1:** kanał 1 wewnętrzny, bez bufora wyjściowego.
7. **Wyjście MFP:** R18 (10 kΩ) pobiera średnio ok. 165 µA przy wypełnieniu
   50 %. Przewidziana zmiana na 47–100 kΩ po sprawdzeniu w E1 zboczy na PC14
   i stabilności LSE.
8. **Piny wolne** w stanie analogowym, pomiar VBAT nieużywany.

## Konsekwencje

- Największym odbiornikiem pozostaje transceiver w trybie normalnym; budżet
  mierzony jest torem własnym węzła (INA186) w E1 i po każdym etapie
  zmieniającym pracę peryferiów.
- Odsyłacz do Stop 2 w opisie sprzętowym (README) zastąpiony opisem trybu
  Run/Sleep.
- Filtry sprzętowe wykluczają odbiór zdarzeń poziomu 2 wspólnej osi czasu
  (`NODE` = adres wzorca). Węzeł poziomu 2 nie deklaruje; ewentualne włączenie
  wymaga trzeciego filtra.
- Pin PA15 (`CAN_RX_IT`) pozostaje wejściem bez EXTI, jako rezerwa sprzętowa.
