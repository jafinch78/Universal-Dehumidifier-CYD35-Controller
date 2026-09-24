# Universal Dehumidifier CYD35 Controller

> **Status: unreviewed thought experiment / early design work.**
>
> This repository is being published to get the project started and to make the design easier to review. The circuit concepts, component selections, pin assignments, firmware architecture, protection logic, and appliance-integration assumptions have **not yet been critically reviewed as a complete system, assembled, electrically validated, or bench-tested on a dehumidifier**. Do not treat the current material as a finished or safety-certified controller.

## Project idea

The goal is a reusable replacement controller for conventional fixed-speed compressor dehumidifiers whose original electronic control boards have failed. The current concept uses a 3.5-inch ESP32 CYD (`ESP32-3248S035R / E32N35T`) as the touchscreen supervisory controller and keeps appliance power switching on separate, appropriately rated hardware.

## Condensate pump / garden-water motivation

One reason for including an **optional condensate-pump output** is to make collected dehumidifier water easier to route to a storage container or garden-watering setup instead of simply dumping the reservoir. The concept allows bucket shutdown, a pump output, high-water sensing, pump timeout/fault handling, and future bucket/gravity/pumped modes. Dehumidifier condensate is not potable and is not automatically suitable for every irrigation use.

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

The V0.1 controller core has been compiled as C++17 and exercised for startup lockout, compressor timing, bucket-full shutdown, required-I2C loss, invalid configuration, 32-bit timer wraparound, immediate safety override, condensate-pump request behavior, and secondary-high-water fault shutdown. This is software-only evidence, not hardware validation.

## Important electrical/safety boundary

The CYD and low-voltage I/O board are not a universal mains power board. Compressor/fan/pump mains switching remains external and application-specific. Relay/contactor selection must be based on actual motor/compressor starting and running loads, and existing appliance safety devices remain required unless separately engineered.

**Do not connect the current untested design to mains voltage or an appliance compressor based solely on this repository.** Initial development should use low-voltage simulated loads.

## Hardware target currently assumed

- `ESP32-3248S035R / E32N35T`
- TFT backlight GPIO27
- External appliance I2C GPIO25 SDA / GPIO32 SCL
- MCP23017 `0x20`
- ADS1115 `0x48`
- SHT31 or SHT4x room RH/T sensor

## Development status

- [x] Concept architecture drafted
- [x] Proposed prototype BOM drafted
- [x] Connection-level low-voltage wiring drafted
- [x] Host-testable compressor/controller core drafted
- [x] Initial host safety tests passing
- [ ] Critical schematic/BOM review
- [ ] Low-voltage prototype assembled
- [ ] Arduino sketch compiled for the target CYD
- [ ] Simulated-load bench testing on real I/O hardware
- [ ] Fault-injection testing on real hardware
- [ ] Appliance-specific mains power-stage review
- [ ] Long-duration operational validation

Until the unchecked items are completed, this repository remains **research/design notes and a development starting point only**.
