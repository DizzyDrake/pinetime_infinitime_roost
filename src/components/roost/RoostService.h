#pragma once

#include <FreeRTOS.h>
#include <semphr.h>
#include <timers.h>
#include <atomic>
#include "components/roost/RoostCore.hpp"
#include "components/roost/RoostPreferences.hpp"

namespace Pinetime {
  namespace Controllers {
    class MotorController;
    class FS;

    // One service belongs to DisplayApp, independent of any LVGL screen.
    // Timer and alert callbacks serialize session changes through a mutex.
    class RoostService {
    public:
      RoostService(MotorController& motor, FS& filesystem) : motor {motor}, filesystem {filesystem} {}
      bool Init();
      bool Start();
      void Stop();
      void EndForCall();
      void EndForAlarm();
      bool Select(Roost::Selection selection);
      bool SaveCustom(const uint8_t seconds[4]);
      void GetCustomEditValues(uint8_t seconds[4]);
      bool HasSavedCustom();
      Roost::Selection Selected();
      bool Running();
      Roost::EndReason LastEnd();
      Roost::View Observe();

    private:
      static void TimerCallback(TimerHandle_t timer);
      void OnTimer();
      bool ArmNext(uint32_t nowMs);
      void End(Roost::EndReason reason);
      void MarkUrgent(Roost::EndReason reason);
      static uint32_t NowMs();
      bool ReadSlot(const char* path, uint32_t& generation, Roost::Preferences& value);
      bool WriteSelection();
      void LoadSelection();

      MotorController& motor;
      FS& filesystem;
      Roost::Session session {};
      Roost::Preferences preferences {};
      uint32_t preferenceGeneration = 0;
      uint8_t preferenceSlot = 1; // First save goes to slot 0.
      SemaphoreHandle_t mutex = nullptr;
      TimerHandle_t timer = nullptr;
      bool alertPending = false;
      std::atomic<uint8_t> urgentReason {0};
      std::atomic<uint32_t> urgentEpoch {0};
      uint32_t sessionEpoch = 0;
    };
  }
}
