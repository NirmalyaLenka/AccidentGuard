# AccidentGuard

A small device you wear on your vehicle or body that detects accidents automatically, tracks your location with GPS, and calls for help if you do not respond.

Built using an ESP32, an ADXL345 accelerometer, a NEO-6M GPS module, a buzzer, a flashlight LED, and a touch button. The device talks to your phone over Bluetooth (BLE).

---

## What It Does

**Accident Detection**
The device constantly reads the accelerometer. If it detects a sudden jolt or impact above a set threshold (like a vehicle crash), it starts a 60-second countdown. During those 60 seconds, a buzzer beeps and the LED flashes to let you know it triggered.

**If You Are Fine**
Tap the touch button 3 times within 3 seconds. The alert cancels. Everything goes back to normal.

**If You Do Not Respond**
After 60 seconds with no cancel, the device triggers the emergency mode. The buzzer runs at full power for 1 minute. The LED flashes to help rescuers find you. It sends your GPS location to your emergency contacts via the companion phone app using BLE.

**GPS Location**
The GPS module tracks your current coordinates. When an emergency happens, it sends a Google Maps link to your emergency contacts so they know exactly where you are.

**Anti-Theft Mode**
You can turn on anti-theft mode when your vehicle is parked. If someone moves the vehicle, the buzzer and LED fire immediately. Tap the button 3 times to turn it off. To toggle anti-theft mode, tap 5 times quickly while the device is idle.

**BLE Transmission**
The device broadcasts data over Bluetooth to a nearby phone. The companion app on your phone handles the actual SMS and phone calls to emergency contacts. The device sends a JSON payload every second with state, GPS coordinates, and sensor status.

---

## Hardware You Need

- ESP32 DevKit (any 38-pin or 30-pin version)
- ADXL345 accelerometer module
- NEO-6M GPS module with antenna
- Active buzzer (5V, loud)
- 1 or more bright white LEDs
- A piece of wire or small metal plate for the touch button
- 330 ohm resistor for the LED
- USB cable and charger, or a LiPo battery

Optional for direct SMS and calling:
- SIM800L GSM module
- SIM card with calling plan
- Separate 4V power supply for SIM800L

Full parts list with prices is in hardware/BOM.txt.

---

## How to Wire It

See hardware/WIRING.txt for the full diagram.

Quick summary:
- ADXL345 goes to I2C pins (GPIO21 = SDA, GPIO22 = SCL)
- GPS goes to UART2 (GPIO16 = RX, GPIO17 = TX)
- Buzzer positive wire goes to GPIO26
- LED goes to GPIO27 through a 330 ohm resistor
- Touch button wire goes to GPIO4

---

## How to Upload the Code

1. Download and install Arduino IDE from arduino.cc
2. Add ESP32 board support:
   - Open File > Preferences
   - Add this URL to "Additional Board Manager URLs":
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   - Open Tools > Board > Boards Manager, search "ESP32", install it
3. Install libraries (Sketch > Manage Libraries):
   - Adafruit ADXL345
   - Adafruit Unified Sensor
   - TinyGPSPlus
4. Open firmware/accident_guard.ino
5. Select your board: Tools > Board > ESP32 Dev Module
6. Select your port: Tools > Port
7. Click Upload

---

## How to Use It

**Normal operation:**
Power it on. The LED blinks twice to confirm startup. That is it. The device runs on its own.

**Cancel a false alarm:**
Tap the touch button 3 times fast (within 3 seconds). The buzzer and LED stop.

**Enable anti-theft (parked mode):**
Tap the touch button 5 times quickly. The buzzer beeps twice to confirm. Now any movement triggers the alarm. Tap 3 times to cancel.

**Emergency happens automatically:**
You do not need to do anything. If an impact is detected and you do not cancel it within 60 seconds, the device handles everything.

---

## Adjusting Sensitivity

Open firmware/config.h and change these values:

- ACCIDENT_G_THRESHOLD: Higher number means less sensitive (default 2.5G). If it triggers on road bumps, increase to 3.0 or 3.5.
- EMERGENCY_DELAY_SEC: How many seconds before it calls for help (default 60).
- TOUCH_SENSITIVITY: Lower number means touch is more sensitive (default 40).

---

## BLE Data Format

The device broadcasts JSON every second:

  {"state":"IDLE","lat":23.831457,"lng":91.286778,"gps_valid":true,"antitheft":false,"uptime":120}

State can be: IDLE, ACCIDENT_WAITING, EMERGENCY, ANTITHEFT

---

## Demo File

Open demo/index.html in any browser. It simulates the full device behavior so you can see how it works before building the hardware. Click "Simulate Accident" to see the countdown, use the touch pad to cancel, and toggle anti-theft mode.

---

## Folder Structure

```
accident-guard/
    firmware/
        accident_guard.ino   (main code to upload to ESP32)
        config.h             (all settings in one place)
        sms_caller.h         (optional SIM800L support)
        LIBRARIES.txt        (what to install in Arduino IDE)
    hardware/
        WIRING.txt           (how to connect everything)
        BOM.txt              (parts list with prices)
    demo/
        index.html           (browser demo of the device)
    README.md                (this file)
```

---

## Limitations

- The device needs a BLE-connected phone app to send SMS and make calls. Without a phone nearby, BLE data is broadcast but not acted on.
- If you add a SIM800L module, it can call and text directly without a phone. See firmware/sms_caller.h.
- GPS needs some time outdoors to get a fix (30 seconds to 2 minutes on first use).
- The ADXL345 must be mounted flat and secure. A loose sensor gives false readings.

---

## License

MIT License. Free to use, modify, and build.
 ## Thank you

Thank you for exploring the AccidentGuard dashboard. This project was developed with a single goal in mind: to save lives by bridging the critical gap between when a crash occurs and when help arrives. Your interest in NirmalyaLenka/AccidentGuard helps push open-source safety technology forward. Stay safe on the roads!"
happy jurney
## +++++++++++++++++++++++++++++++++++++++++++++++++
