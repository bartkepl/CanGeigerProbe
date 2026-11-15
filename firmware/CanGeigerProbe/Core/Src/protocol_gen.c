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
#include "protocol_gen.h"

const mpc_param_desc_t mpc_param_table[MPC_PARAM_COUNT] = {
  { MPC_PARAM_NODE_ID, MPC_PTYPE_U8, MPC_ACC_RW, MPC_STORE_NV, MPC_PFLAG_MANDATORY, 48, 63, 48 },  /* Adres wezla na magistrali. Zakres przydzielony typowi urzadzenia - patrz registry/node_addresses.yaml. Zmiana dziala od razu. */
  { MPC_PARAM_TELEMETRY_PERIOD, MPC_PTYPE_U16, MPC_ACC_RW, MPC_STORE_NV, MPC_PFLAG_MANDATORY, 100, 60000, 2000 },  /* Okres cyklicznego rozglaszania telemetrii. */
  { MPC_PARAM_MEASURE_PERIOD, MPC_PTYPE_U16, MPC_ACC_RW, MPC_STORE_NV, 0, 100, 60000, 1000 },  /* Okres pelnego cyklu pomiarowego. Dolna granica moze byc podniesiona przez device.yaml, gdy peryferia tego wymagaja. */
  { MPC_PARAM_CAN_TERMINATION, MPC_PTYPE_U8, MPC_ACC_RW, MPC_STORE_NV, 0, 0, 1, 1 },  /* Rezystor terminujacy 120 Ohm na plytce. 1 = wlaczony (wezel na koncu magistrali). Ustaw 0 dla wezla w srodku. */
  { MPC_PARAM_LOW_POWER, MPC_PTYPE_U8, MPC_ACC_RW, MPC_STORE_NV, 0, 0, 1, 1 },  /* Usypianie rdzenia (WFI) miedzy zdarzeniami. Nie wplywa na timing. */
  { MPC_PARAM_CAL_LOCK, MPC_PTYPE_U8, MPC_ACC_RW, MPC_STORE_NV, 0, 0, 1, 1 },  /* Blokada zapisu parametrow kalibracyjnych. 1 = zablokowane (WRITE_PARAM na parametr oznaczony 'calibration' zwraca ERR_READONLY). Domyslnie WLACZONA - metrologia wezla nie moze byc zmieniona jedna przypadkowa ramka. Zdjecie blokady jest swiadoma, osobna czynnoscia operatora. */
  { MPC_PARAM_HEARTBEAT_PERIOD, MPC_PTYPE_U16, MPC_ACC_RW, MPC_STORE_NV, 0, 1000, 60000, 10000 },  /* Okres zdarzenia HEARTBEAT. Host uznaje wezel za utracony po braku HEARTBEAT przez 3 okresy. */
  { MPC_PARAM_TUBE_TYPE, MPC_PTYPE_U8, MPC_ACC_RW, MPC_STORE_NV, 0, 0, 1, 0 },  /* Typ tuby: 0 = STS-5 (R12 4.7 MOhm), 1 = DOI-80 (R12 5.1 MOhm). Wybiera domyslne wartosci HV i czasu martwego przy LOAD_DEFAULTS. */
  { MPC_PARAM_HV_ENABLE, MPC_PTYPE_U8, MPC_ACC_RW, MPC_STORE_NV, 0, 0, 1, 0 },  /* Wlaczenie generatora HV. Domyslnie wylaczony; po HV_FAULT wymaga ponownego zapisu 1. */
  { MPC_PARAM_HV_SETPOINT, MPC_PTYPE_U16, MPC_ACC_RW, MPC_STORE_NV, 0, 300, 500, 400 },  /* Nastawa napiecia anodowego. */
  { MPC_PARAM_HV_MIN, MPC_PTYPE_U16, MPC_ACC_RW, MPC_STORE_NV, 0, 250, 500, 360 },  /* Dolna granica pracy; ponizej po lagodnym starcie — HV_FAULT UNDERVOLTAGE. */
  { MPC_PARAM_HV_MAX, MPC_PTYPE_U16, MPC_ACC_RW, MPC_STORE_NV, 0, 300, 550, 450 },  /* Gorna granica pracy; powyzej — HV_FAULT OVERVOLTAGE. Niezaleznie obowiazuje limit wkompilowany. */
  { MPC_PARAM_COMP_THRESHOLD, MPC_PTYPE_U16, MPC_ACC_RW, MPC_STORE_NV, 0, 20, 3000, 1000 },  /* Prog komparatora impulsow (DAC1 kanal 1, wzgledem VDDA). Wartosc domyslna do ustalenia w etapie E3. */
  { MPC_PARAM_AVG_MODE, MPC_PTYPE_U8, MPC_ACC_RW, MPC_STORE_NV, 0, 0, 1, 1 },  /* Usrednianie CPM i mocy dawki: 0 = stale okno AVG_WINDOW_S, 1 = adaptacyjne (okno skracane przy szybkiej zmianie czestosci). */
  { MPC_PARAM_AVG_WINDOW_S, MPC_PTYPE_U16, MPC_ACC_RW, MPC_STORE_NV, 0, 10, 3600, 60 },  /* Dlugosc okna usredniania (w trybie adaptacyjnym — okno maksymalne). */
  { MPC_PARAM_ALARM_RATE_WARN, MPC_PTYPE_U32, MPC_ACC_RW, MPC_STORE_NV, 0, 0, 100000000, 50 },  /* Prog ostrzegawczy mocy dawki; 50 = 0.5 uSv/h. 0 = wylaczony. */
  { MPC_PARAM_ALARM_RATE_HIGH, MPC_PTYPE_U32, MPC_ACC_RW, MPC_STORE_NV, 0, 0, 100000000, 1000 },  /* Prog alarmowy mocy dawki; 1000 = 10 uSv/h. 0 = wylaczony. */
  { MPC_PARAM_ALARM_DOSE, MPC_PTYPE_U32, MPC_ACC_RW, MPC_STORE_NV, 0, 0, 4000000000, 100000 },  /* Prog dawki skumulowanej; 100000 = 1 mSv. 0 = wylaczony. */
  { MPC_PARAM_ALARM_HYST, MPC_PTYPE_U8, MPC_ACC_RW, MPC_STORE_NV, 0, 0, 50, 10 },  /* Histereza powrotu ponizej progow mocy dawki. */
  { MPC_PARAM_CAL_CPM_TO_USVH, MPC_PTYPE_F32, MPC_ACC_RW, MPC_STORE_NV, MPC_PFLAG_CALIBRATION, 953267991, 1065353216, 1002096401 },  /* Wspolczynnik przeliczenia CPM -> uSv/h. Wartosc domyslna jest tymczasowa (typowa dla STS-5/SBM-20) do czasu kalibracji. */
  { MPC_PARAM_CAL_DEAD_TIME_US, MPC_PTYPE_F32, MPC_ACC_RW, MPC_STORE_NV, MPC_PFLAG_CALIBRATION, 0, 1148846080, 1128136704 },  /* Czas martwy tuby do korekcji n = m / (1 - m * tau). 0 = bez korekcji. */
  { MPC_PARAM_CAL_HV_C0, MPC_PTYPE_F32, MPC_ACC_RW, MPC_STORE_NV, MPC_PFLAG_CALIBRATION, -1035468800, 1112014848, 0 },  /* Napiecie anodowe: offset c0 (HV = c1 * HV_nom + c0). */
  { MPC_PARAM_CAL_HV_C1, MPC_PTYPE_F32, MPC_ACC_RW, MPC_STORE_NV, MPC_PFLAG_CALIBRATION, 1056964608, 1069547520, 1065353216 },  /* Napiecie anodowe: wzmocnienie c1 wzgledem nominalnego dzielnika 1:1001. */
  { MPC_PARAM_CAL_VIN_C0, MPC_PTYPE_F32, MPC_ACC_RW, MPC_STORE_NV, MPC_PFLAG_CALIBRATION, -1082130432, 1065353216, 0 },  /* Napiecie zasilania: offset c0 (V = c1 * V_nom + c0). */
  { MPC_PARAM_CAL_VIN_C1, MPC_PTYPE_F32, MPC_ACC_RW, MPC_STORE_NV, MPC_PFLAG_CALIBRATION, 1056964608, 1069547520, 1065353216 },  /* Napiecie zasilania: wzmocnienie c1 wzgledem nominalnego dzielnika 11:1. */
  { MPC_PARAM_CAL_ISENS_C0, MPC_PTYPE_F32, MPC_ACC_RW, MPC_STORE_NV, MPC_PFLAG_CALIBRATION, -1165815185, 981668463, 0 },  /* Prad zasilania: offset c0 (I = c1 * I_nom + c0). */
  { MPC_PARAM_CAL_ISENS_C1, MPC_PTYPE_F32, MPC_ACC_RW, MPC_STORE_NV, MPC_PFLAG_CALIBRATION, 1056964608, 1069547520, 1065353216 },  /* Prad zasilania: wzmocnienie c1 wzgledem nominalnego INA186A4 (G = 200, shunt 0.5 Ohm). */
};

const uint8_t mpc_sensor_ids[MPC_SENSOR_COUNT] = { MPC_SENSOR_GM_TUBE, MPC_SENSOR_HV_MONITOR, MPC_SENSOR_SUPPLY_MON, MPC_SENSOR_MCU };

const uint8_t mpc_channel_ids[MPC_CHANNEL_COUNT] = { MPC_CH_CPM, MPC_CH_DOSE_RATE, MPC_CH_DOSE_ACC, MPC_CH_TUBE_HV, MPC_CH_SUPPLY_VOLTAGE, MPC_CH_SUPPLY_CURRENT, MPC_CH_MCU_TEMP, MPC_CH_VDDA };

