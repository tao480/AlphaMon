# Heltec WiFi LoRa 32 (V3) Specification

The AlphaMon Platform currently utilizes the **Heltec WiFi LoRa 32 V3** sub-board on its main PCB. 
This board is based on the **ESP32-S3** microcontroller and the **SX1262** LoRa node chip.
It also includes a 64x128 OLED display which is used for graphical device status displays.

## 📌 Microcontroller Core
- **MCU**: ESP32-S3 (Dual-core Xtensa® 32-bit LX7, up to 240 MHz)
- **Flash**: 8MB (Standard)
- **SRAM**: 512KB
- **Connectivity**: Wi-Fi (802.11 b/g/n), Bluetooth 5 (LE)

## 📡 LoRa Connectivity
- **Chip**: Semtech SX1262
- **Frequency**: Region-dependent (433MHz, 470~510MHz, 863~928MHz)
- **Interface**: SPI

## 🔌 GPIO Pinout Map (Technical Users)

For developers needing to interface with the AlphaMon PCB, the following pins are assigned to the Heltec V3's onboard peripherals. **Exercise caution when using remaining GPIOs to avoid conflicts.**

### Onboard Peripherals
| Peripheral | Pin (GPIO) | Notes |
| :--- | :--- | :--- |
| **OLED SDA** | GPIO 17 | I2C Data |
| **OLED SCL** | GPIO 18 | I2C Clock |
| **OLED RST** | GPIO 21 | Reset Pin |
| **User Button** | GPIO 0 | PRG Button |
| **User LED** | GPIO 35 | Active High |
| **Vext Control** | GPIO 36 | Controls power to external sensors (Active Low) |

### LoRa (SX1262) Assignments
| Signal | Pin (GPIO) |
| :--- | :--- |
| **NSS** | GPIO 8 |
| **SCK** | GPIO 9 |
| **MOSI** | GPIO 10 |
| **MISO** | GPIO 11 |
| **RST** | GPIO 12 |
| **BUSY** | GPIO 13 |
| **DIO1** | GPIO 14 |

### Available GPIOs for Expansion
The following pins are typically routed or available on the AlphaMon expansion headers (Verify with your specific AlphaMon PCB version):
- **Analog Input**: GPIO 1, 2, 3, 4, 5, 6, 7
- **Digital I/O**: GPIO 41, 42, 45, 46, 47, 48

## ⚡ Power Management
- **Battery Input**: 3.7V Lithium (via SH1.25-2 connector)
- **Charging Chip**: TP4054
- **Voltage Detection**: GPIO 1 (ADC) - Use a voltage divider to monitor battery levels.

## 🛠 Programming
- **Interface**: USB-C (CP2102 USB-to-UART)
- **Baud Rate**: 115200 (Recommended)
- **Auto-Programming**: Supported (No need to hold BOOT/PRG buttons in most environments).

---
*For more detailed schematics, visit the [Heltec Documentation Portal](https://docs.heltec.org).*
