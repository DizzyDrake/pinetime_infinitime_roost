#pragma once
#include "FreeRTOS.h"
#include <functional>
inline std::function<void()> fakeBeforeGetTick;
inline TickType_t fakeTickCount=1234u;
inline TickType_t xTaskGetTickCount() {
  if (fakeBeforeGetTick) { auto cb=fakeBeforeGetTick; fakeBeforeGetTick=nullptr; cb(); }
  return fakeTickCount;
}
