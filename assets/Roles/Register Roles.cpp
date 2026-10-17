//The following is a list of the current valid AlphaMon Role numbers, expressed as a C++ enum. 
//You'll note that both single-phase and three-phase devices are supported.

//Mapping values that can optionally be assigned to Modbus Registers to identify their role.
//Maps the registers to their roles, especially those involved in display outputs.

enum eRegRole : uint8_t
  {
  NoType      = 0, //these registers don't have a key role

  //Grid-related types first
  GridFreq    = 1,  //Grid Frequency
  GridVolts_A = 2,  //Grid Volts, L1
  GridVolts_B = 3,  //Grid Volts, L2
  GridVolts_C = 4,  //Grid Volts, L3
  GridAmps_A  = 6,  //Grid Amps, L1
  GridAmps_B  = 7,  //Grid Amps, L2
  GridAmps_C  = 8,  //Grid Amps, L3
  GridWatts_A  = 9,  //Grid Watts, L1
  GridWatts_B  = 10,  //Grid Watts, L2
  GridWatts_C  = 11,  //Grid Watts, L3
  GridWatts    = 12,  //Grid Watts, All phases

  // Grid-related types (under GridWatts)
  GridPF_A     = 13, // Grid Power Factor, L1
  GridPF_B     = 14, // Grid Power Factor, L2
  GridPF_C     = 15, // Grid Power Factor, L3
  GridPF       = 16, // Grid Power Factor, All phases/Generic (e.g. DDS6619)

  //Grid power consumption
  GridInAmps_A  = 20, //Grid Amps being consumed L1
  GridInAmps_B  = 21, //Grid Amps being consumed L2
  GridInAmps_C  = 22, //Grid Amps being consumed L3
  GridInWatts_A = 23, //Grid Watts being consumed L1
  GridInWatts_B = 24, //Grid Watts being consumed L2
  GridInWatts_C = 25, //Grid Watts being consumed L3
  GridInWatts   = 26, //Grid Watts being consumed - All phases
  //Grid power dispatch
  GridExAmps_A  = 30, //Grid Amps being exported L1
  GridExAmps_B  = 31, //Grid Amps being exported L2
  GridExAmps_C  = 32, //Grid Amps being exported L3
  GridExWatts_A = 33, //Grid Watts being exported L1
  GridExWatts_B = 34, //Grid Watts being exported L2
  GridExWatts_C = 35, //Grid Watts being exported L3
  GridExWatts   = 36, //Grid Watts being exported - All phases

  //Inverter's Grid-tied Registers
  InvertFreq    = 40,  //The inverter's non-UPS freq (Probably the same metric as GridFreq)
  InvertVolts_A = 41,  //Inverter Volts, L1
  InvertVolts_B = 42,  //Inverter Volts, L2
  InvertVolts_C = 43,  //Inverter Volts, L3
  InvertAmps_A  = 44,  //Inverter Amps, L1
  InvertAmps_B  = 45,  //Inverter Amps, L2
  InvertAmps_C  = 46,  //Inverter Amps, L3
  InvertWatts_A = 47,  //Inverter Watts, L1
  InvertWatts_B = 48,  //Inverter Watts, L2
  InvertWatts_C = 49,  //Inverter Watts, L3
  InvertPvCapacity = 50,  //Rated capacity of the inverter

  // Solar DC Registers / Inverter's Grid-Tied (under InvertPvCapacity)
  InvertPF_A   = 51, // Inverter Power Factor, L1
  InvertPF_B   = 52, // Inverter Power Factor, L2
  InvertPF_C   = 53, // Inverter Power Factor, L3
  InvertPF     = 54, // Inverter Power Factor, Total

  //Inverter's UPS-tied Registers
  UpsFreq    = 60,  //The inverter's UPS output freq.
  UpsVolts_A = 61,  //UPS Volts, L1
  UpsVolts_B = 62,  //UPS Volts, L2
  UpsVolts_C = 63,  //UPS Volts, L3
  UpsAmps_A  = 64,  //UPS Load Amps, L1 
  UpsAmps_B  = 65,  //UPS Load Amps, L2
  UpsAmps_C  = 66,  //UPS Load Amps, L3
  UpsWatts_A = 67,  //UPS Load Watts, L1
  UpsWatts_B = 68,  //UPS Load Watts, L2
  UpsWatts_C = 69,  //UPS Load Watts, L3

  //Solar DC Registers
  SdcVolts_1 = 80,  //Solar array DC volts
  SdcVolts_2 = 81,  //Solar array DC volts
  SdcVolts_3 = 82,  //Solar array DC volts
  SdcVolts_4 = 83,  //Solar array DC volts
  SdcVolts_5 = 84,  //Solar array DC volts
  SdcVolts_6 = 85,  //Solar array DC volts
  SdcAmps_1  = 86,  //Solar array DC Amps
  SdcAmps_2  = 87,  //Solar array DC Amps
  SdcAmps_3  = 88,  //Solar array DC Amps
  SdcAmps_4  = 89,  //Solar array DC Amps
  SdcAmps_5  = 90,  //Solar array DC Amps
  SdcAmps_6  = 91,  //Solar array DC Amps
  SdcWatts_1  = 92,  //Solar array DC Watts
  SdcWatts_2  = 93,  //Solar array DC Watts
  SdcWatts_3  = 94,  //Solar array DC Watts
  SdcWatts_4  = 95,  //Solar array DC Watts
  SdcWatts_5  = 96,  //Solar array DC Watts
  SdcWatts_6  = 97,  //Solar array DC Watts

  //Smart Meter Registers
  MeterFreq    = 110,  //The Meter's measured freq.
  MeterVolts_A = 111,  //Meter Volts, L1
  MeterVolts_B = 112,  //Meter Volts, L2
  MeterVolts_C = 113,  //Meter Volts, L3
  MeterAmps_A  = 114,  //Meter Load Amps, L1 
  MeterAmps_B  = 115,  //Meter Load Amps, L2
  MeterAmps_C  = 116,  //Meter Load Amps, L3
  MeterWatts_A = 117,  //Meter Load Watts, L1
  MeterWatts_B = 118,  //Meter Load Watts, L2
  MeterWatts_C = 119,  //Meter Load Watts, L3

  // Smart Meter Registers (under MeterWatts_C)
  MeterPF_A    = 120, // Meter Power Factor, L1
  MeterPF_B    = 121, // Meter Power Factor, L2
  MeterPF_C    = 122, // Meter Power Factor, L3
  MeterPF      = 123, // Meter Power Factor, Total
  
  //Battery Registers
  BattSoC     = 150,  //Battery State_of_Charge (SoC %)
  BattVolts   = 151,  //Battery Voltage
  BattAmps    = 152,  //Battery Amps
  BattWatts   = 153,  //Battery Watts
  BattReserve = 154,  //Battery Reserve %
  BattCutSoC  = 155,  //Battery Cut-off Soc%

  //System-level Registers
  CumPvEnergy   = 160,  //Cumulative PV energy of the inverter generated 
  SysPvEnergy   = 161,  //System-level cumulative PV energy generated 
  CumGridExport = 162, //Total energy exported to the grid
  CumGridImport = 163, //Total energy consumed from the grid

  //Temperature Measurement
  TempC         = 170,  //Temperature Celsius
  TempF         = 171,  //Temp Fahrenheit
  
  //Pseudo Registers
  CalcSolarWatts = 180,  //Total Solar Watts (calculated)
  CalcLoadWatts  = 181,  //Total Load Watts  (calculated)

  //AEMO Price Data
  AemoPrice = 190,  //Wholesale price data received from an AEMO data feed

  //Regs unique to AlphaESS 
  DateTime  = 200,
  YearMonth = 201,
  DayHour   = 202,
  MinSec    = 203,
  SerialReg = 204,

  // --- High-Efficiency Group Request Tokens ---
  Group_Start        = 240, // Group start placeholder
  Group_GridWatts    = 241, // Groups GridWatts_A, GridWatts_B, GridWatts_C
  Group_GridIn       = 242, // Groups GridInWatts_A, GridInWatts_B, GridInWatts_C
  Group_GridEx       = 243, // Groups GridExWatts_A, GridExWatts_B, GridExWatts_C
  Group_SolarDcWatts = 244, // Groups SdcWatts_1 through SdcWatts_6
  Group_InvertWatts  = 245, // Groups InvertWatts_A, InvertWatts_B, InvertWatts_C
  Group_UpsWatts     = 246, // Groups UpsWatts_A, UpsWatts_B, UpsWatts_C
  Group_MeterWatts   = 247, // Groups MeterWatts_A, MeterWatts_B, MeterWatts_C
  Group_BatteryBasic = 248, // Groups BattSoC, BattVolts, BattAmps, BattWatts, BattReserve

  // Last value for vbounds check only
  RegTypeLast  = 248

  };  

