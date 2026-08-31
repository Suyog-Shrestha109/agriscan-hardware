# Hardware

Design documents for the AgriScan hub.

| File | What it is |
|---|---|
| `hub-perfboard-layout-dimensioned.jpg` | Hand-drawn component layout, 9.3 × 7 cm perfboard. Measured footprints, clearances, connector positions, mounting holes. The enclosure CAD was dimensioned from this. |
| `AgriScan Hub Prefboard.kicad_sch` | Hub schematic — KiCad source |
| `AgriScan Hub Prefboard.kicad_pro` | KiCad project file |
| `AgriScan Hub Prefboard.pdf` | Schematic, plotted for viewing |
| `bom.csv` | Bill of materials, generated from the schematic |

## Power chain

Voltaic 6V 2W panel → bq24074 DC jack → 18650 (1S DW01 protection) → MT3608 boost → 5V → ESP32.
No diode or clamp — the bq24074 handles reverse current and input limiting internally.
