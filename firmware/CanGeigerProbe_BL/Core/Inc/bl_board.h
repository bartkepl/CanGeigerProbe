/**
 * @file    bl_board.h
 * @brief   Konfiguracja bootloadera CanGeigerProbe.
 *
 *  Jedyny plik bootloadera zyjacy w repozytorium urzadzenia — pozostale
 *  pochodza z podmodulu common/bootloader/ (szablon: bl_board.h.template).
 *
 *  Kod bootloadera jest pin-agnostyczny: makra pinow (CAN_R_CTRL_Pin,
 *  CAN_S_Pin) pochodza z main.h generowanego przez CubeMX. Opis pinow nizej
 *  sluzy diagnostyce bez zagladania do .ioc.
 */
#ifndef BL_BOARD_H
#define BL_BOARD_H

/* ---------------------------------------------------------------- wezel ---- */
/*  Adres, na ktorym bootloader odpowiada. WKOMPILOWANY w obraz i NIEZALEZNY od
 *  NODE_ID aplikacji — narzedzie flashujace musi znac oba.
 *  Wartosc bierze sie z registry/device_types.yaml (pole bootloader_node);
 *  walidator sprawdza zgodnosc.
 *
 *  UWAGA: adres bootloadera musi byc UNIKALNY na magistrali. Dwa bootloadery
 *  o tym samym adresie odpowiadalyby jednoczesnie na te sama komende. */
#define BL_NODE_ID       0x30u

/*  DEVICE_TYPE z registry/device_types.yaml. Zwracany w odpowiedzi HELLO
 *  (bajty [6:7]) od wersji BL 1.1 — pozwala narzedziu flashujacemu upewnic
 *  sie, ze nie wgrywa obrazu innego wezla do sondy. */
#define BL_DEVICE_TYPE   0x0004u

/* ----------------------------------------------------------- wersja wire --- */
/*  1 = HELLO wypelnia node_id i device_type (v1.1). Pola v1.0 bez zmian,
 *  wiec stary host dziala dalej. */
#define BL_VER_MINOR     1u

/* ------------------------------------------------------------------ piny --- */
/*  Dokumentacja, nie konfiguracja — faktyczne przypisanie robi CubeMX (.ioc).
 *
 *  CanGeigerProbe: CAN_R_CTRL = PA10 (terminacja split przez PhotoMOS,
 *  HIGH = zalaczona), CAN_S = PA9 (ATA6561 STBY, LOW = praca normalna).
 *
 *  Bootloader nie uzywa zadnego USART (ADR 0006 CanBusCommon) — jedynym
 *  transportem jest CAN, a droga odzysku poza CAN jest SWD (Tag-Connect).
 *
 *  Generator HV: PA2 (FBACK_CTRL, TIM2_CH3) pozostaje w stanie po resecie
 *  (wejscie analogowe), a R7 trzyma bramke klucza nisko — HV jest wylaczone
 *  przez caly czas pracy bootloadera.
 *
 *  UWAGA: bootloader zalacza terminacje BEZWARUNKOWO na czas swojej pracy
 *  (bl_core.c / enable_bus). Dla wezla w SRODKU magistrali oznacza to trzecie
 *  zakonczenie na czas programowania. Przy dlugiej magistrali warto to
 *  sprawdzic oscyloskopem przed pierwszym zdalnym flashowaniem. */
#define BL_BOARD_NAME    "CanGeigerProbe"

#endif /* BL_BOARD_H */
