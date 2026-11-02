#include "components/roost/RoostService.h"
#include "components/motor/MotorController.h"
#include "components/fs/FS.h"
#include <task.h>

using Pinetime::Controllers::RoostService;

namespace {
  uint16_t RecordCrc(const uint8_t* bytes, uint8_t count) {
    uint16_t crc = 0xffff;
    for (uint8_t i = 0; i < count; ++i) {
      crc ^= static_cast<uint16_t>(bytes[i]) << 8u;
      for (uint8_t bit = 0; bit < 8; ++bit)
        crc = static_cast<uint16_t>((crc << 1u) ^ ((crc & 0x8000u) ? 0x1021u : 0u));
    }
    return crc;
  }
}

// [READER NOTE WQB038] Tick-to-ms conversion assumes configTICK_RATE_HZ from pinned firmware; test if rebasing to a different scheduler tick rate.
uint32_t RoostService::NowMs() {
  // Source tick rate is 1024 Hz; this conversion has sub-ms quantization.
  // Revisit this conversion if configTICK_RATE_HZ or clock width changes.
  return static_cast<uint32_t>(static_cast<uint64_t>(xTaskGetTickCount()) * 1000u / configTICK_RATE_HZ);
}

// [READER NOTE WQB038] Init creates the service mutex and one-shot timer and restores saved selection; it does not auto-start breathing.
bool RoostService::Init() {
  mutex = xSemaphoreCreateMutex();
  if (!mutex) return false;
  timer = xTimerCreate("Roost", 1, pdFALSE, this, TimerCallback);
  if (timer) LoadSelection();
  return timer != nullptr;
}

// [READER NOTE WQB038] ArmNext schedules the next phase boundary in FreeRTOS, independent of the screen refresh rate and sleep state.
bool RoostService::ArmNext(uint32_t nowMs) {
  const auto view = session.Observe(nowMs);
  if (!view.running || !timer) return false;
  const uint32_t remaining = view.phaseDurationMs - view.phaseElapsedMs;
  TickType_t delay = pdMS_TO_TICKS(remaining);
  if (delay == 0) delay = 1;
  return xTimerChangePeriod(timer, delay, 0) == pdPASS;
}

// [READER NOTE WQB038] Start samples urgent-event state before taking the mutex and checks it again around startup and timer rearm to avoid losing an incoming call/alarm.
// [READER NOTE RC2] An alert can arrive before Start takes its mutex. Snapshot the urgent epoch at function entry; recheck after scheduling the timer to avoid missing or overwriting a call/alarm.
bool RoostService::Start() {
  // Capture at function entry, BEFORE taking the mutex or checking the motor.
  // MarkUrgent may increment this epoch while we own the mutex; capturing it
  // afterward would incorrectly treat that unhandled alert as already seen.
  const uint32_t entryEpoch = urgentEpoch.load();
  if (!mutex || !timer || xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) return false;
  if (motor.IsRinging()) { xSemaphoreGive(mutex); return false; }
  const uint32_t now = NowMs();
  session.Start(now);
  alertPending = false;
  sessionEpoch = entryEpoch;
  if (urgentEpoch.load() != entryEpoch) {
    // MarkUrgent could not acquire our lock. Finish the terminal transition
    // here, without scheduling a phase cue or overwriting the alert retry.
    alertPending = true;
    session.End(static_cast<Roost::EndReason>(urgentReason.load()));
    xTimerStop(timer, 0);
    xSemaphoreGive(mutex);
    return false;
  }
  const bool armed = ArmNext(now);
  // ArmNext may overlap a queued urgent retry: its longer timer period could
  // otherwise replace the urgent 1-tick retry while our mutex is held.
  if (urgentEpoch.load() != entryEpoch) {
    alertPending = true;
    session.End(static_cast<Roost::EndReason>(urgentReason.load()));
    xTimerStop(timer, 0);
    xSemaphoreGive(mutex);
    return false;
  }
  if (!armed) session.End(Roost::EndReason::Manual); // Avoid active-but-silent state.
  xSemaphoreGive(mutex);
  return armed;
}

// [READER NOTE WQB038] End invalidates the current session and cancels its timer, but deliberately never stops another alert's motor vibration.
void RoostService::End(Roost::EndReason reason) {
  if (!mutex || xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) return;
  alertPending = reason == Roost::EndReason::IncomingCall || reason == Roost::EndReason::WatchAlarm;
  session.End(reason); // Invalidates queued cue generation before timer cancellation.
  if (timer) xTimerStop(timer, 0);
  xSemaphoreGive(mutex);
  // Do not StopRinging: the shared motor may already belong to an alert.
}

void RoostService::Stop() { End(Roost::EndReason::Manual); }
// [READER NOTE WQB038] Call/alarm ingress increments an atomic epoch immediately. If the mutex is busy, the timer retries the terminal transition.
void RoostService::MarkUrgent(Roost::EndReason reason) {
  // This ingress flag closes the queue-latency window before the normal
  // notification/alarm display path. Do not block the FreeRTOS timer daemon.
  urgentReason.store(static_cast<uint8_t>(reason));
  urgentEpoch.fetch_add(1);
  if (!mutex || xSemaphoreTake(mutex, 0) != pdTRUE) {
    if (timer) xTimerChangePeriod(timer, 1, 0); // Retry terminal state on timer task.
    return;
  }
  alertPending = true;
  session.End(reason);
  if (timer) xTimerStop(timer, 0);
  xSemaphoreGive(mutex);
}
void RoostService::EndForCall() { MarkUrgent(Roost::EndReason::IncomingCall); }
void RoostService::EndForAlarm() { MarkUrgent(Roost::EndReason::WatchAlarm); }

// [READER NOTE WQB038] Select stops the old session and persists the user's deliberate pattern selection, not every animation frame.
bool RoostService::Select(Roost::Selection selection) {
  if (!mutex || xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) return false;
  const bool selected = session.Select(selection); // Selection ends the old cycle.
  if (selected && timer) xTimerStop(timer, 0);
  if (selected) preferences.selected = selection;
  xSemaphoreGive(mutex);
  if (selected) WriteSelection(); // Only a deliberate selection writes flash.
  return selected;
}

// [READER NOTE WQB038] SaveCustom validates and persists the user's custom slots only on explicit editor commit.
bool RoostService::SaveCustom(const uint8_t seconds[4]) {
  if (!mutex || xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) return false;
  const bool saved = session.SaveCustom(seconds);
  if (saved) {
    for (uint8_t i = 0; i < 4; ++i) preferences.custom[i] = seconds[i];
    preferences.hasCustom = true;
    if (preferences.selected == Roost::Selection::Custom && timer) xTimerStop(timer, 0);
  }
  xSemaphoreGive(mutex);
  if (saved) WriteSelection(); // Save occurs on explicit editor commit, not animation.
  return saved;
}

// [READER NOTE WQB038] The editor gets four conceptual phase slots, restoring zeros for skipped phases; normalized runtime slots are compacted.
void RoostService::GetCustomEditValues(uint8_t seconds[4]) {
  if (!seconds) return;
  if (!mutex || xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) {
    const uint8_t fallback[4] {4, 4, 4, 4};
    for (uint8_t i = 0; i < 4; ++i) seconds[i] = fallback[i];
    return;
  }
  if (preferences.hasCustom) {
    for (uint8_t i = 0; i < 4; ++i) seconds[i] = preferences.custom[i];
  } else {
    const auto preset = Roost::Preset(session.Selected());
    // Normalize() omits Skipped slots, so recover the four conceptual slots.
    for (uint8_t i = 0; i < 4; ++i) seconds[i] = 0;
    for (uint8_t i = 0; i < preset.count; ++i) {
      seconds[static_cast<uint8_t>(preset.slots[i].phase)] = preset.slots[i].seconds;
    }
  }
  xSemaphoreGive(mutex);
}

bool RoostService::HasSavedCustom() {
  if (!mutex || xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) return false;
  const bool available = preferences.hasCustom;
  xSemaphoreGive(mutex);
  return available;
}

// [READER NOTE WQB038] ReadSlot checks a fixed 18-byte record, CRC and decoded payload before considering persisted state valid.
bool RoostService::ReadSlot(const char* path, uint32_t& generation, Roost::Preferences& value) {
  uint8_t bytes[18] {};
  lfs_file_t file {};
  if (filesystem.FileOpen(&file, path, LFS_O_RDONLY) != LFS_ERR_OK) return false;
  const int read = filesystem.FileRead(&file, bytes, sizeof(bytes));
  const int closed = filesystem.FileClose(&file);
  if (read != static_cast<int>(sizeof(bytes)) || closed != LFS_ERR_OK) return false;
  const uint16_t stored = static_cast<uint16_t>(bytes[16]) | (static_cast<uint16_t>(bytes[17]) << 8u);
  if (stored != RecordCrc(bytes, 16) || !Roost::Decode(bytes + 4, value)) return false;
  generation = static_cast<uint32_t>(bytes[0]) | (static_cast<uint32_t>(bytes[1]) << 8u) |
               (static_cast<uint32_t>(bytes[2]) << 16u) | (static_cast<uint32_t>(bytes[3]) << 24u);
  return true;
}

// [READER NOTE WQB038] LoadSelection compares two alternating LittleFS records and restores the newest valid generation after interrupted writes.
void RoostService::LoadSelection() {
  uint32_t aGeneration = 0, bGeneration = 0;
  Roost::Preferences a {}, b {};
  const bool validA = ReadSlot("/roost-a.dat", aGeneration, a);
  const bool validB = ReadSlot("/roost-b.dat", bGeneration, b);
  if (!validA && !validB) return; // Default Box; no automatic session start.
  const bool chooseA = validA && (!validB || static_cast<int32_t>(aGeneration - bGeneration) > 0);
  preferences = chooseA ? a : b;
  preferenceGeneration = chooseA ? aGeneration : bGeneration;
  preferenceSlot = chooseA ? 0 : 1;
  Roost::Apply(preferences, session);
}

// [READER NOTE WQB038] WriteSelection alternates files /roost-a.dat and /roost-b.dat; do not simplify to one overwrite without studying torn-write recovery.
bool RoostService::WriteSelection() {
  // Alternate two fixed-size records. A torn write leaves the prior valid
  // slot available on reboot; a failed write does not advance the slot.
  uint8_t bytes[18] {};
  const uint32_t next = preferenceGeneration + 1;
  for (uint8_t i = 0; i < 4; ++i) bytes[i] = static_cast<uint8_t>(next >> (i * 8u));
  Roost::Encode(preferences, bytes + 4);
  const uint16_t crc = RecordCrc(bytes, 16);
  bytes[16] = static_cast<uint8_t>(crc);
  bytes[17] = static_cast<uint8_t>(crc >> 8u);
  const uint8_t nextSlot = preferenceSlot == 0 ? 1 : 0;
  const char* path = nextSlot == 0 ? "/roost-a.dat" : "/roost-b.dat";
  lfs_file_t file {};
  if (filesystem.FileOpen(&file, path, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC) != LFS_ERR_OK) return false;
  const int written = filesystem.FileWrite(&file, bytes, sizeof(bytes));
  const int closed = filesystem.FileClose(&file);
  if (written != static_cast<int>(sizeof(bytes)) || closed != LFS_ERR_OK) return false;
  preferenceGeneration = next;
  preferenceSlot = nextSlot;
  return true;
}

Roost::Selection RoostService::Selected() {
  if (!mutex || xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) return Roost::Selection::Box;
  const auto value = session.Selected();
  xSemaphoreGive(mutex);
  return value;
}

bool RoostService::Running() {
  if (!mutex || xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) return false;
  const bool value = session.Running();
  xSemaphoreGive(mutex);
  return value;
}

Roost::EndReason RoostService::LastEnd() {
  if (!mutex || xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) return Roost::EndReason::None;
  const auto reason = session.LastEnd();
  xSemaphoreGive(mutex);
  return reason;
}

Roost::View RoostService::Observe() {
  if (!mutex || xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) return {};
  const auto view = session.Observe(NowMs());
  xSemaphoreGive(mutex);
  return view;
}

void RoostService::TimerCallback(TimerHandle_t timer) {
  auto* service = static_cast<RoostService*>(pvTimerGetTimerID(timer));
  service->OnTimer();
}

// [READER NOTE WQB038] Timer callbacks run outside LVGL, serialize with the mutex, check urgent epochs and motor ownership, then issue short boundary pulses.
// [READER NOTE RC2] Timer callbacks recheck urgent events around motor and timer operations; the final check-to-pulse gap is not atomic because motor arbitration is shared with upstream InfiniTime.
void RoostService::OnTimer() {
  // Timer callbacks execute in the FreeRTOS timer daemon, outside LVGL.
  if (!mutex || xSemaphoreTake(mutex, 0) != pdTRUE) {
    // Never block the timer daemon waiting for a display/alert task.
    if (timer) xTimerChangePeriod(timer, 1, 0);
    return;
  }
  const uint32_t now = NowMs();
  if (urgentEpoch.load() != sessionEpoch) {
    const uint8_t reason = urgentReason.load();
    alertPending = true;
    session.End(static_cast<Roost::EndReason>(reason));
    xSemaphoreGive(mutex);
    return;
  }
  const auto request = session.Poll(now);
  if (request.due && session.Authorize(request) && !alertPending && urgentEpoch.load() == sessionEpoch) {
    // IsRinging() is an external call. An urgent event may arrive while it
    // runs, after the first epoch guard but before the phase cue.
    const bool ringing = motor.IsRinging();
    if (!ringing && urgentEpoch.load() == sessionEpoch) {
      // Provisional first-trial cue: one short pulse at each phase boundary.
      motor.RunForDuration(30);
    }
  }
  // MarkUrgent can run while this callback owns the mutex, including inside
  // IsRinging() or ArmNext(). End before rearming, then check once more after
  // the normal timer change to avoid overwriting the urgent one-tick retry.
  if (urgentEpoch.load() != sessionEpoch) {
    alertPending = true;
    session.End(static_cast<Roost::EndReason>(urgentReason.load()));
    xTimerStop(timer, 0);
  } else {
    if (session.Running() && !ArmNext(now)) session.End(Roost::EndReason::Manual);
    if (urgentEpoch.load() != sessionEpoch) {
      alertPending = true;
      session.End(static_cast<Roost::EndReason>(urgentReason.load()));
      xTimerStop(timer, 0);
    }
  }
  xSemaphoreGive(mutex);
}
