# IoT ATM with Fingerprint Auth & Cash Dispensing

## Description  
A secure IoT-based ATM system utilizing fingerprint authentication (FPM10A), a 4x4 keypad, and WiFi connectivity for real-time balance checks and cash transactions. Integrates a 20x4 LCD, servo-controlled cash gate, and stepper motor for dispensing notes. Built with Arduino/C++ on ESP32, enabling remote database syncing via HTTP requests.

## Features  
- 🔒 Fingerprint-based user authentication  
- 💵 Stepper motor-driven cash dispensing (100 notes max)  
- 📶 WiFi connectivity for balance updates (API integration)  
- 🎚️ Keypad input for transaction controls  
- 📟 20x4 I2C LCD for system feedback  
- 🔊 Audio alerts via buzzer  

## Pin Mapping  
| Component       | GPIO Pins                |  
|-----------------|--------------------------|  
| Keypad (Rows)   | 36, 39, 34, 35           |  
| Keypad (Columns)| 32, 33, 25, 26           |  
| Buzzer          | 19                       |  
| Stepper Motor   | 13, 12, 14, 27           |  
| Servo Motor     | 4                        |  
| LCD I2C         | 0x27 (SDA/SCL default)   |  
| Fingerprint Sensor | UART2 (RX2=16, TX2=17) |  

*Note: Pin assignments may vary depending on microcontroller model. Verify connections against your board's spec sheet.*

## Usage  
1. **Initial Setup**:  
   - Connect to WiFi via hardcoded SSID/PWD  
   - LCD guides through fingerprint enrollment (press 'D' + ID entry)  

2. **Transaction Flow**:  
   - Scan registered fingerprint → Enter amount → Automatic:  
     - Balance validation via cloud API  
     - Cash dispensing (notes counted on LCD)  
     - Servo-controlled gate actuation  

3. **Error Handling**:  
   - Invalid amounts → Buzzer alerts + LCD warnings  
   - Server failures → Local error messages  

## Dependencies  
- [Adafruit Fingerprint Sensor Library](https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library)  
- `Keypad`, `Stepper`, `LiquidCrystal_I2C` (Arduino Library Manager)  
- ESP32 WiFi/HTTPClient stack  

*Diagrams in project docs may show illustrative component layouts - verify against actual hardware configuration.*
```
