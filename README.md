# bike-speedometer-kyoto-2019
Arduino bicycle speedometer (Kyoto Award 2019): LCD/servo speed, Hall pulses, LDR night LEDs, 0–45 km/h.

# Arduino Bicycle Speedometer — Kyoto Award 2019

**One-liner.** LCD speed & distance (0–45 km/h), analog **servo needle**, **Hall sensor** pulses, and **LDR-based** night LEDs for safer cycling. Awarded **Excellence in Electronics (Kyoto, 2019).**

## Features
- Live **speed** and **trip distance** on LCD 16×2 (I²C).
- Servo needle angle mapped to speed for analog feel.
- Night mode: LEDs toggle when ambient light falls below threshold (CDS/LDR).
- Simple BOM; reproducible wiring; configurable wheel circumference.

## Hardware (BOM) — WIP
Arduino Uno (or compatible), Hall sensor + magnet, LCD 16×2, micro servo (SG90), LDR + divider, LEDs + resistors.

## Wiring / Media
- Schematic: `hardware/fritzing/diagram.png` (to be added)  
- Short demo GIF: `media/demo.gif` (to be added)
