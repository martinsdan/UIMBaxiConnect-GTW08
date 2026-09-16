#pragma once

#include <cstdint>
#include <string>

// Decodes a GTW-08 board device type register into a display string.
// High byte = device category, low byte = instance number within that
// category (0 = unnumbered). See README's "Board Diagnostics" table.
inline std::string decode_board_device_type(uint16_t device_type) {
  if (device_type == 0xFFFF || device_type == 0) {
    return "Not Available";
  }
  uint8_t category = (device_type >> 8) & 0xFF;
  uint8_t number = device_type & 0xFF;
  std::string suffix = number > 0 ? std::to_string(number) : "";
  switch (category) {
    case 0x00: return "CU-GH" + suffix;
    case 0x01: return "CU-OH" + suffix;
    case 0x02: return "EHC" + suffix;
    case 0x14: return "MK" + suffix;
    case 0x19: return "SCB" + suffix;
    case 0x1B: return "EEC" + suffix;
    case 0x1E: return "Gateway" + suffix;
    default: return "Unknown (0x" + std::to_string(device_type) + ")";
  }
}
