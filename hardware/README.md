# Hardware

Design documents for the AgriScan hub and CropBand nodes.

| File | What it is |
|---|---|
| `hub-perfboard-layout-dimensioned.jpg` | Hand-drawn component layout, 9.3 × 7 cm perfboard. Measured footprints, clearances, connector positions, mounting holes. The enclosure CAD was dimensioned from this. |
| `schematic.pdf` | Hub schematic (KiCad) |
| `pin-map.md` | GPIO assignments |
| `bom.csv` | Parts list — quantity, hub or cropband, price |
| `power-chain.md` | Panel → charger → cell → boost → ESP32 |
| `cad/` | Enclosure — Fusion source, STL, STEP |

## Power chain

Voltaic 6V 2W panel → bq24074 DC jack → 18650 (1S DW01 protection) → MT3608 boost → 5V → ESP32.
No diode or clamp — the bq24074 handles reverse current and input limiting internally.
