# Baxi Heat Pump Modbus Integration (GTW-08)

[![ESPHome](https://img.shields.io/badge/ESPHome-2025.11.2-blue.svg)](https://esphome.io)
[![License](https://img.shields.io/github/license/martinsdan/UIMBaxiConnect-GTW08)](LICENSE)
[![ESP32-S3](https://img.shields.io/badge/ESP32-S3-orange)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)

ESPHome firmware for **M5Stack AtomS3 Lite + Atomic RS485 Base** to integrate **Baxi Heat Pumps** with **GTW-08 Modbus Gateway** into Home Assistant.

## Features

- ✅ **Flow/Return Temperatures** (Primary & Heat Pump circuits)
- ✅ **Outdoor Temperature** & **DHW Setpoint**
- ✅ **Water Pressure** monitoring
- ✅ **System Power Output** & **COP** (Coefficient of Performance)
- ✅ **Status monitoring** (Main/Sub Status, Heat Pump Active, DHW Active, Cooling)
- ✅ **Control**: Power setpoint, Flow temperature setpoint, ON/OFF
- ✅ **Status LED** feedback with AtomS3 RGB LED
- ✅ **OTA Updates** & **Web Server** for diagnostics
- ✅ **WiFi Fallback AP** for configuration

## Hardware Required

| Component | Purpose |
|-----------|---------|
| M5Stack AtomS3 Lite | ESP32-S3 microcontroller |
| Atomic RS485 Base | RS485 Modbus interface |
| Baxi GTW-08 Modbus Gateway | Baxi to Modbus RTU converter |

**Links:**
- [AtomS3 Lite on M5Stack Shop](https://shop.m5stack.com/products/atoms3-lite-esp32s3-dev-kit)
- [Atomic RS485 Base on M5Stack Shop](https://shop.m5stack.com/products/atomic-rs485-base)
- [Baxi GTW-08 Gateway](https://www.baxi.pt/profissionais/produtos/termostatos-regulacao/gtw)

## Wiring Diagram

```
Baxi Heat Pump
     │ (L-Bus)
     ↓
   GTW-08 (Modbus Gateway)
     │ (X6 Connector)
     ├─ OV (GND) ──→ Atomic RS485 Base GND
     ├─ A (Data+) ──→ Atomic RS485 Base A
     └─ B (Data-) ──→ Atomic RS485 Base B
     │ (RS485)
     ↓
Atomic RS485 Base
     │ (GPIO5/GPIO6 UART)
     ↓
AtomS3 Lite
     │ (WiFi)
     ↓
Home Assistant
```

## GTW-08 Configuration

**Before connecting, set the GTW-08 dip switches:**

- **Rotary Dial:** Position 0 (Modbus Address 100 = 0x64)
- **DIP Switches 1-2:** OFF / OFF (9600 baud)
- **DIP Switches 3-4:** OFF / OFF (No parity)

## Installation

### Step 1: Prepare ESPHome

1. Open Home Assistant → Settings → Devices & Services → ESPHome
2. Click **Create New Device** → Select **ESP32-S3**
3. Name it: `baxiheatpumpatom`

### Step 2: Add Configuration

Replace the generated YAML with `baxi-heat-pump-atom.yaml` from this repository.

### Step 3: Configure Secrets

Edit your `secrets.yaml` file:

```yaml
wifi_ssid: "YourWiFiNetworkName"
wifi_password: "YourWiFiPassword"
fallback_ap_password: "fallback_password_123"
api_key: "your-esphome-api-key"
ota_password: "your-ota-password"
```

### Step 4: Flash

1. Click **Install**
2. Choose **Plug into this computer**
3. Connect AtomS3 Lite via USB-C
4. Wait for flashing to complete

### Step 5: Verify Connection

After first boot:
- Check ESPHome Logs for Modbus communication
- All sensors should appear in Home Assistant within 1-2 minutes
- Status LED should turn green when connected to Modbus

## YAML Configuration Highlights

### Board Selection
```yaml
esp32:
  board: esp32-s3-devkitc-1 # A generic S3 board
  framework:
    type: arduino
```

### UART (RS485 via Atomic Base)
```yaml
uart:
  id: modbus_uart
  tx_pin: GPIO6
  rx_pin: GPIO5
  baud_rate: 9600
  stop_bits: 1
  parity: NONE
```

### Modbus Controller (GTW-08)
```yaml
modbus_controller:
  - id: gtw08_controller
    address: 0x64           # Matches rotary dial position 0
    modbus_id: modbus1
    update_interval: 15s
```

## Available Sensors

### Temperatures
- **Flow Temperature** (Register 400) - Primary circuit
- **Return Temperature** (Register 401) - Primary circuit
- **HP Flow Temperature** (Register 403) - Heat pump circuit
- **HP Return Temperature** (Register 404) - Heat pump circuit
- **Outdoor Temperature** (Register 384)
- **DHW Setpoint** (Register 408)

### System Information
- **Water Pressure** (Register 409)
- **Power Output** (Register 413)
- **COP Current** (Register 9230)
- **System Status** (Register 411)
- **Heat Pump Active** (Register 279)
- **DHW Active** (Register 280)
- **Cooling Active** (Register 280)

### Controls
- **Power Setpoint** (Register 256) - 0-100%
- **Flow Temperature Setpoint** (Register 257) - 20-80°C
- **ON/OFF Switch** (Register 259)

## Troubleshooting

### Sensors Show "Unknown"

**Check 1: GTW-08 Configuration**
```
❌ Rotary Dial not at position 0 → Set to position 0
❌ DIP 1-2 not OFF/OFF → Set both to OFF
❌ DIP 3-4 not OFF/OFF → Set both to OFF
```

**Check 2: Wiring**
```
❌ A/B wires swapped → Swap the RS485 A and B connections
❌ Missing GND connection → Connect OV from GTW-08 to GND
❌ Loose connections → Verify all terminal connections
```

**Check 3: ESPHome Settings**
```
❌ Wrong board → Use m5stack-atoms3-lite
❌ Wrong GPIO pins → TX=GPIO6, RX=GPIO5
❌ Wrong stop_bits → Use stop_bits: 1
❌ Wrong address → Use address: 0x64
```

### Communication Errors

**Check Logs:**
1. Open ESPHome Dashboard
2. Select device → **Show Logs**
3. Look for messages like:
   - `Modbus timeout` → Check wiring and GTW-08 settings
   - `CRC error` → Check baud rate and parity
   - `Exception code` → Check register addresses

**Quick Test:**
Create a minimal YAML with only one sensor:
```yaml
sensor:
  - platform: modbus_controller
    modbus_controller_id: gtw08_controller
    name: "Test Flow Temp"
    address: 400
    register_type: holding
    value_type: S_WORD
    unit_of_measurement: "°C"
    accuracy_decimals: 2
    filters:
      - multiply: 0.01
```

If this sensor shows values, the Modbus connection is working!

## Supported Baxi Models

This integration has been tested and is compatible with:

- ✅ Baxi Platinum BC Plus Monobloc 2
- ✅ UIMB Baxi Connect with GTW-08

**Note:** Compatibility depends on the GTW-08 being officially supported by your specific Baxi model. Check with your installer.

## Status LED Feedback

The RGB LED on AtomS3 Lite provides visual feedback:

| Color | Meaning |
|-------|---------|
| 🟢 Green (50%) | Modbus connected, reading values |
| 🔵 Blue (100%) | Heat pump turned ON |
| 🟠 Orange (30%) | Heat pump in standby |

## Performance & Polling

- **Update Interval:** 15 seconds (configurable)
- **Modbus Baud Rate:** 9600 (fixed by GTW-08)
- **Response Time:** ~250ms per Modbus request
- **WiFi Signal Display:** Every 60 seconds

## Known Limitations

- **Single GTW-08 per system** - Cascade support not yet implemented
- **Read-only COP** - Write operations not supported
- **No alarm codes** - Status only (main/sub state)
- **Zoning support** - Manual configuration required

## Roadmap

- [ ] Energy counters (kWh consumption)
- [ ] Advanced alarm codes & diagnostics
- [ ] Multi-zone support
- [ ] Cascade system optimization
- [ ] InfluxDB direct integration

## Contributing

We welcome contributions! Please:

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/your-feature`
3. Commit your changes: `git commit -m 'Add feature'`
4. Push to branch: `git push origin feature/your-feature`
5. Open a Pull Request

**Before submitting:**
- Test with your Baxi model
- Update README if adding new features
- Include hardware/software versions used

## Issues & Support

Found a problem? Have a question?

- 🐛 **Bug Report:** [Open an Issue](https://github.com/martinsdan/UIMBaxiConnect-GTW08/issues)
- 💬 **Discussion:** Use GitHub Discussions
- 📚 **Documentation:** Check the Wiki

## License

This project is licensed under the **GNU GENERAL PUBLIC LICENSE** - see [LICENSE](LICENSE) file for details.

## Acknowledgments

- **M5Stack** - For the excellent AtomS3 Lite and Atomic RS485 Base
- **Baxi/Remeha** - GTW-08 Modbus Gateway documentation
- **ESPHome** - Robust and flexible microcontroller firmware
- **Home Assistant** - Smart home automation platform
- **Community** - Testing, feedback, and contributions

## References

- [ESPHome Documentation](https://esphome.io/)
- [Baxi GTW-08 Manual](https://www.baxi.pt/)
- [M5Stack AtomS3 Lite Docs](https://docs.m5stack.com/en/core/AtomS3%20Lite)
- [Home Assistant Modbus Integration](https://www.home-assistant.io/integrations/modbus/)

---

**Made with ❤️ for the Home Assistant community**

⭐ If this helped you, please star the repository!

![Last Updated](https://img.shields.io/github/last-commit/martinsdan/UIMBaxiConnect-GTW08)
