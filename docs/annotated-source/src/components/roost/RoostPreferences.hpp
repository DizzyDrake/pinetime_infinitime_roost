#pragma once

#include "RoostCore.hpp"

namespace Roost {
// Encodes a 12-byte preference payload. RoostService wraps it in an
// 18-byte generation+CRC record stored as alternating /roost-a.dat and /roost-b.dat.
// Decode validates the entire record; callers fall back to Box on error.
// Filesystem write/atomic replacement and version policy belong to the native adapter.
// [READER NOTE WQB038] Preferences stores selection, optional four custom seconds and availability. The encoded payload is 12 bytes; the service wraps it in an 18-byte generation+CRC record.
struct Preferences {
  Selection selected = Selection::Box;
  uint8_t custom[4] = {4, 4, 4, 4};
  bool hasCustom = false;
};

// [READER NOTE WQB038] Encode serializes a versioned ROOS signature, selection, four durations and XOR checksum; it does not itself write flash.
inline void Encode(const Preferences& value, uint8_t bytes[12]) {
  bytes[0] = 'R'; bytes[1] = 'O'; bytes[2] = 'O'; bytes[3] = 'S';
  bytes[4] = 1; // file format version
  bytes[5] = static_cast<uint8_t>(value.selected);
  bytes[6] = value.hasCustom ? 1 : 0;
  for (uint8_t i = 0; i < 4; ++i) bytes[7 + i] = value.custom[i];
  uint8_t check = 0;
  for (uint8_t i = 0; i < 11; ++i) check ^= bytes[i];
  bytes[11] = check;
}

// [READER NOTE WQB038] Decode validates magic/version/range/checksum and a runnable custom pattern before accepting saved state.
inline bool Decode(const uint8_t bytes[12], Preferences& out) {
  if (bytes[0] != 'R' || bytes[1] != 'O' || bytes[2] != 'O' || bytes[3] != 'S' || bytes[4] != 1 ||
      bytes[5] > static_cast<uint8_t>(Selection::Custom) || bytes[6] > 1) return false;
  uint8_t check = 0;
  for (uint8_t i = 0; i < 11; ++i) check ^= bytes[i];
  if (check != bytes[11]) return false;
  uint8_t slots[4] = {bytes[7], bytes[8], bytes[9], bytes[10]};
  Pattern normalized {};
  if (bytes[6] && !Normalize(slots, normalized)) return false;
  if (bytes[5] == static_cast<uint8_t>(Selection::Custom) && !bytes[6]) return false;
  Preferences next {};
  next.selected = static_cast<Selection>(bytes[5]);
  next.hasCustom = bytes[6] != 0;
  for (uint8_t i = 0; i < 4; ++i) next.custom[i] = slots[i];
  out = next;
  return true;
}

// [READER NOTE WQB038] Apply restores preferences into Session without starting haptics when the user reopens the app.
inline void Apply(const Preferences& preferences, Session& session) {
  if (preferences.hasCustom) session.SaveCustom(preferences.custom);
  session.Select(preferences.selected);
  // Apply restores the selected pattern, never a running session.
}
} // namespace Roost
