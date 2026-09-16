# Baxi Heat Pump Modbus Integration (GTW-08)

[![ESPHome](https://img.shields.io/badge/ESPHome-2025.11.5-blue.svg)](https://esphome.io)
[![License](https://img.shields.io/github/license/martinsdan/UIMBaxiConnect-GTW08)](LICENSE)
[![ESP32-S3](https://img.shields.io/badge/ESP32-S3-orange)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)

ESPHome firmware for **M5Stack AtomS3 Lite + Atomic RS485 Base** to integrate **Baxi Heat Pumps** with **GTW-08 Modbus Gateway** into Home Assistant.

## Features

- Temperature Monitoring:
  - Outdoor Temperature (from GTW-08 sensor)
  - Flow & Return Temperatures (primary circuit)
  - HP Flow & Return Temperatures (heat pump circuit)
  - DHW / CH / Cooling Flow Setpoints
  - System Flow Setpoint (including backups)
  - Heating Curve Target (calculated: base + gradient * (20 - outdoor temp))
  - Zone 1 Room Temperature, Flow Temperature, Current Setpoint

- System Control (Selects):
  - Algorithm Type (Both Temp & Power / Power Only / Temperature Only / Monitoring Only)
  - Heat Demand (Standby / Heating / Cooling)
  - Zone 1 Operating Mode (Scheduling / Manual / Off)
  - Force Summer Mode (On/Off)
  - DHW Mode (Eco HP only / Comfort HP+Boiler)
  - Silent Mode (Off / Level 1-5)
  - Backup Type (No Backup / 1-Stage Electric / 2-Stage Electric / Boiler)
  - Cooling Type (Off / Active / Free)
  - Force Cooling (No/Yes)
  - CH Heat Demand Enable (On/Off)
  - DHW Heat Demand Enable (On/Off)
  - Hybrid Mode (No Hybrid / Cost / Primary Energy / CO2)

- System Control (Numbers):
  - Zone 1 Heating Curve Base & Gradient (adjustable)
  - Target Power (0-100%)
  - Target Flow Temperature (20-80C)
  - Target Room Temperature (10-30C)
  - Target Pump Speed (0-100%)
  - Heating Temperature Setpoint (remote, 0.01C resolution)
  - Cooling Temperature Setpoint (remote, 0.01C resolution)
  - Outdoor Temp Heating Limit (upper outdoor temp limit for heating)
  - Antifreeze Outdoor Temp (outdoor temp threshold for antifreeze)
  - DHW Comfort & Reduced Setpoints

- Monitoring & Diagnostics:
  - Water Pressure (bar)
  - Power Output (%) - correctly scaled (AM024, 0.0001 resolution)
  - COP - Instantaneous & Threshold
  - Power - Actual Output (kW)
  - Pump Speed (%) & Condenser Pump Speed
  - Flow Rate (l/min)
  - ODU Current Draw (A)
  - HP Defrost Active (binary sensor)
  - Low Noise Mode state
  - System Status (25+ operational states)
  - Sub Status (100+ detailed operation states)
  - Zone 1 Status: Heat Demand, Activity, Running Mode, Functioning Mode, Pump Status
  - Error Detection & Board Diagnostics
  - Energy Counters (Central Heating, DHW, Cooling, Total)
  - Seasonal Mode

- Status LED feedback with AtomS3 RGB LED:
  - Blue (100%): Heating Active
  - Cyan (100%): Cooling Active
  - Orange (30%): Standby

- Smart Data Handling:
  - Sentinel value detection (shows "unavailable" for unsupported registers)
  - Outlier filtering for temperature sensors
  - Range validation for all sensor values
  - Boot protection prevents spurious commands during startup
  - `reuse_previous_range: false` on section boundaries to prevent GTW-08 bulk read rejection
  - `use_write_multiple: true` on all writable entities (GTW-08 requires FC16)

- OTA Updates & Web Server for diagnostics
- WiFi Fallback AP for configuration
- Hardware Button (GPIO41) - Toggle status LED on press

## Critical: Modbus Write Configuration

The GTW-08 **only accepts Modbus function code 16 (0x10, Write Multiple Registers)** for all writes, regardless of data type. ESPHome defaults to function code 06 (Write Single Register) for single-register writes, which the GTW-08 silently rejects.

**All writable entities** (selects and numbers) have `use_write_multiple: true` set. **Do not remove this** or writes will fail without error.

Reference: GTW-08 Protocol Document (NOT-7854678), Section 5.5 - all data types specify write function code = 16d.

## Critical: Modbus Read Grouping

ESPHome groups nearby registers into single bulk reads for efficiency. The GTW-08 cannot handle bulk reads that span across its internal register section boundaries. `reuse_previous_range: false` is set on:
- Register 1100 (Zone 1 monitoring) - splits from Zone config section (640+)
- Register 9230 (COP/Hybrid) - splits from Zone monitoring section (1100+)

Without these, the GTW-08 returns **Modbus Exception 3 (Illegal Data Value)** and the registers fail to read.

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
     | (L-Bus)
     v
   GTW-08 (Modbus Gateway)
     | (X6 Connector)
     +- OV (GND) --> Atomic RS485 Base GND
     +- A (Data+) --> Atomic RS485 Base A
     +- B (Data-) --> Atomic RS485 Base B
     | (RS485)
     v
Atomic RS485 Base
     | (GPIO5/GPIO6 UART)
     v
AtomS3 Lite
     | (WiFi)
     v
Home Assistant
```

## GTW-08 Configuration

**Before connecting, set the GTW-08 dip switches:**

- **Rotary Dial:** Position 0 (Modbus Address 100 = 0x64)
- **DIP Switches 1-2:** OFF / OFF (9600 baud)
- **DIP Switches 3-4:** OFF / OFF (No parity)

## Installation

### Step 1: Prepare ESPHome

1. Open Home Assistant -> Settings -> Devices & Services -> ESPHome
2. Click **Create New Device** -> Select **ESP32-S3**
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
modbus:
  id: modbus1
  uart_id: modbus_uart
  turnaround_time: 1000ms  # Wait between requests (increased for reliability)

modbus_controller:
  - id: gtw08_controller
    address: 0x64           # GTW-08 default address
    modbus_id: modbus1
    setup_priority: 100     # Ensures reads before select evaluates
    update_interval: 30s    # Poll every 30 seconds (optimized for stability)
```

Board diagnostic registers (device type, board count) rarely change, so they're
polled by a second `modbus_controller` on the same bus at a slower interval
instead of every 30s.

## Home Assistant Integration

### Selects (Read-Write)

**System Control:**
| Select | Register | Options |
|--------|----------|---------|
| Algorithm Type | 258 | Both Temp & Power (0), Power Only (1), Temperature Only (2), Monitoring Only (3) |
| Heat Demand | 259 | Standby (0), Heating (7), Cooling (8) |
| Force Summer Mode | 389 | Off (0), On (1) |
| CH Heat Demand | 500 | Off (0), On (1) |
| DHW Heat Demand | 501 | Off (0), On (1) |
| Cooling Type | 502 | Off (0), Active Cooling (1), Free Cooling (2) |
| Force Cooling | 503 | No (0), Yes (1) |

**Zone Configuration:**
| Select | Register | Options |
|--------|----------|---------|
| Zone 1 Operating Mode | 649 | Scheduling (0), Manual (1), Off (2) |
| DHW Mode | 479 | Eco HP only (0), Comfort HP+Boiler (1) |
| Silent Mode | 490 | Off (0), Level 1-5 (1-5) |
| Backup Type | 482 | No Backup (0), 1-Stage Electric (1), 2-Stage Electric (2), Boiler (3) |
| Hybrid Mode | 464 | No Hybrid (0), Cost (1), Primary Energy (2), CO2 (3) |

### Number Controls (Read-Write)

| Control | Register | Range | Resolution |
|---------|----------|-------|------------|
| Heating Curve Base | 675 | 15-40 C | 0.1 C |
| Heating Curve Gradient | 674 | 0-2 | 0.1 |
| Target Power | 256 | 0-100% | 1% |
| Target Pump Speed | 459 | 0-100% | 0.1% |
| Target Flow Temperature | 648 | 20-80 C | 0.01 C |
| Target Room Temperature | 664 | 10-30 C | 0.1 C |
| Heating Temp Setpoint | 257 | 20-80 C | 0.01 C |
| Cooling Temp Setpoint | 260 | 5-30 C | 0.01 C |
| Outdoor Temp Heating Limit | 386 | -20 to 30.5 C | 0.01 C |
| Antifreeze Outdoor Temp | 388 | -20 to 10 C | 0.01 C |
| DHW Comfort Setpoint | 665 | 40-60 C | 0.01 C |
| DHW Reduced Setpoint | 666 | 40-60 C | 0.01 C |

### Sensors (Read-Only)

**Temperatures (C):**
- Outdoor Temperature (Register 384)
- Flow Temperature (Register 400)
- Return Temperature (Register 401)
- HP Flow Temperature (Register 403)
- HP Return Temperature (Register 404)
- DHW Internal Setpoint (Register 405)
- CH Setpoint (Register 406)
- Cooling Flow Setpoint (Register 407)
- DHW Flow Setpoint (Register 408)
- System Flow Setpoint incl. backups (Register 458)
- Heating Curve Target (calculated template)
- Zone 1 Flow Temperature (Register 1100)
- Zone 1 Current Setpoint (Register 1101)
- Zone 1 Room Temperature (Register 1104)

**Power & Performance:**
- Power Output % (Register 413) - UINT16, 0.0001 resolution
- Power - Actual Output kW (Register 460) - UINT32, 0.01 kW
- Pump Speed % (Register 459)
- Condenser Pump Speed % (Register 453)
- Flow Rate l/min (Register 410)
- ODU Current A (Register 456)
- Current System Power % (Register 272)

**COP:**
- COP - Instantaneous (Register 9230, 0.001 resolution)
- COP - Threshold (Register 463, 0.001 resolution)

**Energy Monitoring (kWh):**
- Energy - Central Heating (Register 433)
- Energy - Domestic Hot Water (Register 435)
- Energy - Cooling (Register 437)
- Energy - Total (Register 439)

**Status & Diagnostics:**
- System Status (Register 411, decoded text)
- Sub Status (Register 412, decoded text)
- HP Defrost Active (Register 481, binary sensor)
- Low Noise Mode (Register 480, decoded text)
- Seasonal Mode (Register 385)
- Error Flags (Register 277)
- Water Pressure bar (Register 409)
- Zone 1 Activity / Running Mode / Functioning Mode (Registers 1107-1109, decoded text)
- Zone 1 Heat Demand & Pump Status (Registers 1106, 1110, binary sensors)
- Board Information (Number of boards, device types)

**Network & Device:**
- WiFi Signal (dBm)
- Uptime (seconds)
- IP Address

## Data Availability

Some sensors may show **"unavailable"** depending on your specific heat pump model, system configuration, or operational state. This is normal behavior:

### Always Available:
- Temperature sensors (outdoor, flow, return, HP circuits)
- System status and sub-status
- Pump speed (when pump is running)
- Energy counters
- Error detection and diagnostics

### May Show "Unavailable":
- Water Pressure - Availability depends on system configuration
- Power Output & Actual Power - Many monobloc models do not report power data to the gateway
- COP - Instantaneous - Most models return a sentinel value indicating this is not supported
- Registers marked "Present from version 1.02/1.03" - Require GTW-08 firmware >= that version

**Note:** The GTW-08 gateway returns sentinel values (0xFFFF / 0xFFFFFFFF) for registers that are not available or supported on your specific system. The integration properly handles these and shows "unavailable" instead of incorrect or "raw" sentinel values.

## Register Map Reference

Key register sections from the GTW-08 Modbus mapping (NOT-7854678):

| Section | Register Range | Access |
|---------|---------------|--------|
| Device Information | 1-12 | R |
| System Discovery | 128-200 | R (200: R/W reset) |
| Main Control & Monitoring | 256-355 | R/W (256-260), R (272+) |
| Boiler/Appliance | 384-503 | R/W (386-389, 464-503), R (384-385, 400-463) |
| Service | 512-551 | R |
| Zone Config (x12, 512 regs each) | 640-990 | R/W |
| Zone Monitoring (Zone 1) | 1100-1120 | R (1105: R/W v1.01) |
| Buffer Tank | 7500-7606 | R/W (7500-7568), R (7600+) |
| Cascade | 7000-7146 | R/W (7000-7024), R (7100+) |
| Thermodynamic WH | 9001-9012 | R/W |
| Hybrid | 9200-9234 | R/W (9221-9234), R (9200-9220) |
| BMS | 21020-21030 | R/W (21020-21028, 21030), R (21029) |

## Board Diagnostics

The integration includes board diagnostic capabilities to identify connected control boards:

### Device Type Codes:

| Code | Device Type |
|------|-------------|
| 0x00XX | CU-GH (Gas/Heat pump control unit) |
| 0x01XX | CU-OH (Oil/Heat pump control unit) |
| 0x02XX | EHC (Electric heating controller) |
| 0x14XX | MK (Mixing kit / HMI) |
| 0x19XX | SCB (System control board) |
| 0x1BXX | EEC (External expansion controller) |
| 0x1EXX | Gateway (GTW-08) |

Board diagnostic sensors update every 5 minutes to minimize Modbus traffic.

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
| Blue (100%) | Heating |
| Cyan (100%) | Cooling |
| Orange (30%) | Standby |


## Supported Baxi Models

Compatibility depends on GTW-08 support from Baxi:

- Baxi Platinum BC Plus Monobloc 2
- UIM Baxi Connect with GTW-08

Verify GTW-08 compatibility with your system's documentation.

## Performance & Timing

- **Modbus Polling:** Every 30 seconds (optimized for stability)
- **Board Diagnostics:** Every 5 minutes (reduced frequency for stability)
- **Baud Rate:** 9600 (fixed by GTW-08)
- **Response Time:** ~250ms per Modbus command
- **Command Throttle:** 1000ms between writes (increased for reliability)
- **WiFi Signal Update:** Every 60 seconds
- **Boot Protection Delay:** ~100ms to ensure modbus reads complete

## Bug Fixes Applied

### Write failures (use_write_multiple)
All writable entities now have `use_write_multiple: true`. The GTW-08 only accepts Modbus function code 16 (Write Multiple Registers) for writes. ESPHome defaults to FC06 (Write Single Register), which the GTW-08 silently rejects.

### Power Output scaling (Register 413)
Was incorrectly treating the raw UINT16 value as a direct percentage (0-100). The GTW-08 uses 0.0001 resolution (raw 10000 = 100%). Fixed with proper `raw * 0.01` scaling and updated range validation.

### Bulk read rejection (reuse_previous_range)
ESPHome groups nearby registers into bulk reads. When reads span across GTW-08 section boundaries (e.g., Zone Config 640-990 + Zone Monitoring 1100+), the GTW-08 returns Exception 3 (Illegal Data Value). Fixed with `reuse_previous_range: false` on section boundaries (registers 1100 and 9230) - the modern replacement for ESPHome's now-deprecated `force_new_range`.

## Known Limitations

- Single GTW-08 per system (cascade not yet implemented)
- Some registers require GTW-08 firmware >= 1.02 or 1.03
- Zone monitoring registers only mapped for Zone 1 (add zones by offsetting +512 per zone)
- Register 12 (AP059) alternative Modbus mapping not exposed (default=0, Baxi mode)

## Roadmap

- [x] Energy counters (kWh consumption)
- [x] Full writable register set (DHW, cooling, silent mode, hybrid, etc.)
- [x] Zone 1 monitoring (room temp, activity, pump status)
- [x] HP defrost detection
- [x] COP threshold
- [ ] Multi-zone support (Zone 2-12 via register offset)
- [ ] Cascade support
- [ ] Buffer tank registers
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

- Bug Report: [Open an Issue](https://github.com/martinsdan/UIMBaxiConnect-GTW08/issues)
- Questions: GitHub Discussions
- Documentation: Check the Wiki

## License

This project is licensed under **GPL-3.0** - see [LICENSE](LICENSE) file for details.

## Acknowledgments

- **M5Stack** - AtomS3 Lite and Atomic RS485 Base
- **Baxi/Remeha** - GTW-08 Modbus Gateway documentation
- **ESPHome** - Microcontroller firmware platform
- **Home Assistant** - Home automation ecosystem
- **Community** - Testing, feedback, and contributions

## References

- [ESPHome Modbus Controller](https://esphome.io/components/modbus_controller/)
- [Baxi GTW-08 Manual (NOT-7730367)](https://www.baxi.pt/)
- [Baxi GTW-08 Protocol (NOT-7854678)](https://www.baxi.pt/)
- [M5Stack AtomS3 Lite](https://docs.m5stack.com/en/core/AtomS3%20Lite)
- [Home Assistant Modbus](https://www.home-assistant.io/integrations/modbus/)

---

**Made with ❤️ for the Home Assistant community**
