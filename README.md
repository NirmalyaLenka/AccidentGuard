# AccidentGuard — ESP32 Impact-Alert Prototype

AccidentGuard is an ESP32 prototype that reads an ADXL345 accelerometer and NEO-6M GPS module, signals a local alert with a buzzer/LED, and sends a JSON status notification to a connected BLE client. It is suitable for experimentation with sensors and state machines, **not as a dependable emergency-response or life-safety device**.

## What the current firmware actually does

- Reads acceleration from the ADXL345 and calculates the magnitude of the acceleration vector.
- Enters an alert/cancellation countdown when that magnitude exceeds a hard-coded threshold.
- Pulses the buzzer and LED during the cancellation window; three touch taps can cancel the alert.
- Reads NMEA data from the GPS module over UART2 and includes coordinates plus a `gps_valid` flag in the status payload.
- Provides a BLE GATT characteristic that notifies a connected client about once per second.

### Important behavior gaps

- The emergency routine is a placeholder: it prints the GPS position and a message to Serial. The main firmware does **not** send SMS or place phone calls.
- `sms_caller.h` is not included or invoked by `accident_guard.ino`; its presence does not mean SIM800L support is integrated.
- The repository does not contain a companion mobile app. `index.html` is a browser demo, not a verified emergency-contact application.
- `config (1).h` is not included by the sketch. Thresholds and timings are currently hard-coded in `accident_guard.ino`, so editing the header alone has no effect.
- The code describes a five-tap anti-theft toggle, but the tap counter resets after the third tap before the five-tap check can be reached. Treat the toggle as nonfunctional until the source is corrected and tested.
- The impact check uses the magnitude of acceleration, including gravity, with a single threshold and no validated crash-classification/filtering logic. False alerts and missed events are possible.

Do not use this prototype as your only means of requesting help, as a substitute for a certified crash detector, or as an anti-theft security product.

## Hardware and pinout

See [`WIRING.txt`](WIRING.txt), [`BOM.txt`](BOM.txt), and the included [`ADXL345.pdf`](ADXL345.pdf) reference.

| Device | ESP32 connection | Notes |
|---|---|---|
| ADXL345 SDA | GPIO 21 | I²C |
| ADXL345 SCL | GPIO 22 | I²C |
| GPS TX | GPIO 16 / UART2 RX | ESP32 receives GPS data |
| GPS RX | GPIO 17 / UART2 TX | ESP32 sends to GPS |
| Active buzzer control | GPIO 26 | Use a transistor driver if current exceeds GPIO capability |
| LED | GPIO 27 | Use the specified current-limiting resistor/driver |
| Capacitive touch pad | GPIO 4 / T0 | Touch input |
| Common ground | As required by the wiring diagram | Verify supply and module requirements |

The main sketch initializes the ADXL345 over I²C and the GPS UART at 9600 baud. GPS location is only marked valid after the library reports a valid fix.

### First-run safety checklist

1. Build the circuit with the buzzer and high-power LED disconnected or current-limited initially.
2. Verify ADXL345 supply voltage and I²C wiring; confirm the sensor is detected in Serial Monitor.
3. Place the GPS antenna where it has a clear view of the sky and check that the firmware eventually reports a valid fix. Until then, coordinates are not suitable for dispatch.
4. Test touch cancellation and local alerts on the bench without relying on the emergency routine to contact anyone.
5. Keep any battery, charging circuit, and high-current modules within their ratings and protected from shorts.

The optional SIM800L helper notes a dedicated supply near 4 V with up to 2 A peak capability. Do not power a SIM800L from the ESP32 3.3 V pin; any future integration needs a verified supply, UART wiring, network coverage, and real end-to-end tests.

## Software prerequisites

- Arduino IDE 2.x.
- **ESP32 by Espressif Systems** board package.
- Libraries listed in [`LIBRARIES.txt`](LIBRARIES.txt): Adafruit ADXL345, Adafruit Unified Sensor, and TinyGPSPlus. BLE headers are supplied by the selected ESP32 Arduino core.
- USB data cable and correct serial port.

## Build and upload

1. Install the ESP32 board package and the libraries above.
2. Open the root-level `accident_guard.ino` in Arduino IDE.
3. Select **ESP32 Dev Module** (or your exact board), choose its serial port, and upload.
4. Open Serial Monitor at **115200 baud**.
5. Confirm ADXL345 initialization succeeds before attempting any alert test. If it fails, the sketch remains in an LED-blink loop.
6. Test local alert/cancel behavior and GPS validity separately. Do not treat a printed emergency message or BLE notification as proof that a call or SMS was sent.

No firmware compilation, GPS field test, BLE client integration, or physical alert test was performed during this documentation update.

## Configuration

Current operational constants live directly in `accident_guard.ino`:

| Constant | Current value | Purpose |
|---|---:|---|
| `ACCIDENT_THRESHOLD` | 2.5 | Acceleration-magnitude threshold in G units after conversion |
| `ANTITHEFT_THRESHOLD` | 0.8 | Movement threshold in the anti-theft branch |
| `TOUCH_THRESHOLD` | 40 | Touch reading below which is treated as pressed |
| `TOUCH_CANCEL_TAPS` | 3 | Tap count used for cancellation |
| `CANCEL_TAP_WINDOW` | 3000 ms | Tap grouping window |
| `ALERT_WAIT_TIME` | 60000 ms | Delay before entering emergency state |

The similarly named values in `config (1).h` are not wired into the current sketch. Do not assume changing that header changes runtime behavior.

## BLE payload

When a BLE client is connected, the firmware sends a notification approximately once per second. The JSON payload contains:

- `state`: `IDLE`, `ACCIDENT_WAITING`, `EMERGENCY`, or `ANTITHEFT`
- `lat` and `lng`: last GPS coordinates (may remain zero before a fix)
- `gps_valid`: whether the GPS library reports a valid location
- `antitheft`: internal anti-theft mode flag
- `uptime`: elapsed seconds since boot

The data is delivered through a connected GATT characteristic, not through a verified emergency notification service. A compatible client must connect and subscribe to notifications.

## Repository map

| Path | Purpose |
|---|---|
| `accident_guard.ino` | Main ESP32 firmware |
| `config (1).h` | Configuration draft; not included by the current sketch |
| `sms_caller.h` | Optional SIM800L helper; not integrated into main firmware |
| `LIBRARIES.txt` | Arduino library and board settings |
| `WIRING.txt` | Hardware connections and power notes |
| `BOM.txt` | Parts list and approximate cost estimates |
| `ADXL345.pdf` | Accelerometer reference document |
| `index.html` | Browser simulation/demo only |
| `README.md` | Project overview and limitations |

All files currently sit at repository root; the older folder-tree description does not match the checked-in layout.

## Troubleshooting

- **ADXL345 not detected:** verify I²C address, SDA/SCL, power, ground, and module mode pins.
- **GPS stays invalid:** check UART TX/RX crossover, 9600 baud, antenna placement, and outdoor sky visibility.
- **Coordinates are zero or stale:** check `gps_valid`; never treat coordinates as current unless a valid fix is reported.
- **BLE payload is absent:** a client must connect to the `AccidentGuard` GATT service and subscribe to the notify characteristic.
- **No SMS/call is received:** this is expected with the current main sketch; the emergency function is a Serial placeholder and the SIM800L helper is not integrated.
- **False accident triggers:** the single acceleration-magnitude threshold is not a validated crash detector. Do not lower/raise it and assume the device is safe without systematic tests.

## Verification and license status

The README was checked against the current firmware, wiring notes, BOM, library list, configuration header, and SIM800L helper. Build/runtime behavior remains unverified. No root-level `LICENSE` file was present in the inspected tree, so the previous MIT claim is not backed by a checked-in license file.

## Maintenance & audit

- **Audit date:** 11 November 2026
- **Scope:** documentation-only review; firmware, pins, and hardware behavior were not changed.
- **Verification:** compilation, GPS, BLE client behavior, and emergency signaling remain unverified.
