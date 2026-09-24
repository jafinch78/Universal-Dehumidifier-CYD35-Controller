# Universal Dehumidifier Controller V1 — Design Specification

Date: 2026-09-23
Target UI/controller: CYD ESP32-3248S035R / E32N35T, 3.5-inch 320x480 resistive-touch, non-PSRAM ESP32
Firmware environment: Arduino IDE / ESP32 Arduino core / TFT_eSPI

## Revision note — V1 board lock

The initial generic CYD discussion suggested GPIO27 as an expansion candidate. That is incorrect for the proven E32N35T board because GPIO27 drives the TFT backlight. V1 therefore locks to the user's known-working ESP32-3248S035R / E32N35T profile and uses GPIO25/GPIO32 for the external I2C bus.

## 1. Purpose

Create a reusable replacement control system for conventional portable compressor-based dehumidifiers whose original electronic controller has failed. The design must avoid dependence on any one appliance brand, keep mains switching physically separate from the CYD, and provide a touchscreen UI, humidity regulation, compressor protection, condensate interlocks, fan control, optional defrost sensing, diagnostics, and configurable appliance profiles.

The V1 target is conventional fixed-speed compressor dehumidifiers. Variable-speed/inverter compressor appliances are explicitly out of scope for V1.

## 2. Design principles

1. The CYD is the supervisory controller and user interface, not a mains power board.
2. All appliance I/O is expanded over a low-voltage I2C bus so CYD pin scarcity and board-pinout variation are isolated from the dehumidifier logic.
3. Compressor switching uses an external appliance-rated relay/contactor or approved power module sized from the compressor nameplate and locked-rotor/start characteristics.
4. Existing compressor overload, start capacitor/PTC/relay, fuse, grounding, and other manufacturer safety devices remain in circuit unless a replacement is explicitly engineered for that appliance.
5. Every reset, sensor failure, communication failure, or invalid configuration must fail toward compressor OFF.
6. A minimum compressor off-time is mandatory and cannot be bypassed by the normal UI.
7. CYD display/touch pin definitions are isolated in a board profile so future 3.5-inch CYD variants can be supported without altering control logic. V1 is compiled and validated first on the user's proven ESP32-3248S035R / E32N35T resistive-touch board.

## 3. System architecture

```text
                 LOW-VOLTAGE CONTROLLER

     +------------------------------------------+
     | CYD 3.5 / ESP32-3248S035                |
     |                                          |
     | UI / touch / NVS / state machine         |
     | logging / timers / fault management      |
     +-------------------+----------------------+
                         |
                   3.3 V I2C BUS
                  SDA/SCL profile
                         |
           +-------------+--------------+
           |                            |
     +-----v------+                +----v------+
     | MCP23017   |                | ADS1115   |
     | 16 GPIO    |                | 4 ch ADC  |
     +---+--------+                +----+------+
         |                              |
   DI + relay commands            thermistors/
         |                        analog sensors
         |
     +---v-------------------+
     | relay-driver stage    |
     | pulldown = OFF        |
     +---+-------------------+
         |
       12 VDC
         |
  +------v---------------------------------------------------+
  | external appliance-rated relay/contactors/power modules |
  +------+------------+------------+-----------+-------------+
         |            |            |           |
     compressor      fan(s)     pump       aux/defrost

     ROOM RH/T SENSOR: SHT31/SHT41 on I2C
```

## 4. CYD board-profile abstraction

The firmware shall not scatter physical CYD pins through the application. A single `board_pins.h` / board-profile module defines:

- TFT driver and pins
- touch controller and pins
- backlight pin and polarity
- I2C SDA/SCL pins
- optional SD-card pins
- optional speaker/buzzer pin

V1 uses the already-proven ESP32-3248S035R / E32N35T resistive-touch board profile from the earlier CYD work. Its established assignments are:

- TFT/touch SPI: SCLK GPIO14, MOSI GPIO13, MISO GPIO12
- resistive touch CS: GPIO33
- TFT backlight: GPIO27
- microSD: CS GPIO5, CLK GPIO18, MISO GPIO19, MOSI GPIO23
- external appliance-control I2C bus for this project: SDA GPIO25, SCL GPIO32
- touch calibration baseline: `{295, 3524, 310, 3487, 7}`

GPIO25/GPIO32 were previously used successfully as the external TWAI/CAN pair on this same physical CYD. This dehumidifier controller does not require TWAI, so those two pins are reassigned to the expansion I2C bus. GPIO27 is reserved exclusively for the TFT backlight and must not be used for I2C.

Initial compiled profile:

- `CYD35_E32N35T_R`

Future Dorhea or capacitive CYD profiles may be added later, but they are not part of V1 acceptance and must not change appliance-control logic.

## 5. Low-voltage power architecture

Recommended topology:

```text
120/230 VAC appliance input
       |
       +---- existing appliance mains loads / safety devices
       |
       +---- isolated universal AC/DC supply ---> 12 VDC
                                                |
                                                +--> relay/contactor coils
                                                |
                                                +--> fused 5 V buck ---> CYD
                                                |
                                                +--> 3.3 V logic as required
```

Requirements:

- AC/DC supply must be isolated and appropriately rated for the appliance input voltage.
- 12 V branch shall be fused.
- CYD 5 V supply shall be separately fused or current-limited.
- Logic ground and 12 V control ground may be common on the low-voltage side.
- Mains wiring and low-voltage wiring shall remain physically segregated.
- Protective earth remains bonded to the appliance chassis where applicable.

## 6. I2C devices

### 6.1 MCP23017

Purpose: digital inputs and relay-driver commands.

Nominal address: `0x20`, with address jumpers/pads retained for future expansion.

Power-up behavior: MCP23017 pins default to inputs. Every relay-driver input must additionally have a hardware pulldown so high impedance cannot energize a load.

### 6.2 ADS1115

Purpose: evaporator/condenser thermistors and optional analog inputs.

Nominal address: `0x48`.

Channels:

- A0: evaporator thermistor
- A1: condenser thermistor
- A2: optional OEM analog humidity sensor or spare
- A3: spare/service analog input

### 6.3 SHT31/SHT41

Purpose: primary room humidity and ambient-temperature measurement.

The external sensor should be mounted in the return-air path but shielded from direct evaporator discharge, condensate, and heat generated by the controller/power supply.

## 7. Universal digital I/O map

### 7.1 MCP23017 outputs

| Channel | Function | Default | Notes |
|---|---|---:|---|
| GPA0 | COMPRESSOR_CMD | OFF | drives external compressor contactor/relay coil stage |
| GPA1 | FAN_LOW_CMD | OFF | single-speed fan uses this channel |
| GPA2 | FAN_HIGH_CMD | OFF | optional second speed/winding/relay |
| GPA3 | PUMP_CMD | OFF | optional condensate pump |
| GPA4 | DEFROST_AUX_CMD | OFF | optional heater/valve/auxiliary relay |
| GPA5 | ALARM_AUX_CMD | OFF | buzzer/lamp/auxiliary function |
| GPA6 | spare output | OFF | reserved |
| GPA7 | spare output | OFF | reserved |

### 7.2 MCP23017 inputs

| Channel | Function | Configurable polarity |
|---|---|---|
| GPB0 | BUCKET_FULL | YES |
| GPB1 | SECONDARY_FLOAT / DRAIN_HIGH | YES |
| GPB2 | PRESSURE_OR_THERMAL_INTERLOCK | YES |
| GPB3 | DOOR/COVER/AUX_INTERLOCK | YES |
| GPB4 | FAN_TACH_OR_AUX_INPUT | YES |
| GPB5 | appliance-profile spare | YES |
| GPB6 | appliance-profile spare | YES |
| GPB7 | appliance-profile spare | YES |

Inputs shall use external pull-up/pull-down components and protection appropriate to the connected switch/sensor. OEM circuits must not be connected until their voltage and topology are identified.

## 8. Output driver board

The MCP23017 shall not directly energize relay coils.

Recommended driver:

```text
MCP23017 GPIO
      |
   series R
      |
 ULN2803A input
      |
 ULN2803A sink output ---- 12 V relay/contactor coil ---- +12 V
      |
     GND
```

Requirements:

- hardware pulldown on each ULN input
- flyback suppression for DC coils
- screw terminal or locking connector for each coil circuit
- indicator LED optional but must not alter safe default state
- compressor relay output shall command a 12 VDC coil contactor/relay; it shall not carry compressor mains current on the controller PCB

## 9. Appliance power stage boundary

The universal board provides low-voltage coil commands only.

Typical compressor path:

```text
LINE
 |
 fuse / appliance safety chain
 |
 external compressor-rated contactor
 |
 OEM overload / start components as applicable
 |
 compressor
 |
 NEUTRAL / second line
```

Relay/contactor selection is appliance-specific and based on voltage, running current, motor utilization category, and locked-rotor/start current. Generic resistive-load relay ratings are not sufficient qualification for the compressor output.

## 10. Thermistor interface

Each NTC channel uses a precision divider referenced to 3.3 V and is read by the ADS1115.

Supported profiles initially:

- 10 kOhm NTC, Beta 3950
- 10 kOhm NTC, user-entered beta
- 5 kOhm NTC, user-entered beta
- 15 kOhm NTC, user-entered beta
- custom Steinhart-Hart A/B/C coefficients

Firmware shall detect open/short values outside the configured plausible resistance range and raise a sensor fault.

## 11. Control states

```text
BOOT
  |
  v
SELF_TEST
  |-- invalid config ----------> FAULT
  |-- critical sensor failure -> FAULT
  |-- bucket full -------------> BUCKET_FULL
  v
STARTUP_LOCKOUT
  |
  v
IDLE <------------------------------+
  |                                  |
  | RH demand                        |
  v                                  |
FAN_PRE_RUN                          |
  |                                  |
  v                                  |
DEHUMIDIFY                           |
  |                                  |
  +-- humidity satisfied ----------> FAN_POST_RUN --+
  +-- bucket full -----------------> BUCKET_FULL     |
  +-- freeze condition ------------> DEFROST -------+
  +-- critical fault --------------> FAULT           |
                                                     |
                                                     +--> IDLE
```

## 12. Default operating parameters

Initial defaults are deliberately conservative and user-configurable:

- RH setpoint: 50%
- RH hysteresis: 2 percentage points
- compressor minimum OFF: 180 s
- compressor minimum ON: 60 s
- startup compressor lockout: 180 s
- fan pre-run: 5 s
- fan post-run: 60 s
- evaporator freeze threshold: 2 C
- evaporator recovery threshold: 8 C
- defrost timeout: 15 min
- pump maximum continuous run: 180 s
- sensor-failure debounce: 5 s unless a condition is immediately unsafe

The defaults are starting values for bench testing, not validated universal appliance settings.

## 13. Humidity-control algorithm

For normal mode:

- demand begins when `RH >= setpoint + hysteresis`
- demand ends when `RH <= setpoint - hysteresis`
- compressor minimum-on and minimum-off timers are always observed except that critical safety shutdown may terminate the ON period immediately
- fan mode may be CONTINUOUS or WITH_COMPRESSOR

Optional future modes such as continuous dry, laundry, and timed operation are not required for V1.

## 14. Defrost strategy

V1 supports fan-only evaporator defrost for typical portable dehumidifiers.

Trigger candidates:

1. evaporator temperature below the configured freeze threshold for a configurable persistence time
2. optional rate-of-temperature criterion in later firmware

Defrost behavior:

1. compressor OFF immediately
2. fan ON
3. remain in defrost until evaporator reaches recovery temperature or timeout occurs
4. compressor remains subject to the minimum OFF timer before restart

If a specific appliance contains an active defrost valve or heater, `DEFROST_AUX_CMD` may be enabled by its appliance profile only after that hardware is independently characterized.

## 15. Safety and fault behavior

### 15.1 Critical shutdown events

The compressor command shall be forced OFF for:

- bucket full
- secondary condensate high
- pressure/thermal interlock open, where configured
- I2C communications loss to required I/O
- room RH sensor failure
- evaporator sensor failure when freeze protection depends on it
- invalid appliance profile/configuration
- watchdog reset/reboot
- detected output-state inconsistency when feedback is available

### 15.2 Reset behavior

After every cold boot, watchdog reboot, brownout, or firmware restart:

- all outputs initialize OFF
- expander direction/output registers are explicitly initialized before operation
- compressor lockout timer starts from zero
- compressor cannot start until the startup lockout expires and all required inputs are valid

### 15.3 Noncritical faults

A condenser thermistor or optional auxiliary sensor may be designated noncritical. Its failure produces a UI warning but does not necessarily prevent operation.

## 16. Appliance profile

All brand/model-specific behavior is data-driven.

Proposed structure:

```cpp
struct ApplianceProfile {
  char name[32];
  bool hasFanHigh;
  bool hasPump;
  bool hasEvapSensor;
  bool hasCondSensor;
  bool hasSecondaryFloat;
  bool hasPressureInterlock;

  bool bucketActiveHigh;
  bool secondaryFloatActiveHigh;
  bool interlockActiveHigh;

  uint16_t compressorMinOffSec;
  uint16_t compressorMinOnSec;
  uint16_t fanPreRunSec;
  uint16_t fanPostRunSec;

  float freezeTempC;
  float freezeRecoverC;
};
```

The actual implementation may use a versioned packed structure plus CRC rather than relying directly on this source-level layout.

## 17. Persistent configuration

ESP32 Preferences/NVS shall store:

- profile schema/version
- operating setpoint
- hysteresis
- fan mode
- temperature unit
- input polarities
- installed hardware options
- compressor timers
- defrost thresholds
- thermistor coefficients
- humidity/temperature offsets
- accumulated compressor run hours
- compressor start count
- selected CYD UI preferences

Configuration shall include a version and CRC/check mechanism. Invalid data causes defaults to load with the compressor disabled until configuration is accepted.

## 18. User interface

### 18.1 Main page

Displays:

- room RH, large numeric value
- room temperature
- target RH
- compressor state
- fan state
- evaporator temperature if fitted
- bucket state
- current operating state
- fault/warning banner

Controls:

- Power
- target RH - / +
- Mode/Fan
- Setup

### 18.2 Setup page

User-level settings:

- RH setpoint
- fan mode
- display units
- screen brightness

### 18.3 Service page

Protected by deliberate long-press/service entry sequence rather than normal accidental navigation.

Functions:

- live raw digital inputs
- live ADS1115 readings
- calculated thermistor resistance/temperature
- live I2C-device status
- output-command status
- compressor minimum-off countdown
- compressor run time and start count
- fault history
- appliance profile editor
- sensor calibration
- individual fan/pump/aux output tests

The compressor may have a service-test request, but firmware still enforces critical interlocks and compressor minimum-off time.

## 19. Fault log

Minimum retained fault record:

```text
sequence number
timestamp/runtime counter
fault code
operating state
room RH
room temperature
evaporator temperature
condenser temperature
digital input bitmap
output command bitmap
```

V1 stores a circular fault history in NVS. SD-card continuous logging is optional and not required for first release.

## 20. Firmware module structure

```text
UniversalDehumidifier/
  UniversalDehumidifier.ino
  board_pins.h
  board_pins.cpp
  config.h
  config.cpp
  io_expander.h
  io_expander.cpp
  sensors.h
  sensors.cpp
  thermistor.h
  thermistor.cpp
  controller.h
  controller.cpp
  compressor_guard.h
  compressor_guard.cpp
  faults.h
  faults.cpp
  ui.h
  ui.cpp
  storage.h
  storage.cpp
```

Responsibilities:

- `board_pins`: CYD hardware variant only
- `io_expander`: MCP23017/ADS1115 transactions and safe initialization
- `sensors`: SHT3x and sensor plausibility
- `thermistor`: ADC-to-temperature conversion
- `compressor_guard`: anti-short-cycle and minimum-run invariants
- `controller`: state machine and demand logic
- `faults`: fault creation, latching policy, history
- `storage`: Preferences/NVS versioning and recovery
- `ui`: display/touch only; it requests actions but does not directly control relays

This separation is intentional: the UI cannot bypass compressor safety logic.

## 21. Software invariants

The following must always be true:

1. No code path other than `compressor_guard` can authorize compressor ON.
2. Critical fault => compressor command OFF.
3. Bucket full => compressor command OFF.
4. Compressor cannot restart before minimum OFF time expires.
5. Output hardware initializes to OFF before I2C devices are configured as outputs.
6. UI service commands pass through the same safety layer as automatic commands.
7. Loss of a required I2C device invalidates compressor authorization.
8. A configuration change affecting safety restarts the compressor lockout timer.

## 22. Startup sequence

```text
1. CPU boot
2. keep all appliance outputs de-energized
3. initialize display/touch
4. load and validate NVS
5. initialize I2C
6. discover MCP23017, ADS1115, SHT3x
7. configure relay pins to known OFF values before making them outputs
8. configure digital inputs
9. sample sensors and verify plausibility
10. start startup-lockout timer
11. enter IDLE/BUCKET_FULL/FAULT as appropriate
```

## 23. Verification plan

Bench validation must be performed with low-voltage lamps/LED loads before any compressor is connected.

### Stage A — firmware/UI bench

- boot/reset behavior
- touchscreen pages
- NVS save/restore
- bad-NVS recovery
- I2C device unplug/replug faults
- sensor open/short faults
- input polarity settings

### Stage B — simulated appliance

Use switches for floats/interlocks and indicator lamps for outputs.

Verify:

- humidity demand transitions
- compressor lockout countdown
- forced safety shutdown
- fan pre/post run
- defrost state
- power-cycle restart lockout
- service mode cannot defeat lockout

### Stage C — relay/contactor bench

Operate only low-voltage coils first and verify:

- no relay chatter during ESP32 reset
- no inadvertent energization during programming
- outputs OFF if I2C cable is disconnected
- suppression prevents ESP32 resets/noise faults

### Stage D — appliance integration

For each appliance, document before connection:

- supply voltage
- compressor nameplate current and LRA if stated
- compressor overload/start topology
- fan motor topology/current
- float-switch type/polarity
- thermistor resistance at two or more known temperatures
- any pump or pressure interlocks
- selected relay/contactor ratings

Only then create/select the corresponding appliance profile.

## 24. V1 acceptance criteria

V1 is acceptable when:

1. CYD boots to a usable touch UI and reports all required I2C devices.
2. RH setpoint can be changed and persists across reboot.
3. Simulated RH demand starts fan and compressor in the defined sequence.
4. Compressor cannot restart before the configured minimum OFF time.
5. Bucket-full transition removes compressor command immediately.
6. Required sensor/I2C failure removes compressor command and displays a fault.
7. Defrost simulation removes compressor command, keeps the required fan state, and recovers correctly.
8. No output energizes during flashing, reset, boot, or I2C initialization.
9. Appliance profiles change input polarity and installed-option behavior without recompiling control logic.
10. All tests above pass using low-voltage simulated loads before appliance mains integration.

## 25. Explicitly deferred from V1

- inverter/variable-speed compressor control
- Wi-Fi cloud control
- MQTT/Home Assistant
- OTA updates
- historical graphs
- SD continuous data logging
- automatic OEM sensor identification
- universal mains PCB carrying compressor current

These can be added after the core controller is proven.

## 26. Source notes for CYD pinout decision

Public ESP32-3248S035/CYD references used to establish the board-profile requirement and exposed-pin strategy:

- ardnew, `ESP32-3248S035` board support package: https://github.com/ardnew/ESP32-3248S035
- jhsrennie, `ESP32/CYD/CYD Pinout.md`: https://github.com/jhsrennie/ESP32/blob/main/CYD/CYD%20Pinout.md
- chacuavip10, `CYD-3.5inch_ESP32-3248S035`: https://github.com/chacuavip10/CYD-3.5inch_ESP32-3248S035

The implementation must use the verified `CYD35_E32N35T_R` physical-board profile for V1 rather than assuming every 3.5-inch CYD uses identical display/touch/backlight pins. The project's external I2C bus is GPIO25/GPIO32 on this profile; GPIO27 remains the display backlight.
