# AgriScan — ESP32-to-ESP32 BLE Communication Layer

**Author:** Suyog Shrestha
**Project:** AgriScan (Unity Provisions) — WiFi-free soil moisture sensing for small farms
**Tracked in the project task system as:** "Work on the Bluetooth-like connection between ESP32s"
**Written:** January 22, 2026 — recovered from Slack `#agriscan-general`

The device-to-device communication layer for AgriScan, built in four stages in one session.
The product requirement driving this work: the system has to move sensor data between a Hub
and remote CropBands **without any internet connection**, because the farms it targets don't
have one.

## Progression

| File | Time | What it adds |
|---|---|---|
| `01_sender_basic.ino` | 17:54 | Advertises as `ESP32_SENDER`, custom service/characteristic UUIDs, notifies a ticking counter to prove the channel works |
| `02_receiver_basic.ino` | 17:56 | Scans, matches on device name, connects, reads once, subscribes to notifications, retries on failure |
| `03_sender_ntp_time.ino` | 18:33 | Real-time clock over WiFi via NTP, with graceful fallback to device uptime when WiFi is unavailable; streams timestamped payloads |
| `04_receiver_time_range.ino` | 18:37 | Converts RSSI to estimated distance via the log-distance path-loss model |

## The physics in stage 4

```
distance = 10 ^ ((txPower - rssi) / (10 * n))
```

`txPower` is the reference RSSI at one meter (−59 dBm default), `n` is the path-loss exponent
(2.2 here, tuned above the free-space value of 2.0 to account for obstruction). Signal strength
falls off logarithmically with distance, so inverting that relationship gives a distance estimate
from a value the radio already reports.

## Design decisions worth noting

- **Named-device matching rather than UUID-only scanning** — simpler pairing, at the cost of
  requiring a known device name.
- **NTP with an uptime fallback** — the device must keep producing usable timestamps when the
  network is unavailable, which is the normal case in the field. Rather than blocking on WiFi,
  it degrades to monotonic uptime.
- **8-second WiFi timeout, 20 × 200ms NTP retry** — bounded so the device always reaches its
  BLE advertising state.
- **Notification-based rather than polling** — the receiver subscribes instead of repeatedly
  reading, which matters for battery life on a field-deployed sensor.

## Known limitations

- WiFi credentials are hardcoded placeholders; provisioning was not yet built.
- The path-loss exponent `n` is estimated, not calibrated against measured distances.
- Distance estimation from RSSI is inherently noisy and needs filtering before it drives anything.

## Next

The AgriScan Issue Registry (v1.0, March 2026) flags as High severity: *"No CropBand pairing
system — no code exists to discover, register, or pair ESP32-C3 CropBands via ESP-NOW."*
That is the direct continuation of this work.

---

**Source:** Slack `#agriscan-general`, 2026-01-22. Recovered verbatim; the only edit is restoring
`pool.ntp.org` in stage 3, which Slack had auto-linked.
