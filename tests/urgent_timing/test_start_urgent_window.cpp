#include "components/roost/RoostService.h"
#include "components/motor/MotorController.h"
#include "components/fs/FS.h"
#include "task.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
using namespace Pinetime::Controllers;

static void CheckIngress(int scenario, bool alarm) {
  MotorController motor; FS fs; RoostService service(motor,fs);
  assert(service.Init());
  auto ingress=[&] { if (alarm) service.EndForAlarm(); else service.EndForCall(); };
  if (scenario==0) fakeBeforeSemaphoreTake=ingress; // Before mutex acquisition.
  if (scenario==1) motor.beforeIsRinging=ingress; // Inside locked IsRinging check.
  if (scenario==2) fakeBeforeGetTick=ingress; // Inside locked NowMs call.
  if (scenario==3) fakeBeforeTimerChange=ingress; // Inside locked ArmNext, after first epoch check.
  bool started=service.Start();
  const auto expected=alarm ? Roost::EndReason::WatchAlarm : Roost::EndReason::IncomingCall;
  std::cout << "scenario=" << scenario << " alarm=" << alarm << " started=" << started
            << " running=" << service.Running()
            << " lastEnd=" << static_cast<int>(service.LastEnd()) << '\n';
  if (started || service.Running() || service.LastEnd()!=expected) {
    std::cerr << "FAIL: urgent ingress lost during Start() window\n";
    std::exit(10+scenario);
  }
}

int main() {
  for (int scenario=0; scenario<4; ++scenario) {
    CheckIngress(scenario,false);
    CheckIngress(scenario,true);
  }
  // Control: a past completed alert is not an implicit hold on a new Start.
  MotorController motor; FS fs; RoostService service(motor,fs);
  assert(service.Init());
  service.EndForCall();
  assert(service.Start());
  assert(service.Running());
  service.EndForAlarm();
  assert(!service.Running() && service.LastEnd()==Roost::EndReason::WatchAlarm);
  std::cout << "PASS: eight ingress windows and past-alert/post-start controls\n";
  return 0;
}
