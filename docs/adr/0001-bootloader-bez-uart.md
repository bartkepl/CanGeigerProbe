# ADR 0001 — Bootloader bez transportu UART

**Stan:** zastąpiona przez [CanBusCommon ADR 0006](https://github.com/bartkepl/CanBusCommon/blob/main/docs/adr/0006-bootloader-wylacznie-can.md) (2026-11-15)
**Data:** 2026-11-15

!!! note "Zastąpiona"
    Rewizja 2.1 standardu usunęła transport UART ze źródeł wspólnych
    (CanBusCommon ADR 0006). Obejście opisane niżej jest zbędne: projekt
    bootloadera sondy dołącza `common/bootloader` bez zmian, bez zaślepki
    `bl_uart_none.c` i bez symbolu `HAL_UART_MODULE_ENABLED`, a w `USER CODE 2`
    wywołuje `bl_core_run(&hcan1);`. Treść poniżej zostaje jako zapis
    historyczny.
**Dotyczy:** `firmware/CanGeigerProbe_BL`, podmoduł `common/bootloader`

## Kontekst

Wspólny bootloader (`common/bootloader`) obsługuje dwa transporty: CAN
i UART. Transport UART pochodzi z węzłów MPSWP i MPCC, gdzie pełnił rolę
zapasowego kanału programowania.

Sonda nie ma wolnego USART: PA2/PA3 zajmuje sterowanie flybackiem, PA9/PA10
sterowanie transceiverem CAN i terminacją, PB6/PB7 magistrala I2C układu
MCP79410. Jedyną drogą programowania poza SWD jest CAN.

Źródła wspólne zakładają obecność UART: `bl_core_run()` przyjmuje
`UART_HandleTypeDef *`, wywołuje `HAL_UART_DeInit()` przed skokiem do
aplikacji oraz `bl_uart_setup()` i `bl_uart_poll()` w pętli. Moduł HAL UART
nie jest kompilowany, gdy CubeMX nie konfiguruje żadnego USART.

CanBusCommon jest standardem i nie jest modyfikowany na potrzeby jednego
węzła. Rozbieżność odnotowana jest w repozytorium wspólnym w
`NOTATKA_BOOTLOADER_UART.md` jako materiał do przyszłej rewizji standardu.

## Rozważane warianty

| Wariant | Ocena |
|---|---|
| A. Zmiana źródeł wspólnych (przełącznik `BL_HAS_UART`) | odrzucony — wymaga zmiany standardu |
| B. Własna kopia `bl_core.c` w repozytorium sondy | odrzucony — rozejście się z poprawkami wspólnymi, ta sama klasa usterki co okno „knock” w MPCC |
| C. Źródła wspólne bez zmian, transport UART zastąpiony lokalną zaślepką | przyjęty |

## Decyzja

Bootloader sondy nie ma transportu UART. Źródła wspólne dołączane są bez
modyfikacji, z wyjątkiem pliku `bl_uart.c`, wyłączonego z budowy:

- `bl_uart_none.c` w projekcie bootloadera implementuje `bl_uart_setup()`
  i `bl_uart_poll()` jako operacje puste (`bl_uart_poll()` zwraca `false`,
  więc nie liczy się jako „knock”);
- `bl_core_run(&hcan1, NULL)` — `HAL_UART_DeInit(NULL)` zwraca `HAL_ERROR`
  bez dostępu do peryferium, a wynik jest ignorowany przez kod wspólny;
- symbol `HAL_UART_MODULE_ENABLED` zdefiniowany w ustawieniach preprocesora
  projektu (obie konfiguracje budowy), nie w `stm32l4xx_hal_conf.h`, który
  CubeMX nadpisuje przy regeneracji. Zapewnia typ `UART_HandleTypeDef`
  i symbol `HAL_UART_DeInit` wymagane przez kod wspólny; nieużywany kod
  sterownika usuwa linker.

## Konsekwencje

- Kontrakt wire bootloadera (v1.0/v1.1) bez zmian; narzędzia hosta
  programują sondę przez CAN jak pozostałe węzły.
- Utrata komunikacji CAN oznacza programowanie przez SWD (Tag-Connect).
  Sonda nie jest zalewana, więc SWD pozostaje dostępne.
- Zaślepka i definicja symbolu stają się zbędne po usunięciu transportu UART
  ze standardu; wtedy projekt bootloadera wraca do samych źródeł wspólnych.
