#pragma once
#include "FreeRTOS.h"
#include <functional>
struct FakeTimer;
using TimerHandle_t = FakeTimer*;
using TimerCallbackFunction_t = void (*)(TimerHandle_t);
struct FakeTimer { void* id; TimerCallbackFunction_t callback; bool armed = false; TickType_t period=0; };
inline std::function<void()> fakeBeforeTimerChange;
inline TimerHandle_t fakeLastTimer=nullptr;
inline TimerHandle_t xTimerCreate(const char*, TickType_t, BaseType_t, void* id, TimerCallbackFunction_t cb) {
  fakeLastTimer=new FakeTimer{id,cb,false,0}; return fakeLastTimer;
}
inline BaseType_t xTimerChangePeriod(TimerHandle_t t, TickType_t delay, TickType_t) {
  if (!t) return pdFALSE;
  if (fakeBeforeTimerChange && delay > 1) { auto cb=fakeBeforeTimerChange; fakeBeforeTimerChange=nullptr; cb(); }
  t->period=delay;
  t->armed=true;
  return pdPASS;
}
inline BaseType_t xTimerStop(TimerHandle_t t, TickType_t) {
  if (!t) return pdFALSE;
  t->armed=false;
  return pdPASS;
}
inline void* pvTimerGetTimerID(TimerHandle_t t) { return t->id; }
