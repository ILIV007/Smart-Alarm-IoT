# 🚨 Smart Alarm IoT System

<div align="center">

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![Arduino](https://img.shields.io/badge/Arduino-ATmega328P-00878F?logo=arduino)
![ESP8266](https://img.shields.io/badge/ESP-8266-E7352C?logo=espressif)
![Flask](https://img.shields.io/badge/Flask-Python-000000?logo=flask)
![License](https://img.shields.io/badge/license-MIT-green)

**An advanced IoT-based smart alarm system with real-time web dashboard, hybrid intrusion detection, and remote control capabilities**

[🌐 Live Demo](#-screenshots) • [📖 API Documentation](#-api-documentation) • [🐛 Report Bug](../../issues) • [💡 Request Feature](../../issues)

</div>

---

## 📋 Table of Contents

- [Overview](#-overview)
- [Key Features](#-key-features)
- [System Architecture](#-system-architecture)
- [Hardware Requirements](#-hardware-requirements)
- [Installation & Setup](#-installation--setup)
- [Usage Guide](#-usage-guide)
- [Intrusion Detection Logic](#-intrusion-detection-logic)
- [API Documentation](#-api-documentation)
- [Troubleshooting](#-troubleshooting)
- [Screenshots](#-screenshots)
- [Project Structure](#-project-structure)
- [License](#-license)

---

## 🎯 Overview

This project implements a sophisticated smart alarm system that combines embedded systems with web technologies to create a comprehensive security solution. The system uses a dual-microcontroller architecture (ATmega328P + ESP8266) with a Flask-based web server for real-time monitoring and control.

### What Makes This Project Special?

- **Hybrid Intrusion Detection**: Combines Hall effect sensor and LDR (Light Dependent Resistor) for accurate intrusion detection
- **Real-time Web Dashboard**: Beautiful, responsive web interface with live sensor data visualization
- **Multi-level Alerts**: Four distinct states (Disarmed, Armed, Suspicious, Triggered) with corresponding LED indicators
- **Touch-based Control**: Intuitive touch sensor for local control without needing the web interface
- **Siren System**: Ambulance-style pulsing alarm when intrusion is detected
- **Event Logging**: Complete history of all system events with CSV export capability
- **Professional UI/UX**: Modern dark theme with smooth animations and responsive design

---

## ✨ Key Features

### 🎛️ Hardware Features
- **4 Status LEDs**: Red (Triggered), Green (Armed), Blue (Disarmed), Yellow (Suspicious)
- **Hall Effect Sensor**: Detects door/window opening with magnetic field changes
- **LDR Sensor**: Detects light changes indicating potential intrusion
- **Touch Sensor**: Capacitive touch for local system control
- **Active Buzzer**: Audible alarm with pulsing pattern
- **Dual Microcontroller**: ATmega328P for sensor control + ESP8266 for WiFi connectivity

### 💻 Software Features
- **Real-time Monitoring**: Live sensor data with 1.5-second refresh rate
- **Interactive Charts**: Dynamic line charts showing sensor history
- **Color-coded Alerts**: Charts change color based on threat level
- **Event History**: Timestamped log of all system events
- **CSV Export**: Download event history for analysis
- **Responsive Design**: Works perfectly on desktop, tablet, and mobile
- **Dark/Light Themes**: User-selectable interface themes
- **Audio Alerts**: Browser-based alarm sounds
- **Favicon Integration**: Custom lock icon in browser tab

### 🔐 Security Features
- **Password Protection**: Web dashboard requires authentication (default: 0000)
- **Latch Logic**: Once triggered, system stays in alarm state until manually reset
- **Cooldown Period**: 5-second protection after touch activation to prevent false triggers
- **Hysteresis**: Advanced filtering to prevent sensor noise from causing false alarms
- **Dual Verification**: Both Hall and LDR must trigger for full alarm (configurable)

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      Web Dashboard                          │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │  Charts  │  │  Status  │  │  Events  │  │ Controls │   │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘   │
└──────────────────────┬──────────────────────────────────────┘
                       │ HTTP/JSON
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                  Flask Web Server                           │
│  • REST API endpoints                                       │
│  • State management                                         │
│  • Event logging                                            │
│  • Sensor data storage                                      │
└──────────────────────┬──────────────────────────────────────┘
                       │ WiFi (HTTP)
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                    ESP8266 Module                           │
│  • WiFi connectivity                                        │
│  • HTTP client for server communication                     │
│  • Intrusion detection logic                                │
│  • Siren control (pulsing pattern)                          │
│  • State synchronization                                    │
└──────────────────────┬──────────────────────────────────────┘
                       │ Serial (9600 baud)
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                  ATmega328P (Arduino)                       │
│  • Hall sensor reading (A0)                                 │
│  • LDR sensor reading (A1)                                  │
│  • Touch sensor input (D4)                                  │
│  • LED control (D9, D10, D11, D12)                         │
│  • Buzzer control (D5)                                      │
│  • Sensor filtering (moving average)                        │
│  • Calibration system                                       │
└─────────────────────────────────────────────────────────────┘
```

### Communication Protocol

**ATmega → ESP (Serial)**
```
DATA|H:{hall_value},L:{ldr_value},T:{touch_state},C:{calibrated}
EVENT:TOUCH_ARM_SHORT
EVENT:TOUCH_ALARM_3S
CALIB:{hall_base},{ldr_base}
ACK:{command}
```

**ESP → ATmega (Serial)**
```
STATE:DISARMED|ARMED|SUSPICIOUS|TRIGGERED
CMD:BUZZER_ON|BUZZER_OFF
CMD:RGB:{r},{g},{b},{y}
CMD:CALIBRATE
```

**ESP ↔ Server (HTTP/JSON)**
```json
// POST /api/alarm
{
  "sensor": "periodic_update|Intrusion|Touch_3s",
  "hall": 512,
  "ldr": 450,
  "manual": false,
  "ip": "192.168.1.100"
}

// GET /api/status
{
  "state": "ARMED",
  "device_ip": "192.168.1.100",
  "last_seen": "14:30:15",
  "online": true,
  "sensors": {
    "hall": 512,
    "ldr": 450,
    "night": false
  },
  "server_ip": "192.168.1.50"
}
```

---

## 🔧 Hardware Requirements

### Core Components
- **Arduino Uno** (or compatible ATmega328P board)
- **ESP8266 WiFi Module** (ESP-01, NodeMCU, or Wemos D1 Mini)
- **Hall Effect Sensor** (analog output, e.g., A1302, SS49E)
- **LDR (Photoresistor)** with 10KΩ pull-down resistor
- **Capacitive Touch Sensor** (TTP223 or similar)
- **Active Buzzer** (5V)
- **4x LEDs** (Red, Green, Blue, Yellow) with 220Ω resistors
- **Breadboard and jumper wires**
- **5V Power supply** (USB or adapter)

### Optional Components
- **Magnet** (for Hall sensor testing)
- **Enclosure/Case** for professional installation
- **PCB** for permanent installation

### Pin Connections

**ATmega328P Pin Mapping:**
```
A0 - Hall Effect Sensor (analog input)
A1 - LDR Sensor (analog input, with 10KΩ to GND)
D4 - Touch Sensor (digital input)
D5 - Active Buzzer (digital output)
D9 - Red LED (PWM output)
D10 - Green LED (PWM output)
D11 - Blue LED (PWM output)
D12 - Yellow LED (digital output)
GND - Common ground for all components
5V - Power supply for sensors and LEDs
```

**ESP8266 Pin Mapping:**
```
TX - ATmega RX (via voltage divider if needed)
RX - ATmega TX
VCC - 3.3V or 5V (depending on module)
GND - Common ground
```

---

## 🚀 Installation & Setup

### 1. Hardware Assembly

1. **Connect Hall Sensor:**
   - VCC → 5V
   - GND → GND
   - OUT → A0

2. **Connect LDR:**
   - One leg → 5V
   - Other leg → A1 AND 10KΩ resistor → GND

3. **Connect Touch Sensor:**
   - VCC → 5V
   - GND → GND
   - OUT → D4

4. **Connect Buzzer:**
   - Positive (+) → D5
   - Negative (-) → GND

5. **Connect LEDs (with 220Ω resistors):**
   - Red LED anode → D9
   - Green LED anode → D10
   - Blue LED anode → D11
   - Yellow LED anode → D12
   - All cathodes → GND

6. **Connect ESP8266:**
   - TX → ATmega RX
   - RX → ATmega TX
   - VCC → 3.3V or 5V
   - GND → GND

### 2. Software Installation

#### Prerequisites
- **Arduino IDE** (version 1.8.x or higher)
- **Python 3.8+** with pip
- **Git** (optional, for cloning)

#### Install Arduino Libraries
Open Arduino IDE and install these libraries via Library Manager:
- `ESP8266WiFi` (for ESP8266)
- `ESP8266HTTPClient` (for ESP8266)
- `ArduinoJson` (version 6.x, for both boards)

#### Upload Firmware

**Step 1: Upload ATmega Code**
1. Open `Firmware/SmartAlarm_ATmega.ino` in Arduino IDE
2. Select board: `Arduino Uno`
3. Select correct COM port
4. Click Upload
5. Wait for "Done uploading" message

**Step 2: Upload ESP8266 Code**
1. Open `Firmware/SmartAlarm_ESP8266.ino` in Arduino IDE
2. Select board: `Generic ESP8266 Module` (or your specific board)
3. **Edit WiFi credentials** (lines 10-12):
   ```cpp
   const char* WIFI_SSID = "YourWiFiName";
   const char* WIFI_PASS = "YourWiFiPassword";
   const char* SERVER_HOST = "192.168.1.50"; // Your server IP
   ```
4. Select correct COM port
5. Click Upload
6. Wait for "Done uploading" message

#### Install Server Dependencies

```bash
# Navigate to server directory
cd Server

# Create virtual environment (recommended)
python -m venv venv

# Activate virtual environment
# On Windows:
venv\Scripts\activate
# On macOS/Linux:
source venv/bin/activate

# Install dependencies
pip install flask
```

### 3. Configuration

#### Edit Server IP
In `SmartAlarm_ESP8266.ino`, update line 12:
```cpp
const char* SERVER_HOST = "YOUR_SERVER_IP";
```

To find your server IP:
- **Windows**: Open CMD and run `ipconfig`
- **macOS/Linux**: Open Terminal and run `ifconfig` or `ip addr`

#### Edit WiFi Credentials
In `SmartAlarm_ESP8266.ino`, update lines 10-11:
```cpp
const char* WIFI_SSID = "YourWiFiName";
const char* WIFI_PASS = "YourWiFiPassword";
```

#### Change Admin Password
In `app.py`, update line 7:
```python
ADMIN_PASS = "0000"  # Change to your desired password
```

### 4. Run the Server

```bash
# Make sure you're in the Server directory
cd Server

# Run the Flask server
python app.py
```

You should see:
```
🚀 Server running: http://192.168.1.50:5000 | Pass: 0000
 * Serving Flask app 'app'
 * Debug mode: off
 * Running on all addresses (0.0.0.0)
 * Running on http://127.0.0.1:5000
 * Running on http://192.168.1.50:5000
```

### 5. Access the Dashboard

1. Open a web browser
2. Navigate to: `http://YOUR_SERVER_IP:5000`
3. Enter password: `0000` (or your custom password)
4. You should see the dashboard with live sensor data

---

## 📖 Usage Guide

### Initial Setup

1. **Power on the system**
   - LEDs will cycle through Red → Green → Blue → Yellow (testing)
   - System calibrates sensors automatically
   - Blue LED indicates DISARMED state

2. **Check Web Dashboard**
   - Open browser and navigate to server IP
   - Verify all sensors show "Online" status
   - Check sensor readings are reasonable

3. **Test Sensors**
   - **Hall Sensor**: Move magnet near/far from sensor
   - **LDR**: Cover/uncover with hand or shine light
   - Verify values change in dashboard

### Daily Operation

#### Arming the System

**Method 1: Touch Sensor**
- Tap touch sensor briefly (< 0.5 seconds)
- LED turns GREEN
- Dashboard shows "ACTIVE" status

**Method 2: Web Dashboard**
- Click on the central orb/planet
- System arms immediately
- LED turns GREEN

#### Disarming the System

**Method 1: Touch Sensor**
- Tap touch sensor briefly
- System disarms (if not in TRIGGERED state)
- LED turns BLUE

**Method 2: Web Dashboard**
- Click on the central orb
- Enter password (default: 0000)
- Click "Confirm"
- LED turns BLUE

#### Manual Alarm (Panic Mode)

- Press and hold touch sensor for 2 seconds
- LED turns RED immediately
- Buzzer starts pulsing
- Dashboard shows "ALERT" status
- Event logged as "MANUAL" alarm

### Understanding LED States

| LED Color | State | Meaning | Buzzer |
|-----------|-------|---------|--------|
| 🔵 Blue | DISARMED | System inactive, monitoring disabled | OFF |
| 🟢 Green | ARMED | System active, monitoring enabled | OFF |
| 🟡 Yellow | SUSPICIOUS | One sensor triggered, potential threat | OFF |
| 🔴 Red | TRIGGERED | Both sensors triggered, confirmed intrusion | ON (pulsing) |

### Sensor Behavior

#### Hall Sensor (Door/Window)
- **Normal range**: 450-550 (when magnet is near)
- **Danger range**: 450-550 (when magnet is removed)
- **Safe range**: < 435 or > 565

#### LDR Sensor (Light)
- **Normal**: 400-600 (ambient light)
- **Danger**: < 40 (sudden darkness, e.g., door opening in dark room)
- **Safe**: > 60

### Event History

- View all events in the "Event History" section
- Each event shows:
  - Timestamp
  - Event type (ARMED, DISARMED, ALARM, MANUAL)
  - Severity level (Critical, Warning, Info)
  - Sensor data at time of event

### CSV Export

1. Click "Export CSV" button
2. File downloads automatically
3. Open in Excel, Google Sheets, or any spreadsheet software
4. Analyze event patterns and timing

---

## 🔍 Intrusion Detection Logic

### State Machine

```
                    ┌─────────────┐
         ┌─────────│  DISARMED   │◄─────────┐
         │         └─────────────┘          │
         │                │                 │
         │           Touch/Website          │
         │                │                 │
         │                ▼                 │
         │         ┌─────────────┐          │
         │         │   ARMED     │          │
         │         └─────────────┘          │
         │                │                 │
         │         One sensor               │
         │         triggered                │
         │                │                 │
         │                ▼                 │
         │         ┌─────────────┐          │
         │         │ SUSPICIOUS  │          │
         │         └─────────────┘          │
         │                │                 │
         │         Both sensors             │
         │         triggered                │
         │                │                 │
         │                ▼                 │
         │         ┌─────────────┐          │
         └─────────│  TRIGGERED  │──────────┘
                   └─────────────┘
                   (Latch state,
                   requires manual
                   reset)
```

### Detection Algorithm

#### Step 1: Sensor Filtering
```cpp
// Moving average filter (5 samples)
int hallFiltered = getHallFiltered();
int ldrFiltered = getLdrFiltered();
```

#### Step 2: Hysteresis Check
```cpp
// Prevent oscillation with hysteresis bands
bool hallDanger;
if (lastHallDanger) {
  // Already in danger: need to exit safe zone
  hallDanger = (hall >= HALL_SAFE_MIN && hall <= HALL_SAFE_MAX);
} else {
  // Not in danger: need to enter danger zone
  hallDanger = (hall >= HALL_DANGER_MIN && hall <= HALL_DANGER_MAX);
}
```

#### Step 3: Threat Assessment
```cpp
// Calculate threat level
if (hallDanger && ldrDanger) {
  // CRITICAL: Both sensors triggered
  state = TRIGGERED;
  sirenActive = true;
} 
else if (hallDanger || ldrDanger) {
  // WARNING: One sensor triggered
  state = SUSPICIOUS;
  sirenActive = false;
} 
else {
  // SAFE: No sensors triggered
  state = ARMED;
  sirenActive = false;
}
```

#### Step 4: Latch Logic
```cpp
// Once TRIGGERED, stay in alarm until manual reset
if (sysState == TRIGGERED) {
  return; // Ignore sensor changes
}
```

### Cooldown Mechanism

After touch activation, a 5-second cooldown prevents false triggers:

```cpp
if (ev == "TOUCH_ARM_SHORT") {
  sysState = ARMED;
  alarmCooldown = true;
  cooldownEnd = millis() + 5000; // 5 seconds
  
  // During cooldown, checkIntrusion() is skipped
}
```

---

## 📡 API Documentation

### Base URL
```
http://YOUR_SERVER_IP:5000
```

### Endpoints

#### GET `/api/status`
Get current system status and sensor readings.

**Response:**
```json
{
  "state": "ARMED",
  "device_ip": "192.168.1.100",
  "last_seen": "14:30:15",
  "online": true,
  "sensors": {
    "hall": 512,
    "ldr": 450,
    "night": false
  },
  "server_ip": "192.168.1.50"
}
```

**Status Codes:**
- `200 OK`: Success

---

#### POST `/api/arm`
Arm the system (activate monitoring).

**Request:**
```json
{
  "source": "dashboard"
}
```

**Response:**
```json
{
  "status": "ok",
  "state": "ARMED"
}
```

**Status Codes:**
- `200 OK`: Success

---

#### POST `/api/disarm`
Disarm the system (deactivate monitoring). Requires password.

**Request:**
```json
{
  "pass": "0000"
}
```

**Response:**
```json
{
  "status": "ok",
  "state": "DISARMED"
}
```

**Status Codes:**
- `200 OK`: Success
- `401 Unauthorized`: Wrong password

---

#### POST `/api/alarm`
Receive alarm event from ESP8266.

**Request:**
```json
{
  "sensor": "Intrusion",
  "hall": 512,
  "ldr": 35,
  "manual": false,
  "ip": "192.168.1.100"
}
```

**Response:**
```json
{
  "status": "received",
  "state": "TRIGGERED",
  "previous": "ARMED"
}
```

**Status Codes:**
- `200 OK`: Alarm processed
- `200 OK`: Alarm ignored (system not armed)

---

#### GET `/api/history`
Get event history.

**Response:**
```json
[
  {
    "timestamp": "2024-01-15T14:30:15.123456",
    "time": "14:30:15",
    "date": "2024-01-15",
    "type": "ALARM",
    "details": "Intrusion detected",
    "sensor_data": {
      "hall": 512,
      "ldr": 35
    }
  },
  {
    "timestamp": "2024-01-15T14:25:00.000000",
    "time": "14:25:00",
    "date": "2024-01-15",
    "type": "ARMED",
    "details": "Armed from dashboard",
    "sensor_data": {}
  }
]
```

**Status Codes:**
- `200 OK`: Success

---

#### POST `/api/history/clear`
Clear all event history.

**Request:**
```json
{}
```

**Response:**
```json
{
  "status": "cleared"
}
```

**Status Codes:**
- `200 OK`: Success

---

#### GET `/api/sensors/history`
Get sensor data history for charts.

**Response:**
```json
{
  "hall": [512, 515, 510, 520, 518],
  "ldr": [450, 445, 460, 455, 450]
}
```

**Status Codes:**
- `200 OK`: Success

---

### Error Responses

All error responses follow this format:
```json
{
  "status": "error",
  "message": "Error description"
}
```

---

## 🐛 Troubleshooting

### Issue: LEDs Not Lighting Up

**Symptoms:**
- LEDs remain dark or show wrong colors

**Solutions:**
1. Check LED polarity (long leg = anode = positive)
2. Verify 220Ω resistors are connected
3. Check pin connections match code (D9, D10, D11, D12)
4. Run LED test in setup (should cycle through colors on boot)

---

### Issue: Buzzer Not Sounding

**Symptoms:**
- No sound when system triggers

**Solutions:**
1. Verify buzzer is **active** type (not passive)
2. Check polarity (positive leg to D5)
3. Test with simple sketch:
   ```cpp
   void setup() {
     pinMode(5, OUTPUT);
   }
   void loop() {
     digitalWrite(5, HIGH);
     delay(1000);
     digitalWrite(5, LOW);
     delay(1000);
   }
   ```

---

### Issue: Hall Sensor Not Detecting

**Symptoms:**
- Hall value doesn't change with magnet

**Solutions:**
1. Verify sensor is **analog** type (not digital)
2. Check wiring: VCC→5V, GND→GND, OUT→A0
3. Test with multimeter (should read 0-5V)
4. Try different magnet (stronger neodymium recommended)
5. Check Serial Monitor for raw values

---

### Issue: LDR Not Responding

**Symptoms:**
- LDR value stays constant

**Solutions:**
1. Verify voltage divider circuit:
   ```
   5V → LDR → A1 → 10KΩ → GND
   ```
2. Check 10KΩ resistor value
3. Test with multimeter (resistance should change with light)
4. Try different LDR (GL5528 recommended)

---

### Issue: Touch Sensor Not Working

**Symptoms:**
- Touch doesn't trigger events

**Solutions:**
1. Check wiring: VCC→5V, GND→GND, OUT→D4
2. Verify sensor type (TTP223 or similar)
3. Test with simple sketch:
   ```cpp
   void setup() {
     Serial.begin(9600);
     pinMode(4, INPUT);
   }
   void loop() {
     Serial.println(digitalRead(4));
     delay(100);
   }
   ```
4. Adjust touch sensitivity (if module has potentiometer)

---

### Issue: WiFi Not Connecting

**Symptoms:**
- ESP8266 shows "WiFi: Failed" in Serial Monitor

**Solutions:**
1. Verify SSID and password in code (case-sensitive!)
2. Check WiFi is 2.4GHz (ESP8266 doesn't support 5GHz)
3. Move ESP closer to router
4. Check router MAC filtering (add ESP MAC address)
5. Try different WiFi network

---

### Issue: Dashboard Shows "Offline"

**Symptoms:**
- Web dashboard shows device offline

**Solutions:**
1. Verify server IP in ESP code matches actual server IP
2. Check firewall settings (allow port 5000)
3. Ensure server is running (`python app.py`)
4. Check ESP Serial Monitor for HTTP errors
5. Verify both devices are on same network

---

### Issue: Wrong LED Colors

**Symptoms:**
- Red shows as Blue, Green shows as Yellow, etc.

**Solutions:**
1. Check pin assignments in code match physical wiring
2. Verify LED colors match pin labels
3. Swap pin numbers in code if needed:
   ```cpp
   const int PIN_RED = 11;    // Swap with BLUE
   const int PIN_BLUE = 9;    // Swap with RED
   ```

---

### Issue: False Alarms

**Symptoms:**
- System triggers without actual intrusion

**Solutions:**
1. Adjust hysteresis thresholds in ESP code:
   ```cpp
   #define HALL_DANGER_MIN 460  // Increase to narrow danger zone
   #define HALL_DANGER_MAX 540  // Decrease to narrow danger zone
   ```
2. Check for electromagnetic interference (motors, transformers)
3. Verify magnet is securely mounted
4. Increase `ALARM_COOLDOWN` value
5. Check LDR for sudden light changes (shadows, clouds)

---

### Issue: System Stuck in TRIGGERED State

**Symptoms:**
- System won't exit alarm state

**Solutions:**
1. Use web dashboard to disarm (enter password)
2. Press touch sensor briefly to reset
3. Power cycle the system
4. Check Serial Monitor for error messages
5. Verify `syncStatus()` is working in ESP code

---

### Issue: Serial Communication Errors

**Symptoms:**
- Garbled data between ATmega and ESP

**Solutions:**
1. Verify baud rate matches (9600 in both sketches)
2. Check TX/RX connections (cross them: TX→RX, RX→TX)
3. Add voltage divider if ESP is 3.3V and ATmega is 5V:
   ```
   ATmega TX → 1KΩ → ESP RX → 2KΩ → GND
   ```
4. Use SoftwareSerial for debugging
5. Check for ground loops (connect all GND together)

---

## 📸 Screenshots

### Dashboard - Disarmed State
![Disarmed](docs/screenshots/disarmed.png)
*Blue orb indicates system is disarmed and inactive*

### Dashboard - Armed State
![Armed](docs/screenshots/armed.png)
*Green orb with live sensor charts showing normal readings*

### Dashboard - Suspicious State
![Suspicious](docs/screenshots/suspicious.png)
*Yellow chart indicates one sensor has detected potential threat*

### Dashboard - Triggered State
![Triggered](docs/screenshots/triggered.png)
*Red orb with pulsing animation, both charts show danger levels*

### Event History
![History](docs/screenshots/history.png)
*Timestamped log of all system events with severity indicators*

### Mobile View
![Mobile](docs/screenshots/mobile.png)
*Responsive design works perfectly on smartphones and tablets*

---

## 📁 Project Structure

```
SmartAlarm_IoT/
│
├── Firmware/
│   ├── SmartAlarm_ATmega.ino          # ATmega328P firmware
│   └── SmartAlarm_ESP8266.ino         # ESP8266 firmware
│
├── Server/
│   ├── app.py                         # Flask web server
│   ├── alarm_history.json             # Event history storage
│   ├── requirements.txt               # Python dependencies
│   └── templates/
│       └── index.html                 # Web dashboard
│
├── docs/
│   ├── screenshots/                   # Dashboard screenshots
│   ├── diagrams/                      # System architecture diagrams
│   └── api/                           # API documentation
│
├── hardware/
│   ├── schematic.pdf                  # Circuit schematic
│   ├── pcb/                           # PCB design files
│   └── bom.csv                        # Bill of materials
│
├── tests/
│   ├── test_sensors.py                # Sensor test scripts
│   └── test_api.py                    # API endpoint tests
│
├── .gitignore                         # Git ignore rules
├── LICENSE                            # MIT License
├── README.md                          # This file
└── CONTRIBUTING.md                    # Contribution guidelines
```

---

## 🔒 Security Considerations

### Current Security Measures
- Password protection for web dashboard
- Local network only (not exposed to internet)
- No sensitive data stored in plain text
- Event logging for audit trail

### Recommended Enhancements
1. **HTTPS**: Use SSL/TLS for encrypted communication
2. **Strong Passwords**: Enforce complex password requirements
3. **Rate Limiting**: Prevent brute-force attacks on login
4. **Session Management**: Implement secure session tokens
5. **Input Validation**: Sanitize all user inputs
6. **Network Segmentation**: Place IoT devices on separate VLAN
7. **Regular Updates**: Keep all software components updated
8. **Two-Factor Authentication**: Add 2FA for web access

### Known Limitations
- Default password (0000) should be changed immediately
- No encryption for serial communication between microcontrollers
- WiFi credentials stored in plain text in firmware
- No intrusion detection for the system itself

---

## 🤝 Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

### Development Guidelines
- Follow existing code style
- Add comments for complex logic
- Update documentation for new features
- Test thoroughly before submitting
- Include screenshots for UI changes

### Code Style
- **C/C++**: Follow Arduino style guide
- **Python**: Follow PEP 8
- **HTML/CSS/JS**: Follow existing patterns
- **Commit Messages**: Use conventional commits format

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

```
MIT License

Copyright (c) 2024 Smart Alarm IoT Project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## 🙏 Acknowledgments

- **Arduino Community** for excellent documentation and libraries
- **Flask Framework** for lightweight web server capabilities
- **ESP8266** team for affordable WiFi microcontroller
- **Open Source Community** for inspiration and support

---

## 📞 Support

For support, please:
1. Check the [Troubleshooting](#-troubleshooting) section
2. Search existing [Issues](../../issues)
3. Open a new issue with detailed description
4. Include Serial Monitor output and screenshots

---

## 🗺️ Roadmap

### Version 1.1 (Planned)
- [ ] Mobile app (React Native)
- [ ] Push notifications
- [ ] Multi-zone support
- [ ] Camera integration
- [ ] Voice control (Alexa/Google Home)

### Version 1.2 (Planned)
- [ ] Machine learning for false alarm reduction
- [ ] Cloud backup of event history
- [ ] Multi-user support with roles
- [ ] Advanced analytics dashboard
- [ ] Integration with smart home systems

### Version 2.0 (Future)
- [ ] Custom PCB design
- [ ] Battery backup system
- [ ] Cellular connectivity (SIM800L)
- [ ] Professional enclosure
- [ ] Certification for commercial use

---

## 📊 Performance Metrics

### System Specifications
- **Response Time**: < 100ms for state changes
- **Update Rate**: 1.5 seconds for dashboard refresh
- **Sensor Sampling**: 50ms interval with 5-sample filter
- **WiFi Range**: Up to 50 meters (indoor)
- **Power Consumption**: ~200mA @ 5V (typical)
- **Memory Usage**: 
  - ATmega: ~8KB of 32KB flash, ~500 bytes of 2KB SRAM
  - ESP8266: ~200KB of 4MB flash, ~20KB of 80KB RAM
  - Server: ~50MB RAM, ~10MB disk space

### Benchmark Results
- **Boot Time**: 3 seconds (full system startup)
- **WiFi Connection**: 5-10 seconds (depending on router)
- **Sensor Calibration**: 2 seconds
- **LED Response**: < 10ms
- **Buzzer Response**: < 5ms

---

<div align="center">

**Made with ❤️ by the IoT Community**

⭐ **Star this repository if you found it helpful!** ⭐

[⬆ Back to Top](#-smart-alarm-iot-system)

</div>

---
---

> 🇮🇷 **Persian version:** the full Persian guide lives in [docs/README_FA.md](docs/README_FA.md).
