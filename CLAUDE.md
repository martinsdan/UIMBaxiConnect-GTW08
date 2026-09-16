# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

ESPHome firmware (single YAML config) for an M5Stack AtomS3 Lite + Atomic RS485 Base that bridges a Baxi heat pump (via the Baxi GTW-08 Modbus gateway) into Home Assistant. There is no application source code — the entire integration is `baxi-heat-pump-atom.yaml`, a declarative ESPHome config compiled to firmware by the ESPHome toolchain.

## Commands

This repo has no build scripts, tests, or linters of its own — it's consumed by the ESPHome compiler/uploader.

- Validate config: `esphome config baxi-heat-pump-atom.yaml`
- Compile: `esphome compile baxi-heat-pump-atom.yaml`
- Compile + flash over USB: `esphome run baxi-heat-pump-atom.yaml`
- Requires a `secrets.yaml` (gitignored) alongside the YAML with `wifi_ssid`, `wifi_password`, `fallback_ap_password`, `api_key`, `ota_password`.
- In practice this config is normally deployed through Home Assistant's ESPHome add-on (paste YAML into a new device), not the local CLI — but the CLI commands above are the way to validate syntax/lambdas before proposing changes.

There's no automated test suite; changes are verified by reading ESPHome logs after flashing/OTA and checking values in Home Assistant.

## Architecture

Single file: `baxi-heat-pump-atom.yaml`, organized top to bottom as: board/wifi/OTA boilerplate → `uart`/`modbus`/`modbus_controller` → `sensor` → `text_sensor` → `binary_sensor` → `select` → `number`. Reference doc for the register map/protocol semantics lives in `docs/GTW-08-Modbus-Config.txt` (the GTW-08 manual, NOT-7854678) — consult it before adding or reinterpreting a register.

Every entity is a `platform: modbus_controller` entry addressed by GTW-08 register number (see README's register tables for the map: Device Info 1-12, Main Control 256-355, Boiler/Appliance 384-503, Zone Config 640-990 per zone (×12, 512 regs/zone), Zone Monitoring 1100-1120, Buffer Tank 7500+, Cascade 7000+, Thermodynamic WH 9001+, Hybrid 9200+, BMS 21020+). Only Zone 1 is wired up; additional zones would be added by offsetting register addresses +512 per zone.

Three GTW-08 quirks drive most of the non-obvious code here, and must be preserved when adding entities:

1. **Write function code**: the GTW-08 only accepts Modbus FC16 (Write Multiple Registers), even for single-register writes, and silently rejects ESPHome's default FC06. Every writable `select`/`number` entity must set `use_write_multiple: true`.
2. **Bulk-read section boundaries**: ESPHome auto-groups nearby registers into one bulk read; if a group spans a GTW-08 internal section boundary, the gateway returns Modbus Exception 3 (Illegal Data Value) for the whole read. `force_new_range: true` marks the first register of a new section (currently on 1100 and 9230) to force a fresh read group. When adding a sensor at a new address, check whether it falls in a different section than its YAML-adjacent neighbors and add `force_new_range: true` if so.
3. **Sentinel values**: unsupported/unavailable registers return `0xFFFF` (16-bit) or `0xFFFFFFFF` (32-bit), sometimes `0` depending on register. Sensors that can be absent on some models use a `lambda` filter that checks for the sentinel and `return {};` (drop the sample → HA shows "unavailable") instead of publishing the raw sentinel as a value. Follow this pattern for any new sensor pulling from a register that the README's "May Show Unavailable" list or the GTW-08 manual marks as optional/version-gated.

Other patterns used throughout:
- `boot_complete` global (set `true` in `on_boot` at priority -100) gates `on_value` side effects (e.g. status LED changes) so writes/actions don't fire from stale/default state during startup.
- Multi-bit/packed registers (e.g. board device type, status codes) are decoded via `lambda` into `text_sensor`/`select` values rather than exposed as raw ints — check `docs/GTW-08-Modbus-Config.txt` tables (status codes, sub-status codes, device type codes) before adding a new decoded enum.
- Numeric scaling (`multiply` filter) must match the register's documented resolution (e.g. register 413 power output is 0.0001 resolution, not a direct percentage) — verify resolution in the manual before assuming 1:1 or 0.01 scaling.
