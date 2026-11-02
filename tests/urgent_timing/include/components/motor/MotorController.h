#pragma once
#include <functional>
namespace Pinetime { namespace Controllers {
class MotorController {
public:
  std::function<void()> beforeIsRinging;
  bool ringing = false;
  int pulses = 0;
  bool IsRinging() {
    if (beforeIsRinging) { auto cb=beforeIsRinging; beforeIsRinging=nullptr; cb(); }
    return ringing;
  }
  void RunForDuration(int) { ++pulses; }
};
}}
