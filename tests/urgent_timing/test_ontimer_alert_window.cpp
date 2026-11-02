#include "components/roost/RoostService.h"
#include "components/motor/MotorController.h"
#include "components/fs/FS.h"
#include "timers.h"
#include "task.h"
#include <iostream>
#include <cstdlib>
using namespace Pinetime::Controllers;
static int test(int scenario, bool alarm) {
  fakeTickCount=1234;
  fakeBeforeTimerChange=nullptr;
  MotorController motor; FS fs; RoostService service(motor,fs);
  if(!service.Init() || !service.Start()) return 40;
  fakeTickCount += 5000;
  auto ingress=[&] { if(alarm) service.EndForAlarm(); else service.EndForCall(); };
  if(scenario==0) motor.beforeIsRinging=ingress;
  if(scenario==1) fakeBeforeTimerChange=ingress;
  if(!fakeLastTimer || !fakeLastTimer->armed) return 41;
  fakeLastTimer->callback(fakeLastTimer);
  const auto expected=alarm?Roost::EndReason::WatchAlarm:Roost::EndReason::IncomingCall;
  std::cout << "scenario=" << scenario << " alarm=" << alarm
    << " pulses=" << motor.pulses << " running=" << service.Running()
    << " end=" << static_cast<int>(service.LastEnd()) << '\n';
  if(scenario==0 && motor.pulses) return 10;
  if(service.Running() || service.LastEnd()!=expected) return 20;
  return 0;
}
int main(int argc, char** argv) {
  if(argc==3) return test(std::atoi(argv[1]), std::atoi(argv[2])!=0);
  for(int s=0;s<2;++s) for(int a=0;a<2;++a) {
    int result=test(s,a);
    if(result) { std::cerr<<"FAIL: "<<result<<"\n"; return result; }
  }
  // Control: ordinary boundary still pulses and remains running.
  fakeTickCount=1234; fakeBeforeTimerChange=nullptr;
  MotorController motor; FS fs; RoostService service(motor,fs);
  if(!service.Init() || !service.Start()) return 40;
  fakeTickCount+=5000;
  fakeLastTimer->callback(fakeLastTimer);
  if(motor.pulses!=1 || !service.Running()) return 50;
  std::cout << "PASS: four urgent OnTimer windows and normal cue control\n";
}
