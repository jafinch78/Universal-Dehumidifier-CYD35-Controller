# Universal Dehumidifier CYD35 Controller

> **Status: unreviewed thought experiment / early design work.**
>
> This repository is being published to get the project started and to make the design easier to review. The circuit concepts, component selections, pin assignments, firmware architecture, protection logic, and appliance-integration assumptions have **not yet been critically reviewed as a complete system, assembled, electrically validated, or bench-tested on a dehumidifier**. Do not treat the current material as a finished or safety-certified controller.

## Project idea

The goal is a reusable replacement controller for conventional fixed-speed compressor dehumidifiers whose original electronic control boards have failed. The current concept uses a 3.5-inch ESP32 CYD (`ESP32-3248S035R / E32N35T`) as the touchscreen supervisory controller and keeps appliance power switching on separate, appropriately rated hardware.

The project grew from the same general idea as a universal replacement appliance controller: preserve otherwise useful machines when an unavailable or uneconomical control board is the main failure.

## Condensate pump / garden-water motivation

One reason for including an **optional condensate-pump output** is to make collected dehumidifier water easier to route to a storage container or garden-watering setup instead of simply dumping the reservoir.

The controller concept therefore allows for normal bucket/float shutdown, a condensate pump output, primary and optional secondary high-water/overflow sensing, pump timeout/fault handling, and future selectable bucket/gravity/pumped drain modes.

Dehumidifier condensate is not potable and is not automatically suitable for every irrigation use. Water quality depends on appliance cleanliness, airborne contaminants, storage, and contacted materials.

## Current V1 architecture

```text
CYD 3.5 ESP32 touchscreen
        |
        | GPIO25 SDA / GPIO32 SCL
        v
 +---------------+       +-------------+       +----------------+
 | MCP23017 I/O  |       | ADS1115 ADC |       | SHT31 / SHT4x  |
 +-------+-------+       +------+------+       +--------+-------+
         |                      |                       |
      ULN2803A              thermistors              room RH/T
         |
         v
 external appliance-rated relays/contactors
         |
         +--> compressor
         +--> fan(s)
         +--> condensate pump
```

The CYD remains a **low-voltage supervisory controller**. Compressor and fan mains current must not be routed through the CYD or proposed I/O board.

## Starter engineering package

The repository now contains a V0.1 bench-oriented starter package:

- [Proposed prototype BOM](hardware/BOM.csv)
- [Low-voltage wiring / connection-level schematic](hardware/LOW_VOLTAGE_WIRING.md)
- [Arduino/CYD starter firmware](firmware/UniversalDehumidifier/UniversalDehumidifier.ino)
- [Host-testable compressor guard and controller core](firmware/UniversalDehumidifier/)
- [Host safety tests](tests/host/test_main.cpp)
- [V1 design specification](docs/design/Universal_Dehumidifier_Controller_V1_Design.md)
- [V1 implementation plan](docs/plan/Universal_Dehumidifier_Controller_V1_Implementation_Plan.md)
- [Project disclaimer and validation status](DISCLAIMER.md)

The V0.1 Arduino sketch deliberately displays **`OUTPUTS LOCKED - BENCH MODE`** and does not yet enable the MCP23017 relay-coil output adapter. The compressor timing/state logic can therefore be exercised on a host before any appliance output becomes energizable.

### Current host-test coverage

The local V0.1 controller core has been compiled as C++17 and exercised for startup lockout, compressor minimum timing, bucket-full shutdown, required-I2C loss, invalid configuration, 32-bit timer wraparound, immediate safety override, condensate-pump request behavior, and secondary-high-water fault shutdown. This is software-only evidence, not hardware validation.

## Important electrical/safety boundary

This repository presently describes an **experimental controller architecture**, not a certified appliance repair procedure.

Any eventual implementation must retain or appropriately re-engineer compressor thermal overload/start components, fusing, protective earth, pressure/temperature protection, enclosure/fire protection, creepage/clearance, and conductor/connector current ratings as applicable.

Relay or contactor selection must be based on actual compressor/motor load, including starting or locked-rotor current—not merely a resistive-load rating.

**Do not connect the current untested design to mains voltage or an appliance compressor based solely on this repository.** Initial development should use low-voltage simulated loads, followed by controlled bench validation and appliance-specific engineering review.

## Hardware target currently assumed

- `ESP32-3248S035R / E32N35T`
- 3.5-inch 320x480 resistive touchscreen
- TFT backlight: GPIO27
- External appliance I2C: GPIO25 SDA / GPIO32 SCL
- MCP23017 nominal address: `0x20`
- ADS1115 nominal address: `0x48`
- SHT31 or SHT4x room humidity/temperature sensor

## Development status

- [x] Concept architecture drafted
- [x] CYD target and low-voltage expansion strategy selected
- [x] Control-state and safety requirements drafted
- [x] Firmware implementation plan drafted
- [x] Proposed prototype BOM drafted
- [x] Connection-level low-voltage wiring drafted
- [x] Host-testable compressor/controller core drafted
- [x] Initial host safety tests passing
- [ ] Critical schematic review
- [ ] Exact component/BOM review
- [ ] Low-voltage prototype assembled
- [ ] Arduino sketch compiled for the target CYD
- [ ] Simulated-load bench testing on real I/O hardware
- [ ] Fault-injection testing on real hardware
- [ ] Mains power-stage engineering review
- [ ] Appliance-specific integration test
- [ ] Long-duration operational validation

Until the unchecked items are completed, this repository remains **research/design notes and a development starting point only**.
