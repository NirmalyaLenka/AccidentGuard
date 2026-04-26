/*
  sms_caller.h - Optional SIM800L SMS and Call Support
  Include this only if you have a SIM800L module connected.
  
  Wiring for SIM800L:
    SIM800L TX  -> ESP32 GPIO 14
    SIM800L RX  -> ESP32 GPIO 12
    SIM800L GND -> ESP32 GND
    SIM800L VCC -> 4.0V (use a separate LiPo or buck converter)
    
  NOTE: SIM800L needs 2A peak current. Do not power from ESP32 3.3V pin.
*/

#ifndef SMS_CALLER_H
#define SMS_CALLER_H

#include <HardwareSerial.h>

#define SIM_RX 14
#define SIM_TX 12
#define SIM_BAUD 9600

HardwareSerial SIMSerial(1);

void simSetup() {
  SIMSerial.begin(SIM_BAUD, SERIAL_8N1, SIM_RX, SIM_TX);
  delay(3000);
  SIMSerial.println("AT");
  delay(500);
  SIMSerial.println("AT+CMGF=1"); // SMS text mode
  delay(500);
}

void sendSMS(const char* number, const char* message) {
  SIMSerial.print("AT+CMGS=\"");
  SIMSerial.print(number);
  SIMSerial.println("\"");
  delay(500);
  SIMSerial.print(message);
  delay(100);
  SIMSerial.write(26); // CTRL+Z to send
  delay(3000);
  Serial.println("SMS sent to ");
  Serial.println(number);
}

void makeCall(const char* number) {
  SIMSerial.print("ATD");
  SIMSerial.print(number);
  SIMSerial.println(";");
  delay(20000); // Let it ring for 20 seconds
  SIMSerial.println("ATH"); // Hang up
}

void sendEmergencySMS(float lat, float lng, const char* contact1, const char* contact2) {
  char message[200];
  snprintf(message, sizeof(message),
    "EMERGENCY: Accident detected. Location: https://maps.google.com/?q=%.6f,%.6f - AccidentGuard",
    lat, lng
  );
  sendSMS(contact1, message);
  delay(1000);
  sendSMS(contact2, message);
  delay(1000);
  makeCall(contact1);
}

#endif // SMS_CALLER_H
