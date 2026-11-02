#pragma once
#include "FreeRTOS.h"
#include <functional>
struct FakeSemaphore { bool locked = false; };
using SemaphoreHandle_t = FakeSemaphore*;
inline std::function<void()> fakeBeforeSemaphoreTake;
inline SemaphoreHandle_t xSemaphoreCreateMutex() { return new FakeSemaphore; }
inline BaseType_t xSemaphoreTake(SemaphoreHandle_t m, TickType_t) {
  if (fakeBeforeSemaphoreTake) { auto cb=fakeBeforeSemaphoreTake; fakeBeforeSemaphoreTake=nullptr; cb(); }
  if (m == nullptr || m->locked) return pdFALSE;
  m->locked = true; return pdTRUE;
}
inline BaseType_t xSemaphoreGive(SemaphoreHandle_t m) {
  if (m == nullptr || !m->locked) return pdFALSE;
  m->locked = false; return pdTRUE;
}
