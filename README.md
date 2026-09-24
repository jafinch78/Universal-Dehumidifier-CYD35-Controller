# Universal Dehumidifier CYD35 Controller

> **Status: unreviewed thought experiment / early design work.**
>
> This repository is being published to get the project started and to make the design easier to review. The circuit concepts, component selections, pin assignments, firmware architecture, protection logic, and appliance-integration assumptions have **not yet been critically reviewed as a complete system, assembled, electrically validated, or bench-tested on a dehumidifier**. Do not treat the current material as a finished or safety-certified controller.

## Project idea

The goal is a reusable replacement controller for conventional fixed-speed compressor dehumidifiers whose original electronic control boards have failed. The current concept uses a 3.5-inch ESP32 CYD (`ESP32-3248S035R / E32N35T`) as the touchscreen supervisory controller and keeps appliance power switching on separate, appropriately rated hardware.

The project grew from the same general idea as a universal replacement appliance controller: preserve otherwise useful machines when an unavailable or uneconomical control board is the main failure.

## Condensate pump / garden-water motivation

One reason for including an **optional condensate-pump output** is to make collected dehumidifier water easier to route to a storage container or garden-watering setup instead of simply dumping the reservoir.

The controller concept therefore allows for:

- normal bucket/float shutdown,
- a condensate pump output,
- primary and optional secondary high-water/overflow sensing,
- pump timeout/fault handling,
- future selectable modes such as bucket-only, gravity drain, or pumped drain.

This does **not** imply that dehumidifier condensate is potable or universally suitable for every plant or irrigation method. Water quality depends on the appliance, coil cleanliness, airborne contaminants, storage method, and materials contacted by the water. Any garden reuse should be evaluated separately for the intended application.

## Current V1 architecture

```text
CYD 3.5 ESP32 touchscreen
        |
        | external low-voltage I2C
        | GPIO25 SDA / GPIO32 SCL
        v
 +---------------+       +-------------+       +----------------+
 | MCP23017 I/O  |       | ADS1115 ADC |       | SHT31 / SHT4x  |
 +-------+-------+       +------+------+       +--------+-------+
         |                      |                       |
         | digital I/O          | thermistors           | room RH/T
         v                      v                       v
 low-voltage relay driver / interlocks / sensors
         |
         v
 external appliance-rated relays/contactors/power modules
         |
         +--> compressor
         +--> fan(s)
         +--> condensate pump
         +--> optional defrost / auxiliary output
```

The CYD is intended to remain a **low-voltage supervisory controller**. Compressor and fan mains current must not be routed through the CYD or the proposed low-voltage I/O board.

## Intended V1 functions

- Touchscreen humidity setpoint and status display
- Room humidity/temperature sensing
- Compressor minimum-OFF and minimum-ON protection
- Startup compressor lockout after reset/power loss
- Fan pre-run and post-run sequencing
- Bucket-full / condensate interlock
- Optional condensate pump control
- Optional evaporator-temperature freeze/defrost logic
- Configurable appliance profiles
- Fault handling and service diagnostics
- Safe output initialization after boot/reset
- External I/O through MCP23017 + ADS1115

## Important electrical/safety boundary

This repository presently describes an **experimental controller architecture**, not a certified appliance repair procedure.

Any eventual implementation must retain or appropriately re-engineer the appliance's original protective features, including as applicable:

- compressor thermal overload,
- start relay/PTC/capacitor system,
- fusing,
- protective earth/grounding,
- pressure or temperature protection,
- enclosure/fire protection,
- creepage and clearance,
- conductor and connector current ratings.

Relay or contactor selection must be based on the actual compressor/motor load, including starting or locked-rotor current—not merely a relay's resistive-load rating.

**Do not connect the current untested design to mains voltage or an appliance compressor based solely on this repository.** Initial development should be performed with low-voltage simulated loads, followed by controlled bench validation and appliance-specific engineering review.

## Hardware target currently assumed

V1 is being planned around the known CYD board profile:

- `ESP32-3248S035R / E32N35T`
- 3.5-inch 320x480 resistive touchscreen
- TFT backlight: GPIO27
- External appliance I2C: GPIO25 SDA / GPIO32 SCL
- MCP23017 nominal address: `0x20`
- ADS1115 nominal address: `0x48`
- SHT31 or SHT4x room humidity/temperature sensor

These details are design assumptions until the complete assembled controller is verified.

## Documents

- [V1 design specification](docs/design/Universal_Dehumidifier_Controller_V1_Design.md)
- [V1 implementation plan](docs/plan/Universal_Dehumidifier_Controller_V1_Implementation_Plan.md)
- [Project disclaimer and validation status](DISCLAIMER.md)

## Development status

As of the initial repository publication:

- [x] Concept architecture drafted
- [x] CYD target and low-voltage expansion strategy selected
- [x] Control-state and safety requirements drafted
- [x] Firmware implementation plan drafted
- [ ] Critical schematic review
- [ ] Exact component/BOM review
- [ ] Low-voltage prototype assembled
- [ ] Firmware compiled for the target CYD
- [ ] Simulated-load bench testing
- [ ] Fault-injection testing
- [ ] Mains power-stage engineering review
- [ ] Appliance-specific integration test
- [ ] Long-duration operational validation

Until those unchecked items are completed, this repository should be regarded as **research/design notes and a development starting point only**.
