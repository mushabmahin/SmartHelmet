# SmartHelmet AI – Rider Safety & Emergency Response System

An  IoT safety helmet designed for real-time accident detection, emergency alerting, and rider assistance using embedded systems, sensor fusion, GPS localization, and GSM communication.

---

# Overview

Road accidents involving two-wheelers often suffer from delayed emergency response, especially in isolated locations. Traditional helmets provide physical protection but cannot autonomously detect accidents or contact emergency services.

SmartHelmet AI addresses this problem by integrating real-time sensing, embedded intelligence, and autonomous communication into a wearable safety system capable of:

- Detecting crash-like events
- Triggering emergency alerts
- Sending live GPS location
- Providing fail-safe manual override
- Monitoring rider helmet status
- Managing thermal comfort automatically

---

# Features

- Real-time accident detection using MPU6050 accelerometer + gyroscope
- GPS-based emergency location tracking
- GSM-powered SMS and emergency calling
- 10-second false-alert cancellation system
- Helmet wear detection using IR sensor
- Automatic thermal fan control
- Embedded real-time decision engine using ESP32
- Event-driven emergency response workflow

---

# System Architecture

## Input Layer
- MPU6050 (Impact + Motion Detection)
- IR Sensor (Helmet Wear Detection)
- DHT11 (Temperature Monitoring)
- Neo-7M GPS Module

## Processing Layer
- ESP32 Embedded Controller
- Sensor Fusion Logic
- Real-Time Event Detection
- Emergency Decision Engine

## Response Layer
- SIM800L GSM Module
- SMS Alert System
- Automated Emergency Calls
- Buzzer Alert System
- Cooling Fan Automation

---

# Workflow

1. Helmet continuously monitors acceleration, orientation, GPS, and helmet-wear status
2. ESP32 processes sensor data in real time
3. If abnormal impact threshold is detected:
   - Countdown timer activates
   - Buzzer warning starts
4. Rider can cancel alert within 10 seconds
5. If not cancelled:
   - SMS alert is sent
   - Live GPS location is shared
   - Emergency call is initiated automatically

---

# Hardware Components

| Component | Purpose |
|---|---|
| ESP32 | Main controller and decision engine |
| MPU6050 | Impact and tilt detection |
| Neo-7M GPS | Real-time location tracking |
| SIM800L GSM | SMS + emergency calling |
| IR Sensor | Helmet wear detection |
| DHT11 | Temperature monitoring |
| Buzzer | Local warning alerts |
| Cooling Fan | Thermal comfort automation |

---

# Performance Metrics

- Real-time sensing loop: **50–100 Hz**
- Event recognition latency: **80–150 ms**
- GPS accuracy: **3–8 meters**
- Emergency dispatch time: **6–10 seconds**

---

# Tech Stack

- Embedded C++
- ESP32
- MPU6050
- TinyGPS++
- SIM800L GSM
- DHT11
- Arduino Framework
- Serial Communication

---

# Repository Structure

```bash
smarthelmet-ai/
│
├── firmware/
│   └── smarthelmet.ino
│
├── docs/
│   └── report.pdf
│
├── images/
│   ├── prototype.jpg
│   └── architecture.png
│
└── README.md
```



Installation & Setup
# Installation & Setup

## 1. Clone Repository

```bash
git clone https://github.com/mushabmahin/SmartHelmet.git
```

## 2. Open in Arduino IDE

Load:

```bash
firmware/smarthelmet.ino
```

## 3. Install Required Libraries

- TinyGPS++
- DHT Sensor Library
- Wire Library

## 4. Configure Emergency Number

```cpp
const char* emergencyNumber = "+91XXXXXXXXXX";
```

## 5. Upload to ESP32

Select:
- ESP32 Board
- Correct COM Port

Upload firmware.

Future Improvements
TinyML-based crash classification
Mobile application integration
Cloud event analytics
BLE/WiFi telemetry
Multi-contact emergency broadcasting
Real-time health monitoring
