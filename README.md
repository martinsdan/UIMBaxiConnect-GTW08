# Baxi Heat Pump Modbus Integration (GTW-08)

[![ESPHome](https://img.shields.io/badge/ESPHome-2025.11.5-blue.svg)](https://esphome.io)
[![License](https://img.shields.io/github/license/martinsdan/UIMBaxiConnect-GTW08)](LICENSE)
[![ESP32-S3](https://img.shields.io/badge/ESP32-S3-orange)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)

ESPHome firmware for **M5Stack AtomS3 Lite + Atomic RS485 Base** to integrate **Baxi Heat Pumps** with **GTW-08 Modbus Gateway** into Home Assistant.

## Features

- ✅ **Temperature Monitoring**:
  - Outdoor Temperature (from GTW-08 sensor)
  - Flow & Return Temperatures (primary circuit)
  - HP Flow & Return Temperatures (heat pump circuit)
  - Heating Curve Target (calculated: base + gradient × (20 - outdoor temp))
  
- ✅ **System Control**:
  - Algorithm Type (Both Temp & Power / Power Only / Temperature Only / Monitoring Only)
  - Heat Demand (Standby / Heating / Cooling)
  - Zone 1 Operating Mode (Scheduling / Manual / Off)
  - Zone 1 Heating Curve Base (adjustable)
  - Zone 1 Heating Curve Gradient (adjustable)
  
- ✅ **Monitoring & Diagnostics**:
  - Water Pressure (bar) - *May not be available on all systems*
  - Power Output (%) - *May not be available on all systems*
  - COP - Instantaneous - *May not be available on all systems*
  - Power - Actual Output (kW) - *May not be available on all systems*
  - Pump Speed (%) - Real-time pump operation
  - System Status (25 operational states)
  - Sub Status (100+ detailed operation states)
  - Error Detection & Board Diagnostics
  - Board Information (Number of boards, device types)
  - Energy Counters (Central Heating, DHW, Cooling, Total)
  
- ✅ **Status LED** feedback with AtomS3 RGB LED:
  - 🔵 Blue (100%): Heating Active
  - 🟦 Cyan (100%): Cooling Active
  - 🟠 Orange (30%): Standby
  
- ✅ **Smart Data Handling**:
  - Sentinel value detection (shows "unavailable" for unsupported registers)
  - Outlier filtering for temperature sensors
  - Range validation for all sensor values
  - Boot protection prevents spurious commands during startup
  
- ✅ **OTA Updates** & **Web Server** for diagnostics
- ✅ **WiFi Fallback AP** for configuration
- ✅ **Hardware Button** (GPIO41) - Toggle status LED on press

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
- Status LED should show appropriate color based on heat pump state

## Configuration Details

### Board & Framework
```yaml
esp32:
  board: esp32-s3-devkitc-1
  framework:
    type: arduino
```

### Boot Protection

To prevent unintended changes on ESPHome restart:

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
      - logger.log: "Boot complete, select is now active"
```

Controls only become active after boot completes, preventing spurious heat pump cycling.

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
    setup_priority: 100     # Ensures reads before select evaluates
    update_interval: 30s    # Poll every 30 seconds (optimized for stability)
    command_throttle: 1000ms # Wait between requests (increased for reliability)
```

## Home Assistant Integration

### Selects (Read-Write)

- **Algorithm Type** - Remote control modes:
  - "Both Temp & Power" (0)
  - "Power Only" (1)
  - "Temperature Only" (2)
  - "Monitoring Only" (3)

- **Heat Demand** - System operation:
  - "Standby" (0)
  - "Heating" (7) - Blue LED
  - "Cooling" (8) - Cyan LED

- **Zone 1 Operating Mode** - Heating curve operation:
  - "Scheduling (uses heating curve)" (0)
  - "Manual (fixed setpoint)" (1)
  - "Off" (2)

### Number Controls (Read-Write)

- **Zone 1 Heating Curve Base** - Target flow at 20°C outdoor
- **Zone 1 Heating Curve Gradient** - Increase per °C outdoor drop
- **Target Power** - (0 - 100%) System Power Request (Register 256)
- **Target Pump Speed** - (0 - 100%) Pump Speed Control (Register 459)

### Sensors (Read-Only)

**Temperatures (°C):**
- Outdoor Temperature (from GTW-08)
- Flow Temperature (primary circuit)
- Return Temperature (primary circuit)
- HP Flow Temperature (heat pump circuit)
- HP Return Temperature (heat pump circuit)
- Heating Curve Target (calculated)

**Power & Performance:**
- Water Pressure (bar) - *May show "unavailable" depending on system configuration*
- Power Output (%) - *May show "unavailable" depending on system configuration*
- COP - Instantaneous - *May show "unavailable" depending on system configuration*
- Power - Actual Output (kW) - *May show "unavailable" depending on system configuration*
- Pump Speed (%) - Real-time pump operation
- Current System Power (%) - Power received from consumer manager

**Energy Monitoring:**
- Energy - Central Heating (kWh)
- Energy - Domestic Hot Water (kWh)
- Energy - Cooling (kWh)
- Energy - Total (kWh)
- COP - Instantaneous - *May show "unavailable" depending on model*
- Seasonal Mode (Winter/Summer/Frost Protection)
- System Error Status

**System Diagnostics:**
- System Status (friendly name from code)
- Sub Status (detailed operation state)
- Number of Control Boards (shows connected boards)
- Board 1/2/3 Device Types (identifies board types)
- Error Flags (system-wide error detection)

**Network & Device:**
- WiFi Signal (dBm)
- Uptime (seconds)
- IP Address

## Data Availability

Some sensors may show **"unavailable"** depending on your specific heat pump model, system configuration, or operational state. This is normal behavior:

### Always Available:
- ✅ Temperature sensors (outdoor, flow, return, HP circuits)
- ✅ System status and sub-status
- ✅ Pump speed (when pump is running)
- ✅ Energy counters
- ✅ Error detection and diagnostics

### May Show "Unavailable":
- ❓ **Water Pressure** - Availability depends on system configuration
- ❓ **Power Output & Actual Power** - Many monobloc models (e.g., Platinum BC Plus) do not report power data to the gateway.
- ❓ **COP - Instantaneous** - Most models return a sentinel value indicating this is not supported.

**Note:** The GTW-08 gateway returns sentinel values (0xFFFF / 0xFFFFFFFF) for registers that are not available or supported on your specific system. The integration properly handles these and shows "unavailable" instead of incorrect or "raw" sentinel values.

## Board Diagnostics

The integration now includes board diagnostic capabilities to identify connected control boards:

### Board Information Sensors:
- **Number of Control Boards**: Shows how many boards are connected (typically 3)
- **Board 1/2/3 Device Types**: Shows device type codes for each board

### Device Type Codes:
Board types are displayed as hexadecimal codes (e.g., "Type: 0x1E08"):

| Code | Device Type |
|------|-------------|
| 0x00XX | CU-GH (Gas/Heat pump control unit) |
| 0x01XX | CU-OH (Oil/Heat pump control unit) |
| 0x02XX | EHC (Electric heating controller) |
| 0x14XX | MK (Mixing kit) |
| 0x19XX | SCB (System control board) |
| 0x1BXX | EEC (External expansion controller) |
| 0x1EXX | Gateway (GTW-08) |

**Example**: A board showing "Type: 0x1E08" indicates a GTW-08 Gateway (category 0x1E, number 08).

### Update Frequency:
Board diagnostic sensors update every 5 minutes to minimize Modbus traffic and improve system stability.

<details>
<summary><b>Click to expand System Status Codes (Register 411)</b></summary>

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
<summary><b>Click to expand Sub Status Codes (Register 412)</b></summary>

| Code | Sub Status |
|------|------------|
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

The RGB LED on AtomS3 Lite provides visual feedback based on Heat Demand select:

| Color | Mode |
|-------|------|
| 🔵 Blue (100%) | Heating |
| 🟦 Cyan (100%) | Cooling |
| 🟠 Orange (30%) | Standby |


## Supported Baxi Models

Compatibility depends on GTW-08 support from Baxi:

- ✅ Baxi Platinum BC Plus Monobloc 2
- ✅ UIMB Baxi Connect with GTW-08

Verify GTW-08 compatibility with your system's documentation.

## Performance & Timing

- **Modbus Polling:** Every 30 seconds (optimized for stability)
- **Board Diagnostics:** Every 5 minutes (reduced frequency for stability)
- **Baud Rate:** 9600 (fixed by GTW-08)
- **Response Time:** ~250ms per Modbus command
- **Command Throttle:** 1000ms between writes (increased for reliability)
- **WiFi Signal Update:** Every 60 seconds
- **Boot Protection Delay:** ~100ms to ensure modbus reads complete

**Optimization Notes:**
- Slower polling reduces Modbus errors and improves system stability
- Board diagnostic sensors use `skip_updates: 10` to minimize traffic
- Increased command throttle prevents "Gateway Path Unavailable" errors

## Known Limitations

- Single GTW-08 per system (cascade not supported)
- Status codes limited to GTW-08 manual definitions
- Energy counters not yet implemented (registers available)
- Zoning requires manual configuration per zone

## Roadmap

- [x] Energy counters (kWh consumption)
- [ ] Advanced diagnostics dashboard
- [ ] Multi-zone support
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
