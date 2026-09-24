# Universal Dehumidifier Controller V1 — Implementation Plan

> **Status: draft implementation plan for an unreviewed, unassembled, untested thought experiment.**
>
> This plan is a development starting point. It is not evidence that the controller is safe, correct, assembled, or suitable for connection to an appliance or mains voltage.

## Goal

Build an Arduino-IDE-uploadable controller for the ESP32-3248S035R / E32N35T CYD that supervises a conventional fixed-speed compressor dehumidifier through a separate low-voltage I/O board and appliance-rated external power switching hardware.

## Baseline architecture

- CYD: ESP32-3248S035R / E32N35T, resistive touch
- TFT/touch SPI: GPIO14 SCLK, GPIO13 MOSI, GPIO12 MISO
- touch CS: GPIO33
- TFT backlight: GPIO27
- external appliance-control I2C: GPIO25 SDA / GPIO32 SCL
- MCP23017: nominal `0x20`
- ADS1115: nominal `0x48`
- room RH/T: SHT31 or SHT4x
- relay/contactor coils: low-voltage driver stage such as ULN2803A
- compressor/fan/pump mains current: external appliance-rated hardware only

## Safety invariants

1. All outputs initialize OFF.
2. Only the compressor-guard layer may authorize compressor ON.
3. Bucket full, secondary high-water, required sensor loss, required I2C loss, invalid configuration, watchdog reset, or critical interlock fault forces compressor OFF.
4. Compressor minimum-OFF protection cannot be bypassed from the normal or service UI.
5. Service commands pass through the same interlocks as automatic operation.
6. GPIO27 remains dedicated to the TFT backlight; it is not used for the expansion bus.
7. Existing OEM compressor overload/start components, grounding, fusing, and other safety devices remain unless specifically re-engineered and validated for one appliance.

## Implementation sequence

### Task 1 — CYD board profile and build baseline

Create the Arduino sketch shell, board profile, TFT_eSPI setup notes, display/backlight initialization, and resistive-touch calibration support. Verify the CYD boots, paints the screen, and reads touch without any appliance I/O attached.

### Task 2 — Shared types and configuration model

Define operating states, input/output IDs, fault codes, appliance profile fields, user configuration, timer defaults, and safe validation rules. Configuration changes that affect safety restart compressor lockout.

### Task 3 — MCP23017 / ADS1115 low-voltage I/O layer

Initialize the I2C bus on GPIO25/GPIO32. Initialize relay output registers to OFF before changing direction bits to outputs. Add digital-input sampling, output commands, analog acquisition, device-health status, and a single `forceAllOutputsOff()` path.

### Task 4 — Room humidity/temperature acquisition

Add SHT31/SHT4x support with plausibility checks, stale-data detection, calibration offsets, and required-device fault behavior.

### Task 5 — Thermistor conversion

Implement NTC divider conversion for common 5 kΩ, 10 kΩ, and 15 kΩ sensors plus custom beta or Steinhart-Hart coefficients. Detect open/short implausible values before they reach the controller.

### Task 6 — Compressor guard

Implement startup lockout, minimum-OFF timer, minimum-ON timer, forced-OFF handling, and unsigned elapsed-time arithmetic that remains correct across ESP32 `millis()` wraparound.

Default starting values:

- startup lockout: 180 s
- minimum OFF: 180 s
- minimum ON: 60 s
- fan pre-run: 5 s
- fan post-run: 60 s

### Task 7 — Fault manager

Centralize critical/noncritical fault classification, current-fault state, latching/recovery policy, safe-stop requests, and circular fault history. A critical FAULT state requests compressor, fan, pump, and defrost auxiliary OFF; an optional low-voltage alarm output may remain active.

### Task 8 — Persistent storage

Use ESP32 Preferences/NVS with schema versioning and integrity checking. Corrupt or incompatible stored configuration loads safe defaults and leaves compressor operation disabled until configuration is explicitly accepted.

### Task 9 — Dehumidifier state machine

Implement:

```text
BOOT -> SELF_TEST -> STARTUP_LOCKOUT -> IDLE
                                  |
                    humidity demand
                                  v
                           FAN_PRE_RUN
                                  |
                                  v
                            DEHUMIDIFY
                     /       |        \
               satisfied   freeze    fault/water
                  |          |          |
            FAN_POST_RUN   DEFROST   safe stop
                  |          |
                  +-----> IDLE
```

Normal humidity demand begins at `setpoint + hysteresis` and ends at `setpoint - hysteresis`.

### Task 10 — Touch UI and runtime integration

Add main, setup, service, and fault/status screens. The UI may request actions but never directly write relay outputs. Use nonblocking scheduling rather than long `delay()` calls for controller, sensors, touch, repaint, diagnostics, and I2C-health polling.

### Task 11 — Simulated-appliance validation and documentation

Before any mains integration, construct a low-voltage bench harness with switches for float/interlock inputs, resistor/potentiometer thermistor simulation, and LEDs or 12-V relay coils for outputs. Document one appliance profile worksheet before connecting any real dehumidifier.

## Bench acceptance matrix

The first firmware candidate should not be considered ready for appliance-specific evaluation until all of these pass with simulated loads:

```text
A01 Power-up: every output OFF
A02 Startup lockout blocks compressor for configured interval
A03 RH >= setpoint+hysteresis begins fan pre-run
A04 Compressor starts only after pre-run and guard eligibility
A05 RH <= setpoint-hysteresis ends normal compressor demand
A06 Fan post-run lasts configured interval
A07 Bucket full removes compressor command immediately
A08 Secondary float removes compressor command when enabled
A09 Pressure/thermal interlock removes compressor command when enabled
A10 Required SHT sensor loss produces critical fault
A11 Required evaporator sensor open/short produces critical fault
A12 MCP23017 loss forces all outputs OFF
A13 ADS1115 loss forces compressor OFF when required
A14 Freeze threshold enters fan-only DEFROST
A15 Recovery threshold exits DEFROST subject to minimum-OFF timer
A16 Defrost timeout produces fault
A17 Service fan/pump/aux tests operate through safe output layer
A18 Service compressor request cannot bypass lockout/interlocks
A19 Reset during compressor demand returns to OFF + startup lockout
A20 Corrupt NVS returns to setup-required compressor-disabled state
```

## Appliance profile worksheet required before mains integration

For each actual dehumidifier, record at minimum:

- make/model and serial family
- supply voltage
- compressor model
- compressor running current/RLA
- compressor LRA/start current if marked
- OEM overload/start relay/PTC/capacitor topology
- fan motor voltage/current and speed wiring
- bucket-switch topology and polarity
- secondary-float topology and polarity, if present
- evaporator NTC resistance at two or more known temperatures
- condenser NTC data, if present
- pump voltage/current, if present
- pressure/thermal interlock topology, if present
- chosen compressor contactor and rating
- chosen fan/pump relay and rating
- protective-earth continuity verification
- OEM fuse/safety-chain elements retained

No mains commissioning should proceed until the worksheet is complete for that appliance and the switching design has been critically reviewed.

## Optional condensate-pump / garden-water branch

The V1 I/O map reserves a pump command and primary/secondary water-level inputs. The intended development sequence is:

1. prove ordinary bucket-full shutdown first;
2. add pump command with low-voltage simulated load;
3. add high-water backup interlock;
4. add pump maximum-run timeout and fault;
5. characterize the actual pump voltage/current before relay selection;
6. only then test water transfer into a separate collection container.

Garden reuse is a separate water-quality decision. Dehumidifier condensate is not drinking water, and appliance cleanliness, airborne contaminants, storage, and plant/application suitability must be considered independently.

## Current release boundary

V1 explicitly does **not** include:

- inverter/variable-speed compressor control
- Wi-Fi/cloud/MQTT/Home Assistant
- OTA updates
- a universal mains-current PCB
- automatic OEM sensor identification
- long-duration operational qualification

Those items can be revisited only after the low-voltage controller and safety behavior are demonstrated on the bench.
