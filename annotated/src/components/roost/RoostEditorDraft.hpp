#pragma once
#include <cstdint>
#include "RoostCore.hpp"

namespace Roost {
// Pure editor state: 0 means Skip; no zero-duration phase reaches the motor.
// This does not change a running session until the user explicitly saves.
// [READER NOTE WQB038] EditorDraft is a temporary four-slot edit buffer; the user can set a slot to zero (Skip), cycle slots, and commit only a valid pattern.
class EditorDraft {
public:
  explicit EditorDraft(const uint8_t initial[4]) {
    for (uint8_t i = 0; i < 4; ++i) values_[i] = initial[i] <= 30 ? initial[i] : 0;
  }
  uint8_t SlotIndex() const { return slot_; }
  uint8_t Seconds() const { return values_[slot_]; }
  void Increase() { if (values_[slot_] < 30) ++values_[slot_]; }
  void Decrease() { if (values_[slot_] > 0) --values_[slot_]; }
  void Next() { slot_ = static_cast<uint8_t>((slot_ + 1) % 4); }
  void CopyTo(uint8_t out[4]) const { for (uint8_t i = 0; i < 4; ++i) out[i] = values_[i]; }
// [READER NOTE WQB038] Valid uses the same Normalize rules as runtime, so four skipped phases cannot become a running pattern.
  bool Valid() const { Pattern normalized {}; return Normalize(values_, normalized); }
private:
  uint8_t values_[4] {};
  uint8_t slot_ = 0;
};
} // namespace Roost
