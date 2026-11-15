# ADR 0003 — Bezpieczeństwo generatora HV

**Stan:** przyjęta (mechanizmy); wartości liczbowe do ustalenia w E4 po
pomiarach transformatora
**Data:** 2026-11-15
**Dotyczy:** firmware aplikacji (moduł generatora HV), TIM2_CH3 (PA2),
ADC1 IN10 (PA5), parametry `HV_*`, zdarzenie `HV_FAULT`

## Kontekst

Napięcie anodowe tuby (ok. 400 V) wytwarza flyback bez dedykowanego
kontrolera: TIM2 generuje serię impulsów bramki klucza, ADC mierzy HV przez
dzielnik 1:1001, a firmware zamyka pętlę bang-bang. Za bezpieczeństwo
odpowiada więc wyłącznie oprogramowanie.

Najgroźniejsza usterka to przerwa w gałęzi dzielnika pomiarowego (R9/R10).
Odczyt spada wtedy do zera, pętla odpowiada ciągłym dostarczaniem energii,
a napięcie rośnie bez ograniczenia — do przebicia kondensatora wyjściowego,
tuby albo klucza. Podobny skutek ma zawieszenie pętli z kluczem w stanie
przewodzenia albo błędna nastawa.

Transformator nie jest jeszcze dostępny, więc wartości liczbowe (energia
serii, przyrost HV na serię, dopuszczalny czas przewodzenia) muszą poczekać
na pomiary.

## Rozważane warianty

| Wariant | Ocena |
|---|---|
| A. Wyłącznie limit nastawy (`HV_MAX`) | odrzucony — nie wykrywa przerwy dzielnika (odczyt zero) |
| B. Limity + budżet impulsów + kontrola wiarygodności przyrostu | przyjęty |
| C. Sprzętowy ogranicznik HV | poza zakresem — płytka zamówiona, rozważany w następnej rewizji |

## Decyzja

1. **Twardy limit HV wkompilowany w obraz**, niezależny od parametrów
   i nadrzędny wobec `HV_MAX`. Przekroczenie wyłącza generator
   (`HV_FAULT` `OVERVOLTAGE`).
2. **Budżet impulsów:** stała wkompilowana — maksymalna liczba impulsów klucza
   na sekundę. Przekroczenie wyłącza generator (`PULSE_BUDGET`).
3. **Kontrola wiarygodności:** każda seria impulsów musi podnieść odczyt HV
   o wartość oczekiwaną z jej energii (z tolerancją). Brak przyrostu przez
   N kolejnych serii wyłącza generator (`NO_RISE`). Wykrywa przerwę
   dzielnika, zwarcie wyjścia i uszkodzenie transformatora.
4. **Łagodny start:** po włączeniu nastawa narasta stopniowo; `HV_MIN`
   sprawdzane dopiero po zakończeniu narastania (`UNDERVOLTAGE`).
5. **Stan bezpieczny domyślnie:** HV wyłączone w bootloaderze (PA2 w stanie
   po resecie, R7 trzyma bramkę nisko), po każdym resecie do czasu odczytu
   konfiguracji, przy `ENTER_BOOTLOADER`, przy `HV_FAULT`. `HV_ENABLE`
   domyślnie 0; po `HV_FAULT` generator pozostaje wyłączony do ponownego
   zapisu `HV_ENABLE` = 1.
6. **Czas przewodzenia klucza** ograniczony wartościami wkompilowanymi
   (nasycenie rdzenia, prąd szczytowy klucza). Parametr strojenia zostanie
   dopisany do `device.yaml` w E4 i obcinany do tych granic.
7. **Seria sprzętowa:** liczba impulsów serii ustalana przez DMA TIM2_UP
   (bufor długości N); po ostatnim impulsie wypełnienie wraca do zera
   sprzętowo, więc zawieszenie oprogramowania nie pozostawia klucza
   w przewodzeniu. IWDG resetuje węzeł do stanu bezpiecznego.

## Konsekwencje

- Moduł HV powstaje w E4 razem z badaniami: przerwany dzielnik (symulowany),
  zwarcie wyjścia, blanking komparatora, tętnienie HV.
- Wartości: limit wkompilowany, budżet impulsów, N i tolerancja przyrostu,
  granice czasu przewodzenia — uzupełnienie tego ADR po pomiarach
  transformatora.
- `GET_HV_STATUS` udostępnia stan generatora, ostatnią przyczynę wyłączenia,
  odczyt HV, liczbę impulsów i wypełnienie — dane do strojenia i diagnostyki.
