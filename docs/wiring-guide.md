# Hardware Wiring Guide

## Power Distribution
The ESP32 3.3V and GND pins are connected to the breadboard power rails to provide common power and ground for all connected components.

All sensor modules are connected in parallel using the shared power rails.

---

## Sensor Connections

### MAX6675 Thermocouple
| Sensor Pin | ESP32 Pin |
|---|---|
| SO | D18 |
| CS | D19 |
| SCK | D21 |

### Water Flow Sensor (YF-S401)
| Sensor Pin | ESP32 Pin |
|---|---|
| Signal Output | D4 |

### UV Sensor (GUVA-S12SD)
| Sensor Pin | ESP32 Pin |
|---|---|
| Signal Output | D25 |

### Buzzer
| Component Pin | ESP32 Pin |
|---|---|
| Positive | D13 |
| Negative | GND |

---

## Power Supply
After uploading the code, the ESP32 is powered using two 3.7V batteries connected in series.

- Positive terminal → VIN
- Negative terminal → GND

---

## Water Pump Power Circuit
The 12V water pump is powered using three 3.7V batteries.

A potentiometer is used to control the pump speed.

### Connections
- Battery positive → potentiometer middle pin
- Potentiometer side pin → pump positive
- Battery negative → pump negative
