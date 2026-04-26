/*
  config.h - AccidentGuard Configuration
  Edit this file to customize your device settings.
*/

#ifndef CONFIG_H
#define CONFIG_H

// ---- Emergency Contacts ----
// These are used by the companion app via BLE
// Add phone numbers in international format
#define EMERGENCY_CONTACT_1   "+91XXXXXXXXXX"
#define EMERGENCY_CONTACT_2   "+91XXXXXXXXXX"

// ---- Device Name ----
#define DEVICE_NAME           "AccidentGuard"
#define DEVICE_VERSION        "1.0.0"

// ---- Sensitivity Settings ----
// Higher value = less sensitive to accidents (reduce false triggers)
// Lower value = more sensitive (may trigger on bumps)
#define ACCIDENT_G_THRESHOLD  2.5

// Anti-theft sensitivity (G-force to trigger when parked)
#define ANTITHEFT_G_THRESHOLD 0.8

// Capacitive touch threshold (lower = more sensitive)
#define TOUCH_SENSITIVITY     40

// ---- Timing ----
// Time in seconds before emergency services are called
// after an accident with no cancel input
#define EMERGENCY_DELAY_SEC   60

// Buzzer duration after emergency trigger (seconds)
#define BUZZER_DURATION_SEC   60

// ---- GPS Baud Rate ----
#define GPS_BAUD              9600

// ---- Pin Map (change only if you rewired) ----
// ADXL345 uses I2C: SDA=21, SCL=22 (default ESP32)
// GPS uses UART2: RX=16, TX=17
// Buzzer: GPIO 26
// LED Flashlight: GPIO 27
// Touch: GPIO 4 (T0)

#endif // CONFIG_H
