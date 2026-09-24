# Experimental Status and Safety Disclaimer

This project is currently an **unreviewed thought experiment and engineering-development concept**. It is published early to encourage structured review and iterative development.

The material in this repository has not yet been validated as a complete appliance-control system. In particular, it has not yet undergone all of the following:

- independent critical design review,
- complete schematic/ERC review,
- PCB/layout review,
- component derating review,
- assembled prototype testing,
- compressor/motor load testing,
- mains isolation/creepage/clearance validation,
- fault-injection testing,
- thermal testing,
- long-duration reliability testing,
- appliance-specific regulatory or safety evaluation.

Portable dehumidifiers contain hazardous mains voltage and compressor/motor loads with significant starting current. Incorrect control wiring, relay/contact selection, grounding, insulation, or firmware behavior can create shock, fire, flooding, compressor damage, or other hazards.

The proposed CYD controller and I/O electronics are intended to remain on the **low-voltage side**. Mains loads must be switched only by power hardware properly engineered and rated for the specific appliance.

Existing manufacturer safety devices should remain in circuit unless a replacement has been deliberately engineered and validated. Software protections are supplemental and must not be treated as substitutes for required hardware protection.

## Condensate reuse

The optional condensate-pump feature was included partly to support collection of dehumidifier water for possible garden watering. Dehumidifier condensate is **not drinking water**. Its suitability for irrigation depends on the cleanliness and materials of the appliance and collection system, airborne contaminants, storage conditions, and the intended plants/application. Water reuse should be evaluated independently from the controller design.

## Current intended development order

1. Review and correct the design documents.
2. Build only the low-voltage controller and simulated-load interfaces.
3. Verify all outputs default OFF and remain OFF through resets and faults.
4. Validate sensor failures, I2C failures, bucket-full behavior, pump timeouts, and compressor lockout logic.
5. Review the mains switching design for a specific appliance and measured/nameplate loads.
6. Only then proceed to controlled appliance testing.

Nothing in the current repository should be interpreted as an instruction to bypass appliance protections or connect an untested circuit directly to mains voltage.
