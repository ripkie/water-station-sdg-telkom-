# Water Station

IoT Water Station berbasis ESP32-S3 untuk monitoring penggunaan dispenser.

Project ini dikembangkan oleh 2 orang dan digunakan untuk deployment di Landmark Tower.

## Functions

- Menghitung volume air yang diambil.
- Menghitung jumlah aktivitas pengambilan air.
- Mengirim data monitoring ke API server melalui WiFi.
- Menyediakan data untuk monitoring dan analisis penggunaan dispenser.

## Hardware

- ESP32-S3 N16R8
- HC-SR04 Ultrasonic Sensor
- IR Sensor
- Power Supply 5V

## Pin Configuration

| Component | ESP32-S3 |
|---|---|
| IR Sensor | GPIO 17 |
| HC-SR04 TRIG | GPIO 41 |
| HC-SR04 ECHO | GPIO 42 |
| HC-SR04 VCC | 5V |
| HC-SR04 GND | GND |

> HC-SR04 ECHO menghasilkan 5V. Gunakan voltage divider sebelum masuk ke GPIO ESP32-S3.

## System Workflow

Person
↓
Take Water
↓
IR Sensor
↓
ESP32-S3
├── Usage Counter
└── HC-SR04
    ↓
Water Volume
↓
Data Processing
↓
HTTP Request
↓
API Server
↓
Database / Dashboard

## People / Usage Counter

IR sensor digunakan untuk mendeteksi aktivitas pengambilan air.

Setiap aktivitas yang memenuhi kondisi deteksi → counter bertambah 1.

Counter menunjukkan jumlah aktivitas pengambilan air yang terdeteksi sensor, bukan identitas individu.

## Water Volume

HC-SR04 digunakan untuk mengukur perubahan ketinggian air.

Distance
↓
Water Level
↓
Volume Calculation
↓
Liter

## Project Structure

water-station/
├── include/
│   ├── README
│   └── secrets.example.h
├── lib/
│   └── README
├── src/
│   └── main.cpp
├── test/
│   └── README
├── .gitignore
├── platformio.ini
└── README.md

## Configuration

Credential disimpan lokal di:

include/secrets.h

Buat dari template:

Copy-Item include/secrets.example.h include/secrets.h

Contoh:

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define API_URL "YOUR_API_URL"

`secrets.h` tidak boleh di-commit ke repository.

## Installation

Clone repository:

git clone <REPOSITORY_URL>
cd water-station

Buka project menggunakan VS Code + PlatformIO.

## Upload

Build:

pio run

Upload:

pio run --target upload

Serial Monitor:

pio device monitor

## Data

Contoh payload:

{
  "liter": 0,
  "count": 0
}

Struktur payload mengikuti API server.

## Development

- PlatformIO
- Arduino Framework
- ESP32-S3
- C++
- WiFi
- HTTP

## Project Status

Public Portfolio Project

Developed as part of an IoT dispenser monitoring project deployed at Telkom Landmark Tower.

## Contributors

- Aria Aura Rachman
- M Rifki Widya R

Developed by a 2-person team.

## License

License and usage rights are subject to the applicable project agreement.
