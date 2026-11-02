#pragma once

#include <cstdint>

// Portable Roost logic for the pinned InfiniTime 1.16.0 integration candidate.
// No LVGL, motor, filesystem, FreeRTOS, or global mutable state lives here.
namespace Roost {

enum class Phase : uint8_t { Inhale, HoldAfterInhale, Exhale, HoldAfterExhale };
enum class Selection : uint8_t { Box, FourSevenEight, TwoEight, Custom };
enum class EndReason : uint8_t { None, Manual, IncomingCall, WatchAlarm };

struct Slot {
  Phase phase;
  uint8_t seconds;
};

struct Pattern {
  Slot slots[4] {};
  uint8_t count = 0;
  uint16_t cycleSeconds = 0;
};

// Zero is the editor's Skip value. Only positive slots reach the scheduler.
inline bool Normalize(const uint8_t seconds[4], Pattern& out) {
  Pattern next {};
  constexpr Phase phases[4] = {Phase::Inhale, Phase::HoldAfterInhale,
                                Phase::Exhale, Phase::HoldAfterExhale};
  for (uint8_t i = 0; i < 4; ++i) {
    if (seconds[i] > 30) return false;
    if (seconds[i] == 0) continue;
    next.slots[next.count++] = {phases[i], seconds[i]};
    next.cycleSeconds = static_cast<uint16_t>(next.cycleSeconds + seconds[i]);
  }
  if (next.count == 0) return false;
  out = next;  // Invalid edits leave the previous runnable pattern intact.
  return true;
}

inline Pattern Preset(Selection selected) {
  Pattern result {};
  constexpr uint8_t box[4] = {4, 4, 4, 4};
  constexpr uint8_t fourSevenEight[4] = {4, 7, 8, 0};
  constexpr uint8_t twoEight[4] = {2, 0, 8, 0};
  if (selected == Selection::Box) Normalize(box, result);
  else if (selected == Selection::FourSevenEight) Normalize(fourSevenEight, result);
  else Normalize(twoEight, result);
  return result;
}

struct View {
  Phase phase = Phase::Inhale;
  uint8_t slot = 0;
  uint32_t phaseElapsedMs = 0;
  uint32_t phaseDurationMs = 0;
  bool running = false;
};

struct CueRequest {
  uint32_t generation = 0;
  Phase phase = Phase::Inhale;
  bool due = false;
};

class Session {
 public:
  Session() : pattern_(Preset(Selection::Box)) {}

  Selection Selected() const { return selected_; }
  bool Running() const { return running_; }
  EndReason LastEnd() const { return lastEnd_; }
  uint32_t Generation() const { return generation_; }
  const Pattern& CurrentPattern() const { return pattern_; }

  // A selection change ends the old cycle; the caller must explicitly Start.
  bool Select(Selection value) {
    if (value == Selection::Custom && !hasCustom_) return false;
    if (value > Selection::Custom) return false;
    End(EndReason::Manual);
    selected_ = value;
    pattern_ = value == Selection::Custom ? custom_ : Preset(value);
    return true;
  }

  bool SaveCustom(const uint8_t seconds[4]) {
    Pattern next {};
    if (!Normalize(seconds, next)) return false;
    custom_ = next;
    hasCustom_ = true;
    if (selected_ == Selection::Custom) {
      End(EndReason::Manual);
      pattern_ = custom_;
    }
    return true;
  }

  void Start(uint32_t nowMs) {
    ++generation_;
    startMs_ = nowMs;
    lastBoundary_ = 0;
    running_ = true;
    lastEnd_ = EndReason::None;
  }

  void End(EndReason reason) {
    ++generation_;  // Invalidate requests already queued by timer callbacks.
    running_ = false;
    lastEnd_ = reason;
  }

  View Observe(uint32_t nowMs) const {
    View view {};
    if (!running_) return view;
    const uint32_t position = (nowMs - startMs_) % (pattern_.cycleSeconds * 1000u);
    uint32_t boundary = 0;
    for (uint8_t i = 0; i < pattern_.count; ++i) {
      const uint32_t duration = pattern_.slots[i].seconds * 1000u;
      if (position < boundary + duration) {
        view.phase = pattern_.slots[i].phase;
        view.slot = i;
        view.phaseElapsedMs = position - boundary;
        view.phaseDurationMs = duration;
        view.running = true;
        return view;
      }
      boundary += duration;
    }
    return view; // Unreachable for valid normalized patterns.
  }

  // Called by one serial session caller at a time. Late polls skip stale boundaries;
  // they never replay a burst of motor cues. Caller checks Authorize immediately
  // before handing one request to the shared motor, after alert priority checks.
  CueRequest Poll(uint32_t nowMs) {
    CueRequest request {};
    if (!running_) return request;
    const uint32_t elapsed = nowMs - startMs_;
    const uint32_t cycleMs = pattern_.cycleSeconds * 1000u;
    const uint32_t cycle = elapsed / cycleMs;
    const uint32_t position = elapsed % cycleMs;
    uint32_t boundaryInCycle = 0;
    uint8_t index = 0;
    for (uint8_t i = 0; i < pattern_.count; ++i) {
      if (position >= boundaryInCycle) index = i;
      boundaryInCycle += pattern_.slots[i].seconds * 1000u;
    }
    // Ordinal 0 means initial phase; first cue timing remains an adapter choice.
    const uint32_t ordinal = cycle * pattern_.count + index;
    if (ordinal > lastBoundary_) {
      lastBoundary_ = ordinal;
      request = {generation_, pattern_.slots[index].phase, true};
    }
    return request;
  }

  bool Authorize(const CueRequest& request) const {
    return request.due && running_ && request.generation == generation_;
  }

 private:
  Pattern pattern_;
  Pattern custom_ {};
  Selection selected_ = Selection::Box;
  EndReason lastEnd_ = EndReason::None;
  uint32_t generation_ = 0;
  uint32_t startMs_ = 0;
  uint32_t lastBoundary_ = 0;
  bool running_ = false;
  bool hasCustom_ = false;
};

} // namespace Roost
