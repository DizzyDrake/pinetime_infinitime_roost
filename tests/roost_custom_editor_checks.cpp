#include "src/components/roost/RoostEditorDraft.hpp"
#include "src/components/roost/RoostPreferences.hpp"
#include <cassert>
#include <iostream>
int main() {
  uint8_t seed[4] {4, 4, 4, 4};
  Roost::EditorDraft d(seed);
  assert(d.Valid() && d.Seconds() == 4);
  for (int i=0;i<50;i++) d.Increase();
  assert(d.Seconds()==30);
  for (int i=0;i<50;i++) d.Decrease();
  assert(d.Seconds()==0);
  assert(d.Valid()); // Other phases remain active.
  d.Next(); assert(d.SlotIndex()==1);
  d.Next(); d.Next(); d.Next(); assert(d.SlotIndex()==0);
  uint8_t allSkip[4] {0,0,0,0};
  Roost::EditorDraft empty(allSkip);
  assert(!empty.Valid());
  empty.Increase(); assert(empty.Valid());
  uint8_t copy[4]{}; empty.CopyTo(copy);
  assert(copy[0]==1 && copy[1]==0 && copy[2]==0 && copy[3]==0);
  Roost::Pattern normalized {};
  uint8_t twoPhase[4]{4,0,8,0};
  assert(Roost::Normalize(twoPhase, normalized) && normalized.count==2 && normalized.cycleSeconds==12);
  assert(normalized.slots[0].phase==Roost::Phase::Inhale && normalized.slots[1].phase==Roost::Phase::Exhale);
  Roost::Preferences prefs {};
  prefs.hasCustom=true; prefs.selected=Roost::Selection::Custom;
  for (int i=0;i<4;i++) prefs.custom[i]=twoPhase[i];
  uint8_t bytes[12]{}; Roost::Encode(prefs,bytes);
  Roost::Preferences restored {};
  assert(Roost::Decode(bytes,restored) && restored.selected==Roost::Selection::Custom && restored.custom[2]==8);
  std::cout << "PASS editor draft, Skip, bounds, phase traversal, normalization, persistence assertions" << std::endl;
}
