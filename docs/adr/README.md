# Rejestr decyzji (ADR)

Decyzje projektowe dotyczące wyłącznie tego repozytorium. Decyzje obejmujące
kilka repozytoriów rodziny zapisywane są w `common/docs/adr`.

Format: nagłówek z numerem, **Stan / Data / Dotyczy**, Kontekst, Rozważane
warianty, Decyzja, Konsekwencje.

| Nr | Tytuł | Stan |
|---|---|---|
| [0001](0001-bootloader-bez-uart.md) | Bootloader bez transportu UART | zastąpiona przez CanBusCommon ADR 0006 |
| [0002](0002-strategia-poboru-pradu.md) | Strategia poboru prądu przy stałym nasłuchu magistrali | przyjęta; R18 i budżet do potwierdzenia w E1 |
| [0003](0003-bezpieczenstwo-generatora-hv.md) | Bezpieczeństwo generatora HV | przyjęta (mechanizmy); wartości w E4 |

Decyzje warstwy wspólnej, z których węzeł korzysta wprost:
ADR 0006 CanBusCommon (bootloader wyłącznie CAN) i ADR 0007 CanBusCommon
(wspólna oś czasu — sonda jako odbiorca poziomu 1).
