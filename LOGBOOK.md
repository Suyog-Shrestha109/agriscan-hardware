# AgriScan — Build Log

## 2026-08-31

Set up this repo. Recovered the BLE sketches from Slack — they had never been committed anywhere. Added the sensor integration sketch too.

Found the KiCad schematic from April and got it into version control: source, plotted PDF, and a generated BOM. Ran Annotate first — the second soil sensor header was still sitting at `Capacitive?` and showing up wrong in the BOM. Also added the hand-dimensioned perfboard layout drawing.

Design note: CropBands now run the same ESP32-WROOM-32U as the Hub, not the C3. One chip, one radio stack, one packet format on both ends.

Next up:
- Reflow the intermittent OUT/GND joint to the MT3608, flex-test under continuity
- Set the boost trimpot to 5.00V with output floating, before reconnecting the ESP32
- Confirm the 3V3 pin reads 3.3V after the 9V incident
- Push the firmware fixes (shared hub_config.h + static_assert on packet size) — waiting on repo access
