# Engine Monitoring System

## 📌 Introduction

The **Engine Monitoring System** is a software designed to monitor and analyze aircraft engine performance in real-time. Engine failures can lead to catastrophic incidents, making pre-flight engine testing crucial. This software collects data from multiple sensors via a serial port and provides graphical visualization and data logging capabilities.

## 🎯 Features

- 📡 **Real-time Data Acquisition**: Receives data via a serial port using a predefined protocol.
- 📊 **Graphical Representation**: Displays critical engine parameters using gauges and warning indicators.
- 📋 **Comprehensive Data Table**: Shows all sensor readings in a structured format.
- ⚙️ **Configurable Serial Port Settings**: Allows users to set baud rate, parity, and stop bits.
- 📁 **Data Logging**: Stores all received data in an **Excel file** for further analysis.

## 🔧 System Components

1. **Serial Communication with Sensors**: Receives engine data through a **115200 BaudRate** serial connection.
2. **Graphical Dashboard**: Displays critical engine data using circular gauges and warning lights.
3. **Tabular Data View**: Presents all sensor data in an organized table format.
4. **Settings Page**: Allows users to configure serial port parameters and control data acquisition.
5. **Excel Data Storage**: Saves all incoming data for future analysis.

## 🔄 Data Protocol

The software uses a structured communication format:

- **Baud Rate**: 115200
- **Parity**: Odd
- **Stop Bit**: One
- **Data Structure**:
  - **HEADER**: 4 Bytes (`0xA5 A5 A5 A5`)
  - **Message Counter**: 1 Byte
  - **ID Number**: 1 Byte
  - **Sensor Data**: 8 Bytes per reading
  - **Checksum**: 2 Bytes
  - **FOOTER**: 1 Byte (`0x55`)

### 📡 Supported Sensor Data

| Parameter              | Min | Max  | ID (HEX) |
|------------------------|-----|------|----------|
| Oil Pressure           | 0   | 1000 | 01       |
| Oil Temperature        | 0   | 400  | 02       |
| Fuel Flow              | 0   | 800  | 03       |
| Fuel                   | 0   | 800  | 04       |
| Exhaust Gas Temp (EGT) | 0   | 400  | 05       |
| Torque                 | 0   | 400  | 06       |
| Indicated Power        | 0   | 400  | 07       |
| Fractional Powr        | 0   | 400  | 08       |
| Thermal Efficiency     | 0   | 100  | 09       |
| Air-Fuel Ratio         | 0   | 20   | 0A       |
| Motor Speed            | 0   | 1000 | 0B       |
| Output Air Speed       | 0   | 1000 | 0C       |
| Vibration              | 0   | 100  | 0D       |
| Body Temperature       | 0   | 400  | 0E       |
| Air Temperature        | 0   | 400  | 0F       |

### ⚠️ Sensor Error Codes

Each sensor has an error flag (0 = OK, 1 = ERROR) for quick fault detection.

## 📂 Data Storage

- All data is logged into an **Excel file**.
- Each row represents a data packet received from the engine sensors.
- Logging starts when the **Start** button is pressed and stops upon clicking **Stop**.

## 🖥️ User Interface

- **Circular Gauges** for real-time visualization of:
  - Oil Pressure, Oil Temperature, Fuel Level, Torque, Motor Speed
- **Warning Lights**: Indicate sensor errors (red for fault, green for normal operation).
- **Settings Page**: Serial port configuration and test initiation.

## 🛠️ Testing

- The software is tested using virtual serial ports on **Desktop**.
- Simulated sensor data is transmitted to verify real-time monitoring and logging functions.

## 🚀 Getting Started

### Prerequisites

- **Qt Framework** (Only Qt-based UI is supported)
- **C++ Development Environment**
- **Excel-compatible spreadsheet software** for reviewing logged data.

### Installation

1. Clone the repository:

   ```sh
   git clone https://github.com/moh-skec/Engine-Monitoring-System.git
   ```

2. Open the project in **Qt Creator**.
3. Configure the serial port settings.
4. Run the application and start monitoring.
