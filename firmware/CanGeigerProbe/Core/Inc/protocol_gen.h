/* AUTO-GENEROWANE — NIE EDYTOWAC RECZNIE
 * 
 * Zrodla: CanBusCommon/protocol/common.yaml oraz registry/
 *         protocol/device.yaml
 * Regeneracja: python common/protocol/generate.py protocol/device.yaml
 * 
 * Urzadzenie      : CanGeigerProbe (DEVICE_TYPE 0x0004)
 * Profil wspolny  : 2
 * Rewizja         : 2.1
 * Skrot bazy      : 46c5b193bd6273b0
 */
#ifndef PROTOCOL_GEN_H
#define PROTOCOL_GEN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MPC_PROFILE_VERSION   2u
#define MPC_PROTO_VERSION     1u
#define MPC_STANDARD_REVISION "2.1"
#define MPC_COMMON_BASE_HASH  "46c5b193bd6273b0"
#define MPC_DEVICE_TYPE       0x0004u
#define MPC_DEVICE_NAME       "CanGeigerProbe"
#define MPC_FW_VERSION_MAJOR  0u
#define MPC_FW_VERSION_MINOR  1u
#define MPC_HW_VERSION_MAJOR  1u
#define MPC_HW_VERSION_MINOR  0u
#define MPC_BUILD_REVISION    1u
#define MPC_CAPABILITIES      0x000437FFu

#define MPC_CAN_BITRATE       500000u
#define MPC_CAN_ID_BITS       29u
#define MPC_CAN_MAX_DLC       8u

#define MPC_NODE_BROADCAST    0xFFu
#define MPC_NODE_UNCONFIGURED 0x00u
#define MPC_NODE_HOST         0xFEu
#define MPC_NODE_DEFAULT      0x30u
#define MPC_NODE_MIN          0x30u
#define MPC_NODE_MAX          0x3Fu

/* ---- Uklad 29-bitowego identyfikatora ---- */
#define MPC_FUNC_SHIFT 24u
#define MPC_FUNC_MASK  0x1Fu
#define MPC_NODE_SHIFT 16u
#define MPC_NODE_MASK  0xFFu
#define MPC_OBJ_SHIFT 8u
#define MPC_OBJ_MASK  0xFFu
#define MPC_ARG_SHIFT 0u
#define MPC_ARG_MASK  0xFFu
#define MPC_ID_PACK(func, node, obj, arg) ( \
  (((uint32_t)((func) & MPC_FUNC_MASK)) << MPC_FUNC_SHIFT) | \
  (((uint32_t)((node) & MPC_NODE_MASK)) << MPC_NODE_SHIFT) | \
  (((uint32_t)((obj) & MPC_OBJ_MASK)) << MPC_OBJ_SHIFT) | \
  (((uint32_t)((arg) & MPC_ARG_MASK)) << MPC_ARG_SHIFT) )
#define MPC_ID_FUNC(id) (((uint32_t)(id) >> MPC_FUNC_SHIFT) & MPC_FUNC_MASK)
#define MPC_ID_NODE(id) (((uint32_t)(id) >> MPC_NODE_SHIFT) & MPC_NODE_MASK)
#define MPC_ID_OBJ(id) (((uint32_t)(id) >> MPC_OBJ_SHIFT) & MPC_OBJ_MASK)
#define MPC_ID_ARG(id) (((uint32_t)(id) >> MPC_ARG_SHIFT) & MPC_ARG_MASK)

/* Klasy komunikatow — mniejsza wartosc = wyzszy priorytet na magistrali */
typedef enum {
  MPC_FUNC_EVENT = 0x01,
  MPC_FUNC_REQUEST = 0x08,
  MPC_FUNC_RESPONSE = 0x09,
  MPC_FUNC_STREAM = 0x0A,
  MPC_FUNC_TELEMETRY = 0x10,
} mpc_func_t;

/* Kanaly telemetrii (pole OBJ przy FUNC=TELEMETRY) — id z globalnego rejestru wielkosci */
typedef enum {
  MPC_CH_CPM = 0x41,
  MPC_CH_DOSE_RATE = 0x40,
  MPC_CH_DOSE_ACC = 0x42,
  MPC_CH_TUBE_HV = 0x43,
  MPC_CH_SUPPLY_VOLTAGE = 0x12,
  MPC_CH_SUPPLY_CURRENT = 0x13,
  MPC_CH_MCU_TEMP = 0x0A,
  MPC_CH_VDDA = 0x0B,
} mpc_channel_t;

/* Czujniki — id z globalnego rejestru typow czujnikow */
typedef enum {
  MPC_SENSOR_GM_TUBE = 0x30,
  MPC_SENSOR_HV_MONITOR = 0x31,
  MPC_SENSOR_SUPPLY_MON = 0x11,
  MPC_SENSOR_MCU = 0x0C,
} mpc_sensor_t;

/* Wielkosci fizyczne — id z globalnego rejestru */
typedef enum {
  MPC_QTY_MCU_TEMP = 0x0A,
  MPC_QTY_VDDA = 0x0B,
  MPC_QTY_VBAT = 0x0C,
  MPC_QTY_SUPPLY_VOLTAGE = 0x12,
  MPC_QTY_SUPPLY_CURRENT = 0x13,
  MPC_QTY_DOSE_RATE = 0x40,
  MPC_QTY_CPM = 0x41,
  MPC_QTY_DOSE_ACC = 0x42,
  MPC_QTY_TUBE_HV = 0x43,
} mpc_quantity_t;

/* Opcode'y REQUEST (pole OBJ) */
typedef enum {
  MPC_REQ_PING = 0x00,
  MPC_REQ_GET_INFO = 0x01,
  MPC_REQ_READ_AVG = 0x02,
  MPC_REQ_READ_SENSOR = 0x03,
  MPC_REQ_READ_PARAM = 0x04,
  MPC_REQ_WRITE_PARAM = 0x05,
  MPC_REQ_LIST_SENSORS = 0x06,
  MPC_REQ_SAVE_CONFIG = 0x07,
  MPC_REQ_LOAD_DEFAULTS = 0x08,
  MPC_REQ_GET_TIME = 0x09,
  MPC_REQ_SET_TIME = 0x0A,
  MPC_REQ_GET_STATUS = 0x0B,
  MPC_REQ_RESET = 0x0C,
  MPC_REQ_ENTER_BOOTLOADER = 0x0D,
  MPC_REQ_GET_CAPABILITIES = 0x12,
  MPC_REQ_LIST_PARAMS = 0x13,
  MPC_REQ_LIST_CHANNELS = 0x14,
  MPC_REQ_SET_TIME_SYNC = 0x15,
  MPC_REQ_GET_COUNTERS = 0x16,
  MPC_REQ_GET_STATUS_EXT = 0x20,
  MPC_REQ_SELF_TEST = 0x21,
  MPC_REQ_CLEAR_COUNTERS = 0x22,
  MPC_REQ_RESET_DOSE = 0x80,
  MPC_REQ_READ_RAW_COUNTS = 0x81,
  MPC_REQ_GET_HV_STATUS = 0x82,
} mpc_reqop_t;

/* Kody statusu (byte0 ramki RESPONSE) */
typedef enum {
  MPC_OK = 0x00,
  MPC_ERR_UNKNOWN_CMD = 0x01,
  MPC_ERR_BAD_PARAM = 0x02,
  MPC_ERR_OUT_OF_RANGE = 0x03,
  MPC_ERR_SENSOR_ABSENT = 0x04,
  MPC_ERR_SENSOR_FAULT = 0x05,
  MPC_ERR_BUSY = 0x06,
  MPC_ERR_READONLY = 0x07,
  MPC_ERR_NOT_READY = 0x08,
  MPC_ERR_CRC = 0x09,
} mpc_status_t;

/* Zrodla zdarzen (pole OBJ przy FUNC=EVENT); pozostale = id czujnika */
typedef enum {
  MPC_EVTSRC_SYSTEM = 0x00,
} mpc_evtsrc_t;

/* Kody zdarzen (pole ARG przy FUNC=EVENT) */
typedef enum {
  MPC_EVT_BOOT = 0x01,
  MPC_EVT_SENSOR_ALERT = 0x03,
  MPC_EVT_SENSOR_FAULT = 0x04,
  MPC_EVT_SENSOR_BACK = 0x05,
  MPC_EVT_BUS_RECOVERED = 0x06,
  MPC_EVT_HEARTBEAT = 0x07,
  MPC_EVT_SENSOR_IRQ = 0x08,
  MPC_EVT_CRITICAL_ALARM = 0x09,
  MPC_EVT_SENSOR_RANGE = 0x0C,
  MPC_EVT_ADDR_CONFLICT = 0x0D,
  MPC_EVT_CONFIG_CHANGED = 0x0E,
  MPC_EVT_TIME_SYNC = 0x20,
  MPC_EVT_TIME_FOLLOW_UP = 0x21,
  MPC_EVT_DOSE_ALARM = 0x80,
  MPC_EVT_HV_FAULT = 0x81,
} mpc_event_t;

/* Status odczytu wzgledem limitow (LPL/UPL, LTL/UTL) */
typedef enum {
  MPC_RSTAT_OK = 0x00,
  MPC_RSTAT_WARN_LOW = 0x01,
  MPC_RSTAT_WARN_HIGH = 0x02,
  MPC_RSTAT_ERR_LOW = 0x03,
  MPC_RSTAT_ERR_HIGH = 0x04,
} mpc_reading_status_t;

/* Status czujnika w odpowiedzi LIST_SENSORS */
typedef enum {
  MPC_SSTAT_PRESENT = 0x00,
  MPC_SSTAT_ABSENT = 0x01,
  MPC_SSTAT_FAULT = 0x02,
  MPC_SSTAT_DISABLED = 0x03,
} mpc_sensor_status_t;

/* ---- Bajty zabezpieczajace komend destrukcyjnych ---- */
#define MPC_GUARD_LOAD_DEFAULTS 0x5Au
#define MPC_GUARD_RESET 0xA5u
#define MPC_GUARD_ENTER_BOOTLOADER 0xB0u
#define MPC_GUARD_RESET_DOSE 0xD0u

/* ---- Uklad pola danych ramki TELEMETRY ---- */
#define MPC_TLM_OFF_SEQ 0u
#define MPC_TLM_OFF_FLAGS 1u
#define MPC_TLM_OFF_VALUE 2u
#define MPC_TLM_OFF_QUAL 6u
#define MPC_TLM_FLAG_VALID (1u << 0u)
#define MPC_TLM_FLAG_CLAMPED (1u << 5u)
#define MPC_TLM_FLAG_RANGE_WARN (1u << 6u)
#define MPC_TLM_FLAG_RANGE_ERR_DROPPED (1u << 7u)
#define MPC_TLM_SRC_SHIFT 1u
#define MPC_TLM_SRC_MASK  0x0Fu

/* ---- Naglowek segmentu FUNC=STREAM (byte0) ---- */
#define MPC_STREAM_LAST  (1u << 7u)
#define MPC_STREAM_IMASK 0x7Fu

/* ---- Status podsystemow (odpowiedz GET_STATUS) ---- */
#define MPC_STATUS_LEN 7u
#define MPC_STATUS_OFF_SYS 0u
#define MPC_STATUS_OFF_ERR 1u
#define MPC_STATUS_OFF_PRESENT 2u
#define MPC_STATUS_OFF_FAULT 4u
#define MPC_STATUS_OFF_NHEALTHY 6u

#define MPC_STATUS_SYS_LSE_ACTIVE (1u << 0u)  /* Zegar LSE + autotrim MSI aktywny */
#define MPC_STATUS_SYS_RTC_OK (1u << 1u)  /* RTC obecny (oscylator wystartowal) */
#define MPC_STATUS_SYS_CAN_OK (1u << 2u)  /* Sterownik CAN zainicjowany i pracuje */
#define MPC_STATUS_SYS_I2C_OK (1u << 3u)  /* Magistrala I2C zainicjowana i pracuje */
#define MPC_STATUS_SYS_ADC_OK (1u << 4u)  /* Wewnetrzny ADC sprawny */
#define MPC_STATUS_SYS_FPU_OK (1u << 5u)  /* FPU wlaczone (CP10/CP11 w CPACR) */
#define MPC_STATUS_SYS_CONFIG_OK (1u << 6u)  /* Konfiguracja zaladowana i wazna */
#define MPC_STATUS_SYS_SENSORS_OK (1u << 7u)  /* Przynajmniej jedno zrodlo dostarcza dane */
#define MPC_STATUS_ERR_CRITICAL (1u << 0u)  /* Blad krytyczny aktywny (wyzwala EVENT CRITICAL_ALARM) */
#define MPC_STATUS_ERR_SENSOR_FAULT (1u << 1u)  /* Co najmniej jeden czujnik w awarii */
#define MPC_STATUS_ERR_NO_SENSORS (1u << 2u)  /* Zadne zrodlo nie dostarcza danych */
#define MPC_STATUS_ERR_WDT_RESET (1u << 3u)  /* Ostatni reset od watchdoga (IWDG/WWDG) */
#define MPC_STATUS_ERR_NVM_ERROR (1u << 4u)  /* Blad zapisu/odczytu pamieci nieulotnej */
#define MPC_STATUS_ERR_CAN_BUSOFF (1u << 5u)  /* Wystapil i zostal odzyskany bus-off CAN */
#define MPC_STATUS_ERR_I2C_RECOVERED (1u << 6u)  /* Wykonano odzysk zawieszonej magistrali I2C */
#define MPC_STATUS_ERR_VBAT_LOW (1u << 7u)  /* Napiecie VBAT ponizej progu */

/* ---- Status rozszerzony (odpowiedz GET_STATUS_EXT) ---- */
#define MPC_STATUS_EXT_LEN 13u
#define MPC_STATUS_EXT_LEN_BASE 10u  /* bez pol od rewizji 2.1 */
#define MPC_STATUS_EXT_OFF_SYS2 0u
#define MPC_STATUS_EXT_OFF_ERR2 1u
#define MPC_STATUS_EXT_OFF_UPTIME_S 2u
#define MPC_STATUS_EXT_OFF_BOOT_COUNT 6u
#define MPC_STATUS_EXT_OFF_LAST_FAULT 8u
#define MPC_STATUS_EXT_OFF_POST_MASK 9u
#define MPC_STATUS_EXT_OFF_TIME_SOURCE 10u
#define MPC_STATUS_EXT_OFF_TIME_AGE_S 11u

#define MPC_STATUS_ERR2_ADDR_CONFLICT (1u << 0u)  /* Wykryto inny wezel o tym samym adresie - telemetria wstrzymana */
#define MPC_STATUS_ERR2_CAL_LOCKED (1u << 1u)  /* Parametry kalibracyjne zablokowane (CAL_LOCK=1) */
#define MPC_STATUS_ERR2_TIME_UNSYNCED (1u << 2u)  /* RTC nie byl synchronizowany od startu */
#define MPC_STATUS_ERR2_RESET_LOOP (1u << 3u)  /* Wykryto petle resetow */

/* ---- Liczniki diagnostyczne (odpowiedz GET_COUNTERS) ---- */
#define MPC_COUNTERS_LEN 16u
#define MPC_COUNTERS_OFF_RX_FRAMES 0u
#define MPC_COUNTERS_OFF_TX_FRAMES 4u
#define MPC_COUNTERS_OFF_TX_DROPPED 8u
#define MPC_COUNTERS_OFF_RX_DROPPED 10u
#define MPC_COUNTERS_OFF_BUSOFF 12u
#define MPC_COUNTERS_OFF_I2C_ERR 14u

/* ---- Ramka identyfikacyjna (odpowiedz GET_INFO) ---- */
#define MPC_INFO_LEN 28u
#define MPC_INFO_OFF_PROTO_VERSION 0u
#define MPC_INFO_OFF_FW_MAJOR 1u
#define MPC_INFO_OFF_FW_MINOR 2u
#define MPC_INFO_OFF_NODE 3u
#define MPC_INFO_OFF_HW_MAJOR 4u
#define MPC_INFO_OFF_HW_MINOR 5u
#define MPC_INFO_OFF_BUILD_REVISION 6u
#define MPC_INFO_OFF_BUILD_FLAGS 8u
#define MPC_INFO_OFF_UID 9u
#define MPC_INFO_OFF_DEVICE_TYPE 21u
#define MPC_INFO_OFF_PROFILE_VERSION 23u
#define MPC_INFO_OFF_CAPABILITIES 24u

#define MPC_INFO_FLAG_DEBUG (1u << 0u)  /* Build Debug - NIE uzywac w produkcji */

/* ---- Uklady ladunkow zdarzen uniwersalnych ---- */
#define MPC_EVT_BOOT_LEN 6u
#define MPC_EVT_BOOT_OFF_RESET_CAUSE 0u
#define MPC_EVT_BOOT_OFF_PROTO_VERSION 1u
#define MPC_EVT_BOOT_OFF_LAST_FAULT 2u
#define MPC_EVT_BOOT_OFF_FAULT_COUNT 3u
#define MPC_EVT_BOOT_OFF_BOOT_COUNT 4u
#define MPC_EVT_BOOT_OFF_POST_MASK 5u

#define MPC_EVT_BOOT_RESET_CAUSE_LPWR (1u << 0u)
#define MPC_EVT_BOOT_RESET_CAUSE_WWDG (1u << 1u)
#define MPC_EVT_BOOT_RESET_CAUSE_IWDG (1u << 2u)
#define MPC_EVT_BOOT_RESET_CAUSE_SOFT (1u << 3u)  /* Reset zamierzony, np. po wyjsciu z bootloadera */
#define MPC_EVT_BOOT_RESET_CAUSE_BOR_POR (1u << 4u)
#define MPC_EVT_BOOT_RESET_CAUSE_PIN (1u << 5u)  /* Wejscie NRST */
#define MPC_EVT_BOOT_RESET_CAUSE_OBL (1u << 6u)
#define MPC_EVT_BOOT_POST_MASK_RAM (1u << 0u)  /* Test marszowy RAM (SRAM2 w calosci + bufor w SRAM1) */
#define MPC_EVT_BOOT_POST_MASK_FLASH (1u << 1u)  /* Suma CRC obszaru programu */
#define MPC_EVT_BOOT_POST_MASK_CPU (1u << 2u)  /* Sciezka ALU i rejestry */
#define MPC_EVT_BOOT_POST_MASK_CLOCK (1u << 3u)  /* Taktowanie rdzenia poza oczekiwanym zakresem */
#define MPC_EVT_HEARTBEAT_LEN 7u
#define MPC_EVT_HEARTBEAT_OFF_UPTIME_S 0u
#define MPC_EVT_HEARTBEAT_OFF_BUSOFF 2u
#define MPC_EVT_HEARTBEAT_OFF_I2C_ERR 3u
#define MPC_EVT_HEARTBEAT_OFF_CAN_DROP 4u
#define MPC_EVT_HEARTBEAT_OFF_SENSORS 5u
#define MPC_EVT_HEARTBEAT_OFF_FLAGS 6u

#define MPC_EVT_HEARTBEAT_FLAGS_LSE (1u << 0u)
#define MPC_EVT_HEARTBEAT_FLAGS_VDD_LOW (1u << 1u)
#define MPC_EVT_HEARTBEAT_FLAGS_STACK_LOW (1u << 2u)
#define MPC_EVT_HEARTBEAT_FLAGS_RESET_LOOP (1u << 3u)
#define MPC_EVT_HEARTBEAT_FLAGS_SELFTEST (1u << 4u)
#define MPC_EVT_HEARTBEAT_FLAGS_TIME_SYNCED (1u << 5u)  /* Zegar zsynchronizowany: od ostatniej przyjetej synchronizacji uplynelo mniej niz time_sync.valid_s. Wezly sprzed rewizji 2.1 nadaja tu 0 - zgodnie z prawda. */
#define MPC_EVT_SENSOR_IRQ_LEN 1u
#define MPC_EVT_SENSOR_IRQ_OFF_LINES 0u

#define MPC_EVT_SENSOR_IRQ_LINES_GM_PULSE (1u << 0u)  /* Wyjscie komparatora impulsow tuby (COMP1). Impulsy zliczane sa sprzetowo (LPTIM1) i nie sa zglaszane zdarzeniem SENSOR_IRQ; bit opisuje linie na wypadek zgloszenia surowego przerwania. */
#define MPC_EVT_CRITICAL_ALARM_LEN 2u
#define MPC_EVT_CRITICAL_ALARM_OFF_SYS 0u
#define MPC_EVT_CRITICAL_ALARM_OFF_ERR 1u

#define MPC_EVT_SENSOR_RANGE_LEN 6u
#define MPC_EVT_SENSOR_RANGE_OFF_QTY 0u
#define MPC_EVT_SENSOR_RANGE_OFF_READING_STATUS 1u
#define MPC_EVT_SENSOR_RANGE_OFF_VALUE 2u

#define MPC_EVT_ADDR_CONFLICT_LEN 1u
#define MPC_EVT_ADDR_CONFLICT_OFF_NODE 0u

#define MPC_EVT_TIME_SYNC_LEN 2u
#define MPC_EVT_TIME_SYNC_OFF_SEQ 0u
#define MPC_EVT_TIME_SYNC_OFF_QUALITY 1u

#define MPC_EVT_TIME_SYNC_QUALITY_FIX (1u << 0u)
#define MPC_EVT_TIME_SYNC_QUALITY_PPS (1u << 1u)
#define MPC_EVT_TIME_SYNC_QUALITY_HOLDOVER (1u << 2u)
#define MPC_EVT_TIME_FOLLOW_UP_LEN 8u
#define MPC_EVT_TIME_FOLLOW_UP_OFF_SEQ 0u
#define MPC_EVT_TIME_FOLLOW_UP_OFF_UNIX 1u
#define MPC_EVT_TIME_FOLLOW_UP_OFF_USEC 5u

#define MPC_EVT_CONFIG_CHANGED_LEN 2u
#define MPC_EVT_CONFIG_CHANGED_OFF_PARAM_ID 0u
#define MPC_EVT_CONFIG_CHANGED_OFF_SOURCE 1u

#define MPC_EVT_CONFIG_CHANGED_SOURCE_CAN 0u
#define MPC_EVT_CONFIG_CHANGED_SOURCE_LOCAL 1u  /* Menu lokalne wezla */
#define MPC_EVT_CONFIG_CHANGED_SOURCE_AUTO 2u  /* Automatyka wezla */
/* Zdarzenia bez ladunku: SENSOR_ALERT, SENSOR_FAULT, SENSOR_BACK, BUS_RECOVERED */

/* ---- Uklady ladunkow zdarzen wlasnych urzadzenia ---- */
#define MPC_EVT_DOSE_ALARM_LEN 6u
#define MPC_EVT_DOSE_ALARM_OFF_LEVEL 0u
#define MPC_EVT_DOSE_ALARM_OFF_STATE 1u
#define MPC_EVT_DOSE_ALARM_OFF_VALUE 2u

#define MPC_EVT_DOSE_ALARM_LEVEL_RATE_WARN 1u  /* ALARM_RATE_WARN */
#define MPC_EVT_DOSE_ALARM_LEVEL_RATE_HIGH 2u  /* ALARM_RATE_HIGH */
#define MPC_EVT_DOSE_ALARM_LEVEL_DOSE 3u  /* ALARM_DOSE */
#define MPC_EVT_DOSE_ALARM_STATE_CLEARED 0u  /* Powrot ponizej progu (z histereza) */
#define MPC_EVT_DOSE_ALARM_STATE_EXCEEDED 1u  /* Przekroczenie progu */
#define MPC_EVT_HV_FAULT_LEN 3u
#define MPC_EVT_HV_FAULT_OFF_CAUSE 0u
#define MPC_EVT_HV_FAULT_OFF_HV_MEAS 1u

#define MPC_EVT_HV_FAULT_CAUSE_OVERVOLTAGE 1u  /* HV powyzej HV_MAX albo limitu wkompilowanego */
#define MPC_EVT_HV_FAULT_CAUSE_UNDERVOLTAGE 2u  /* HV ponizej HV_MIN po zakonczeniu lagodnego startu */
#define MPC_EVT_HV_FAULT_CAUSE_NO_RISE 3u  /* Seria impulsow bez oczekiwanego przyrostu HV (przerwany dzielnik, zwarcie) */
#define MPC_EVT_HV_FAULT_CAUSE_PULSE_BUDGET 4u  /* Przekroczony budzet impulsow na sekunde */
/* ---- Odpowiedz jednoramkowa: ladunek po bajcie statusu ---- */
#define MPC_RESPONSE_MAX_DATA 7u

/* ---- Odpowiedz READ_AVG ---- */
#define MPC_READ_AVG_LEN 5u
#define MPC_READ_AVG_OFF_FLAGS 0u
#define MPC_READ_AVG_OFF_VALUE 1u

/* ---- Zegar: GET_TIME / SET_TIME (pola BINARNE, nie BCD) ---- */
#define MPC_TIME_LEN 7u
#define MPC_TIME_OFF_SEC 0u
#define MPC_TIME_OFF_MIN 1u
#define MPC_TIME_OFF_HOUR 2u
#define MPC_TIME_OFF_DAY 3u
#define MPC_TIME_OFF_MONTH 4u
#define MPC_TIME_OFF_YEAR 5u
#define MPC_TIME_OFF_WDAY 6u

/* ---- Wspolna os czasu: ladunek SET_TIME_SYNC ---- */
#define MPC_TIME_SYNC_LEN 7u
#define MPC_TIME_SYNC_LEN_BASE 6u  /* bez pol od rewizji 2.1 */
#define MPC_TIME_SYNC_OFF_UNIX 0u
#define MPC_TIME_SYNC_OFF_MS 4u
#define MPC_TIME_SYNC_OFF_QUALITY 6u

#define MPC_TIME_SYNC_QUALITY_FIX (1u << 0u)  /* Zrodlo ma wazne rozwiazanie czasu (np. fix GNSS) */
#define MPC_TIME_SYNC_QUALITY_PPS (1u << 1u)  /* Czas zdyscyplinowany impulsem PPS */
#define MPC_TIME_SYNC_QUALITY_HOLDOVER (1u << 2u)  /* Zrodlo utracilo wzorzec i podtrzymuje czas zegarem wlasnym */

/* ARG ramki SET_TIME_SYNC = adres zrodla; wartosci szczegolne */
#define MPC_TIME_SOURCE_UNSPECIFIED 0x00u  /* Nadawca sprzed rewizji 2.1 (ARG zawsze 0) */
#define MPC_TIME_SOURCE_HOST 0xFEu  /* Stanowisko hosta (jak nodes.host) */
#define MPC_TIME_SOURCE_NONE 0xFFu  /* W statusie: nie bylo synchronizacji od startu */
#define MPC_TIME_SYNC_LEGACY_MIN_LEN 4u
#define MPC_TIME_SYNC_SILENCE_PERIODS 2u
#define MPC_TIME_SYNC_VALID_S 3600u
#define MPC_TIME_SYNC_FOLLOW_UP_WINDOW_MS 100u

/* ---- Rekordy transferow segmentowanych ---- */
#define MPC_STREAM_MAX_PAYLOAD 7u
#define MPC_REC_SENSOR_ENTRY_LEN 2u
#define MPC_REC_SENSOR_ENTRY_OFF_SENSOR 0u
#define MPC_REC_SENSOR_ENTRY_OFF_STATUS 1u

/* Segment miesci 3 rekordow = 6 B; konczenie segmentu
 * na granicy rekordu jest zalecane, patrz common.yaml/stream_header. */
#define MPC_REC_SENSOR_ENTRY_PER_SEG 3u

#define MPC_REC_CHANNEL_ENTRY_LEN 2u
#define MPC_REC_CHANNEL_ENTRY_OFF_QUANTITY 0u
#define MPC_REC_CHANNEL_ENTRY_OFF_FLAGS 1u

/* Segment miesci 3 rekordow = 6 B; konczenie segmentu
 * na granicy rekordu jest zalecane, patrz common.yaml/stream_header. */
#define MPC_REC_CHANNEL_ENTRY_PER_SEG 3u

#define MPC_REC_PARAM_ENTRY_LEN 3u
#define MPC_REC_PARAM_ENTRY_OFF_ID 0u
#define MPC_REC_PARAM_ENTRY_OFF_TYPE 1u
#define MPC_REC_PARAM_ENTRY_OFF_FLAGS 2u

/* Segment miesci 2 rekordow = 6 B; konczenie segmentu
 * na granicy rekordu jest zalecane, patrz common.yaml/stream_header. */
#define MPC_REC_PARAM_ENTRY_PER_SEG 2u

#define MPC_REC_SENSOR_READING_LEN 6u
#define MPC_REC_SENSOR_READING_OFF_QTY 0u
#define MPC_REC_SENSOR_READING_OFF_VALUE 1u
#define MPC_REC_SENSOR_READING_OFF_READING_STATUS 5u

/* Segment miesci 1 rekordow = 6 B; konczenie segmentu
 * na granicy rekordu jest zalecane, patrz common.yaml/stream_header. */
#define MPC_REC_SENSOR_READING_PER_SEG 1u

/* ---- Kodowanie bajtu typu parametru (pole TYPE w param_entry) ---- */
/* Kolejnosc ZAMROZONA — jedzie na magistrali. */

/* ---- Bity CAPABILITIES ---- */
#define MPC_CAP_TELEMETRY (1u << 0u)  /* Nadaje cykliczna telemetrie */
#define MPC_CAP_SENSORS (1u << 1u)  /* READ_SENSOR / LIST_SENSORS maja sens */
#define MPC_CAP_PARAMS_NV (1u << 2u)  /* SAVE_CONFIG / LOAD_DEFAULTS */
#define MPC_CAP_RTC (1u << 3u)  /* GET_TIME / SET_TIME */
#define MPC_CAP_BOOTLOADER (1u << 4u)  /* ENTER_BOOTLOADER */
#define MPC_CAP_TIME_SYNC (1u << 5u)  /* Przyjmuje SET_TIME_SYNC */
#define MPC_CAP_LIST_PARAMS (1u << 6u)
#define MPC_CAP_LIST_CHANNELS (1u << 7u)
#define MPC_CAP_COUNTERS (1u << 8u)  /* GET_COUNTERS / CLEAR_COUNTERS */
#define MPC_CAP_SELF_TEST (1u << 9u)
#define MPC_CAP_CAL_LOCK (1u << 10u)  /* Blokada zapisu parametrow kalibracyjnych */
#define MPC_CAP_RANGE_LIMITS (1u << 11u)  /* LPL/UPL/LTL/UTL + EVENT SENSOR_RANGE */
#define MPC_CAP_ADDR_CONFLICT_DETECT (1u << 12u)
#define MPC_CAP_STATUS_EXT (1u << 13u)  /* GET_STATUS_EXT */
#define MPC_CAP_SENSOR_FUSION (1u << 14u)  /* Wazona fuzja wielu czujnikow na kanal */
#define MPC_CAP_OUTPUTS (1u << 15u)  /* Sterowalne wyjscia */
#define MPC_CAP_DISPLAY (1u << 16u)  /* Lokalny wyswietlacz/menu */
#define MPC_CAP_GNSS (1u << 17u)
#define MPC_CAP_RADIOMETRY (1u << 18u)
#define MPC_CAP_LIGHTNING (1u << 19u)
#define MPC_CAP_TIME_SOURCE (1u << 20u)  /* Wzorzec czasu: nadaje SET_TIME_SYNC (ARG = wlasny adres), parametry TIME_SYNC_PERIOD i TIME_HOLDOVER */
#define MPC_CAP_TIME_SYNC_PRECISE (1u << 21u)  /* Przyjmuje synchronizacje precyzyjna TIME_SYNC / TIME_FOLLOW_UP */

/* ---- Skale (wartosc_fizyczna = raw * skala) ---- */
#define MPC_SCALE_MCU_TEMP 0.01f
#define MPC_SCALE_VDDA 0.001f
#define MPC_SCALE_VBAT 0.001f
#define MPC_SCALE_SUPPLY_VOLTAGE 0.001f
#define MPC_SCALE_SUPPLY_CURRENT 1e-06f
#define MPC_SCALE_DOSE_RATE 0.01f
#define MPC_SCALE_CPM 1.0f
#define MPC_SCALE_DOSE_ACC 0.01f
#define MPC_SCALE_TUBE_HV 0.1f

/* ---- Identyfikatory parametrow ---- */
typedef enum {
  MPC_PARAM_NODE_ID = 0x01,
  MPC_PARAM_TELEMETRY_PERIOD = 0x02,
  MPC_PARAM_MEASURE_PERIOD = 0x03,
  MPC_PARAM_CAN_TERMINATION = 0x04,
  MPC_PARAM_LOW_POWER = 0x07,
  MPC_PARAM_CAL_LOCK = 0x09,
  MPC_PARAM_HEARTBEAT_PERIOD = 0x0A,
  MPC_PARAM_TUBE_TYPE = 0x10,
  MPC_PARAM_HV_ENABLE = 0x11,
  MPC_PARAM_HV_SETPOINT = 0x12,
  MPC_PARAM_HV_MIN = 0x13,
  MPC_PARAM_HV_MAX = 0x14,
  MPC_PARAM_COMP_THRESHOLD = 0x18,
  MPC_PARAM_AVG_MODE = 0x1C,
  MPC_PARAM_AVG_WINDOW_S = 0x1D,
  MPC_PARAM_ALARM_RATE_WARN = 0x20,
  MPC_PARAM_ALARM_RATE_HIGH = 0x21,
  MPC_PARAM_ALARM_DOSE = 0x22,
  MPC_PARAM_ALARM_HYST = 0x23,
  MPC_PARAM_CAL_CPM_TO_USVH = 0x30,
  MPC_PARAM_CAL_DEAD_TIME_US = 0x31,
  MPC_PARAM_CAL_HV_C0 = 0x32,
  MPC_PARAM_CAL_HV_C1 = 0x33,
  MPC_PARAM_CAL_VIN_C0 = 0x34,
  MPC_PARAM_CAL_VIN_C1 = 0x35,
  MPC_PARAM_CAL_ISENS_C0 = 0x36,
  MPC_PARAM_CAL_ISENS_C1 = 0x37,
} mpc_param_id_t;

typedef enum {
  MPC_PTYPE_U8 = 0,
  MPC_PTYPE_U16 = 1,
  MPC_PTYPE_U32 = 2,
  MPC_PTYPE_I16 = 3,
  MPC_PTYPE_I32 = 4,
  MPC_PTYPE_F32 = 5,
} mpc_ptype_t;

typedef enum { MPC_ACC_RO = 0, MPC_ACC_RW = 1 } mpc_access_t;
typedef enum { MPC_STORE_RAM = 0, MPC_STORE_NV = 1 } mpc_storage_t;

/* Flagi parametru. CALIBRATION => zapis blokowany, dopoki CAL_LOCK = 1. */
#define MPC_PFLAG_CALIBRATION (1u << 0u)
#define MPC_PFLAG_MANDATORY   (1u << 1u)

typedef struct {
  uint8_t id;
  uint8_t type;     /* mpc_ptype_t   */
  uint8_t access;   /* mpc_access_t  */
  uint8_t storage;  /* mpc_storage_t */
  uint8_t flags;    /* MPC_PFLAG_*   */
  int32_t min;      /* f32: wzorzec bitowy */
  int32_t max;
  int32_t def;
} mpc_param_desc_t;

#define MPC_PARAM_COUNT 27u
extern const mpc_param_desc_t mpc_param_table[MPC_PARAM_COUNT];

#define MPC_SENSOR_COUNT 4u
extern const uint8_t mpc_sensor_ids[MPC_SENSOR_COUNT];

#define MPC_CHANNEL_COUNT 8u
extern const uint8_t mpc_channel_ids[MPC_CHANNEL_COUNT];

#ifdef __cplusplus
}
#endif
#endif /* PROTOCOL_GEN_H */
