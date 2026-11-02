#include "RoostPreferences.hpp"
#include <cassert>

int main() {
  Roost::Pattern pattern {};
  const uint8_t twoPhase[4] = {4, 0, 8, 0};
  assert(Roost::Normalize(twoPhase, pattern));
  assert(pattern.count == 2 && pattern.cycleSeconds == 12);
  assert(pattern.slots[1].phase == Roost::Phase::Exhale);
  const uint8_t invalid[4] = {0, 0, 0, 0};
  assert(!Roost::Normalize(invalid, pattern) && pattern.count == 2);

  Roost::Session session;
  assert(session.SaveCustom(twoPhase));
  assert(session.Select(Roost::Selection::Custom));
  session.Start(0xfffffff0u);
  assert(session.Observe(0xfffffff0u).phase == Roost::Phase::Inhale);
  assert(!session.Poll(0xfffffff0u).due);
  auto cue = session.Poll(0x00000f90u); // 4000 ms across tick wrap
  assert(cue.due && cue.phase == Roost::Phase::Exhale);
  assert(session.Authorize(cue));
  session.End(Roost::EndReason::IncomingCall);
  assert(!session.Authorize(cue) && !session.Poll(0x00001f30u).due);
  assert(session.LastEnd() == Roost::EndReason::IncomingCall);
  session.Start(100);
  assert(!session.Authorize(cue));
  assert(!session.Poll(101).due);
  // A late observer receives at most one current boundary, not the missed cues.
  auto late = session.Poll(25000);
  assert(late.due && late.phase == Roost::Phase::Inhale);
  assert(!session.Poll(25000).due);

  Roost::Preferences preferences {};
  preferences.selected = Roost::Selection::Custom;
  preferences.hasCustom = true;
  for (int i = 0; i < 4; ++i) preferences.custom[i] = twoPhase[i];
  uint8_t record[12] {};
  Roost::Encode(preferences, record);
  Roost::Preferences restored {};
  assert(Roost::Decode(record, restored));
  Roost::Session reopened;
  Roost::Apply(restored, reopened);
  assert(reopened.Selected() == Roost::Selection::Custom && !reopened.Running());
  record[7] ^= 1;
  assert(!Roost::Decode(record, restored));
}
