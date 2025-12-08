# Baxi Heat Pump Modbus Integration (GTW-08)

[![ESPHome](https://img.shields.io/badge/ESPHome-2025.11.2-blue.svg)](https://esphome.io)
[![License](https://img.shields.io/github/license/martinsdan/UIMBaxiConnect-GTW08)](LICENSE)
[![ESP32-S3](https://img.shields.io/badge/ESP32-S3-orange)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)

ESPHome firmware for **M5Stack AtomS3 Lite + Atomic RS485 Base** to integrate **Baxi Heat Pumps** with **GTW-08 Modbus Gateway** into Home Assistant.

## Features

- ✅ **Flow/Return Temperatures** (Primary & Heat Pump circuits)
- ✅ **Outdoor Temperature** & **DHW Setpoint**
- ✅ **Water Pressure** monitoring
- ✅ **System Power Output**
- ✅ **Status monitoring** with friendly names (from GTW-08 manual)
- ✅ **Full Control**: System ON/OFF, Algorithm Type, Heat Demand, Power/Flow/DHW Setpoints
- ✅ **Status LED** feedback with AtomS3 RGB LED
- ✅ **OTA Updates** & **Web Server** for diagnostics
- ✅ **WiFi Fallback AP** for configuration
- ✅ **Boot protection** - Preserves heat pump state on restart (no spurious cycling)

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

## Configuration Details

### Board & Framework
```yaml
esp32:
  board: esp32-s3-devkitc-1
  framework:
    type: arduino
```

### Boot Protection

To prevent the heat pump from cycling on ESPHome restart, a global boot flag is used:

```yaml
globals:
  - id: boot_complete
    type: bool
    restore_value: no
    initial_value: 'false'

esphome:
  on_boot:
    priority: -100
    then:
      - globals.set:
          id: boot_complete
          value: 'true'
```

The **System ON/OFF switch** only executes its turn_on/turn_off actions after `boot_complete` is set to true. This preserves the device's actual operational state across restarts.

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

### Modbus Controller
```yaml
modbus_controller:
  - id: gtw08_controller
    address: 0x64           # GTW-08 default address
    modbus_id: modbus1
    setup_priority: 100     # Ensures reads before switch evaluates
    update_interval: 15s    # Poll every 15 seconds
    command_throttle: 200ms # Wait between requests
```

## Home Assistant Integration

### Switches
- **System ON/OFF** - Toggle heat pump (sets Algorithm Type: 3=ON, 0=OFF)

### Number Controls (Read-Write)
- **Algorithm Type** (0-3) - 0=OFF, 3=Remote monitoring (ON)
- **Heat Demand** (0-8) - Heat demand level
- **Power Setpoint** (0-100%) - System power output
- **Flow Temperature Setpoint** (20-80°C) - Primary circuit flow temp
- **DHW Temperature Setpoint** (30-60°C) - Domestic hot water setpoint

### Sensors (Read-Only)

**Temperatures:**
- Flow Temperature (Primary circuit)
- Return Temperature (Primary circuit)
- HP Flow Temperature (Heat pump circuit)
- HP Return Temperature (Heat pump circuit)
- Outdoor Temperature
- DHW Setpoint

**System Data:**
- Water Pressure (bar)
- Power Output (%)
- System Status Code (numeric)
- System Status (friendly name)
- Sub Status Code (numeric)
- Sub Status (friendly name)

**Diagnostics:**
- Error Code
- WiFi Signal
- Uptime

## System Status Codes

<details>
<summary><b>Click to expand System Status Codes (Register 411 - AM012)</b></summary>

| Code | Status |
|------|--------|
| 0 | Standby |
| 1 | Heat Demand |
| 2 | Generator start |
| 3 | Generator CH |
| 4 | Generator DHW |
| 5 | Generator stop |
| 6 | Pump Post Run |
| 7 | Cooling Active |
| 8 | Controlled Stop |
| 9 | Blocking Mode |
| 10 | Locking Mode |
| 11 | Load test min |
| 12 | Load test CH max |
| 13 | Load test DHW max |
| 15 | Manual Heat Demand |
| 16 | Frost Protection |
| 17 | Deaeration |
| 18 | Control unit Cooling |
| 19 | Reset In Progress |
| 20 | Auto Filling |
| 21 | Halted |
| 22 | Forced calibration |
| 23 | Factory test |
| 24 | Hydronic balancing |
| 200 | Device Mode |
| 254 | Unknown |

Reference: GTW-08 Manual, Table 16

</details>

## Sub Status Codes

<details>
<summary><b>Click to expand Sub Status Codes (Register 412 - AM014)</b></summary>

| Code | Sub Status |
|------|-----------|
| 0 | Standby |
| 1 | AntiCycling |
| 2 | CloseHydraulicValve |
| 3 | ClosePump |
| 4 | WaitingForStartCond |
| 10 | CloseExtGasValve |
| 11 | StartToGlueGasValve |
| 12 | CloseFlueGasValve |
| 13 | FanToPrePurge |
| 14 | WaitForReleaseSignal |
| 15 | BurnerOnCommandToSu |
| 16 | VpsTest |
| 17 | PreIgnition |
| 18 | Ignition |
| 19 | FlameCheck |
| 20 | Interpurge |
| 21 | Generator starting |
| 30 | Normal Int.Setpoint |
| 31 | Limited Int.Setpoint |
| 32 | NormalPowerControl |
| 33 | GradLevel1PowerCtrl |
| 34 | GradLevel2PowerCtrl |
| 35 | GradLevel3PowerCtrl |
| 36 | ProtectFlamePwrCtrl |
| 37 | StabilizationTime |
| 38 | ColdStart |
| 39 | ChResume |
| 40 | SuRemoveBurner |
| 41 | FanToPostPurge |
| 42 | OpenExtFlueGasValve |
| 43 | StopFanToFlueGVRpm |
| 44 | StopFan |
| 45 | LimitedPwrOnTflueGas |
| 46 | AutoFillingInstall |
| 47 | AutoFillingTopUp |
| 48 | Reduced Set Point |
| 49 | Offset adaption |
| 60 | PumpPostRunning |
| 61 | OpenPump |
| 62 | OpenHydraulicValve |
| 63 | Start anticycle time |
| 65 | Compressor relieved |
| 66 | HP Tmax backup on |
| 67 | Outdoor limit HP off |
| 68 | HP stop by hybrid |
| 69 | Defrost with HP |
| 70 | Defrost with backup |
| 71 | Defrost HP backup |
| 72 | Source pump backup |
| 73 | HP flow over Tmax |
| 74 | Source pump post run |
| 75 | HP off high humidity |
| 76 | HP off water flow |
| 78 | Humidity setpoint |
| 79 | Generators relieved |
| 80 | HP relieved cooling |
| 81 | HP stop outdoor temp |
| 82 | HP off flow Tmax |
| 83 | Deair pump valve CH |
| 84 | Deair pump valve DHW |

Reference: GTW-08 Manual, Table 17

</details>

## Status LED Feedback

The RGB LED on AtomS3 Lite provides visual feedback:

| Color | Meaning |
|-------|---------|
| 🟢 Green (50%) | Modbus connected, reading values |
| 🔵 Blue (100%) | Heat pump turned ON |
| 🟠 Orange (30%) | Heat pump in standby |

## Troubleshooting

### System Turns Off on Every Reboot

The boot protection mechanism prevents this. If you experience spurious cycling:

1. Check that `boot_complete` flag logic is in place in your YAML
2. Verify `setup_priority: 100` is set on modbus_controller
3. Check logs for "Boot complete, switch is now active" message
4. Clear `.storage/` directory and reflash

### Sensors Show "Unknown"

**Check GTW-08 Configuration:**
- Rotary Dial must be at position 0
- DIP switches 1-2 must be OFF/OFF
- DIP switches 3-4 must be OFF/OFF

**Check Wiring:**
- GTW-08 OV (GND) → Atomic RS485 Base GND
- GTW-08 A (Data+) → Atomic RS485 Base A
- GTW-08 B (Data-) → Atomic RS485 Base B
- All connections must be secure

**Check YAML:**
- Board: `esp32-s3-devkitc-1`
- UART TX: GPIO6, RX: GPIO5
- Stop bits: 1
- Parity: NONE
- Modbus address: 0x64

### Communication Errors

Check ESPHome logs for:
- `Modbus timeout` - Verify wiring and GTW-08 settings
- `CRC error` - Verify baud rate (9600) and parity (NONE)
- `Exception code` - Verify register addresses in YAML

**Quick Test:** Create a minimal sensor config with just one register (e.g., 400 for flow temperature). If it shows values, Modbus communication is working.

## Supported Baxi Models

Compatibility depends on GTW-08 support from Baxi:

- ✅ Baxi Platinum BC Plus Monobloc 2
- ✅ UIMB Baxi Connect with GTW-08

Verify GTW-08 compatibility with your system's documentation.

## Performance & Timing

- **Modbus Polling:** Every 15 seconds
- **Baud Rate:** 9600 (fixed by GTW-08)
- **Response Time:** ~250ms per Modbus command
- **WiFi Signal Update:** Every 60 seconds
- **Boot Protection Delay:** ~100ms to ensure modbus reads

## Known Limitations

- Single GTW-08 per system (cascade not supported)
- Status codes limited to GTW-08 manual definitions
- Energy counters not yet implemented (registers available)
- Zoning requires manual configuration

## Roadmap

- [ ] Energy counters (kWh consumption)
- [ ] Advanced diagnostics dashboard
- [ ] Multi-zone support
- [ ] Cascade system optimization
- [ ] InfluxDB integration
- [ ] Home Assistant energy dashboard examples

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/your-feature`
3. Commit your changes: `git commit -m 'Add feature'`
4. Push to branch: `git push origin feature/your-feature`
5. Open a Pull Request

**Before submitting:**
- Test with your Baxi model
- Update README with any new features
- Include hardware/software versions
- Reference GTW-08 manual for register documentation

## Issues & Support

- 🐛 **Bug Report:** [Open an Issue](https://github.com/martinsdan/UIMBaxiConnect-GTW08/issues)
- 💬 **Questions:** GitHub Discussions
- 📚 **Documentation:** Check the Wiki
- 📖 **GTW-08 Manual:** See `/docs` folder

## License

This project is licensed under **GPL-3.0** - see [LICENSE](LICENSE) file for details.

## Acknowledgments

- **M5Stack** - AtomS3 Lite and Atomic RS485 Base
- **Baxi/Remeha** - GTW-08 Modbus Gateway documentation
- **ESPHome** - Microcontroller firmware platform
- **Home Assistant** - Home automation ecosystem
- **Community** - Testing, feedback, and contributions

## References

- [ESPHome Documentation](https://esphome.io/)
- [Baxi GTW-08 Manual](https://www.baxi.pt/)
- [M5Stack AtomS3 Lite](https://docs.m5stack.com/en/core/AtomS3%20Lite)
- [Home Assistant Modbus](https://www.home-assistant.io/integrations/modbus/)

---

**Made with ❤️ for the Home Assistant community**

⭐ If this helped you, please star the repository!

![Last Updated](https://img.shields.io/github/last-commit/martinsdan/UIMBaxiConnect-GTW08)
