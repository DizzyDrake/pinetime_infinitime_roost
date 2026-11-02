#include "displayapp/screens/Roost.h"
#include <lvgl/lvgl.h>
#include <cstdio>

using Pinetime::Applications::Screens::RoostScreen;

namespace {
  void EventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<RoostScreen*>(obj->user_data);
    screen->OnEvent(obj, event);
  }
  const char* PatternName(Roost::Selection selected) {
    switch (selected) {
      case Roost::Selection::Box: return "4-4-4-4";
      case Roost::Selection::FourSevenEight: return "4-7-8";
      case Roost::Selection::TwoEight: return "2-8";
      case Roost::Selection::Custom: return "Custom";
    }
    return "4-4-4-4";
  }
  const char* PhaseName(Roost::Phase phase) {
    switch (phase) {
      case Roost::Phase::Inhale: return "Inhale";
      case Roost::Phase::Exhale: return "Exhale";
      case Roost::Phase::HoldAfterInhale:
      case Roost::Phase::HoldAfterExhale: return "Hold";
    }
    return "Inhale";
  }
  const char* EditPhaseName(uint8_t slot) {
    switch (slot) {
      case 0: return "Inhale";
      case 1: case 3: return "Hold";
      case 2: return "Exhale";
    }
    return "Inhale";
  }
  // Transparent buttons remain touchable but must not obscure the circle.
  void TransparentButton(lv_obj_t* button) {
    lv_obj_set_style_local_bg_opa(button, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_border_width(button, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  }

}

// [READER NOTE WQB038] The constructor creates a vertical blue gradient, white breathing circle, compact pattern/start controls and invisible touch targets.
RoostScreen::RoostScreen(Pinetime::Controllers::RoostService& service) : service {service} {
  background = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(background, 240, 240);
  lv_obj_align(background, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_color(background, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x9CDCF5));
  lv_obj_set_style_local_bg_grad_color(background, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x173D82));
  lv_obj_set_style_local_bg_grad_dir(background, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
  lv_obj_set_style_local_bg_opa(background, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_obj_set_style_local_border_width(background, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(background, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

  circle = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_radius(circle, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_bg_color(circle, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_style_local_bg_opa(circle, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_obj_set_style_local_border_width(circle, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(circle, 64, 64);
  lv_obj_align(circle, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  phaseLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(phaseLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_label_set_text(phaseLabel, "Inhale");
  lv_obj_align(phaseLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 18);

  patternButton = lv_btn_create(lv_scr_act(), nullptr);
  patternButton->user_data = this;
  lv_obj_set_event_cb(patternButton, EventHandler);
  lv_obj_set_size(patternButton, 100, 38);
  lv_obj_align(patternButton, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 8, -7);
  lv_obj_set_style_local_bg_opa(patternButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  patternLabel = lv_label_create(patternButton, nullptr);
  lv_obj_set_style_local_text_color(patternLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xD5DDEC));

  playButton = lv_btn_create(lv_scr_act(), nullptr);
  playButton->user_data = this;
  lv_obj_set_event_cb(playButton, EventHandler);
  lv_obj_set_size(playButton, 76, 38);
  lv_obj_align(playButton, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -8, -7);
  lv_obj_set_style_local_bg_opa(playButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  playLabel = lv_label_create(playButton, nullptr);
  lv_obj_set_style_local_text_color(playLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  // Normal view: long press the central circle area to edit Custom.
  editHotspot = lv_btn_create(lv_scr_act(), nullptr);
  editHotspot->user_data = this;
  lv_obj_set_event_cb(editHotspot, EventHandler);
  lv_obj_set_size(editHotspot, 160, 140);
  lv_obj_align(editHotspot, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  TransparentButton(editHotspot);

  editValueButton = lv_btn_create(lv_scr_act(), nullptr);
  editValueButton->user_data = this;
  lv_obj_set_event_cb(editValueButton, EventHandler);
  lv_obj_set_size(editValueButton, 172, 95);
  lv_obj_align(editValueButton, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  TransparentButton(editValueButton);
  editValueLabel = lv_label_create(editValueButton, nullptr);
  lv_obj_set_style_local_text_color(editValueLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_align(editValueLabel, editValueButton, LV_ALIGN_CENTER, 0, 0);

  editUpButton = lv_btn_create(lv_scr_act(), nullptr);
  editUpButton->user_data = this;
  lv_obj_set_event_cb(editUpButton, EventHandler);
  lv_obj_set_size(editUpButton, 110, 41);
  lv_obj_align(editUpButton, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 5);
  TransparentButton(editUpButton);
  editUpLabel = lv_label_create(editUpButton, nullptr);
  lv_label_set_text(editUpLabel, "+");
  lv_obj_set_style_local_text_color(editUpLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_align(editUpLabel, editUpButton, LV_ALIGN_CENTER, 0, 0);

  editDownButton = lv_btn_create(lv_scr_act(), nullptr);
  editDownButton->user_data = this;
  lv_obj_set_event_cb(editDownButton, EventHandler);
  lv_obj_set_size(editDownButton, 110, 41);
  lv_obj_align(editDownButton, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -65, -10);
  TransparentButton(editDownButton);
  editDownLabel = lv_label_create(editDownButton, nullptr);
  lv_label_set_text(editDownLabel, "-");
  lv_obj_set_style_local_text_color(editDownLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_align(editDownLabel, editDownButton, LV_ALIGN_CENTER, 0, 0);

  ShowEditor(false);
  Refresh();
  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

// [READER NOTE WQB038] Destroying the screen must not end breathing: display sleep and phone notification overlays can replace LVGL views.
RoostScreen::~RoostScreen() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
  // The app screen is preserved during normal sleep. Stop on the physical
  // button instead of destructing here: phone notification overlays also
  // destroy the screen and must not end Roost.
}

// [READER NOTE WQB038] ShowEditor swaps the normal breathing widgets for a small on-watch four-phase editor.
void RoostScreen::ShowEditor(bool show) {
  lv_obj_set_hidden(circle, show);
  lv_obj_set_hidden(phaseLabel, show);
  lv_obj_set_hidden(patternButton, show);
  lv_obj_set_hidden(playButton, show);
  lv_obj_set_hidden(editHotspot, show);
  lv_obj_set_hidden(editValueButton, !show);
  lv_obj_set_hidden(editUpButton, !show);
  lv_obj_set_hidden(editDownButton, !show);
}

// [READER NOTE WQB038] Entering edit mode stops the current session first; unsaved changes stay in EditorDraft until SaveEdit.
void RoostScreen::BeginEdit() {
  service.Stop(); // Provisional: editing a new cycle never changes a live one.
  service.GetCustomEditValues(editSeed);
  draft = Roost::EditorDraft(editSeed);
  saveRejected = false;
  editing = true;
  ShowEditor(true);
  Refresh();
}

// [READER NOTE WQB038] SaveEdit refuses all-Skip, then writes custom seconds and selects Custom. This is two explicit writes, not an atomic combined transaction.
void RoostScreen::SaveEdit() {
  if (!editing) return;
  if (!draft.Valid()) {
    saveRejected = true; // All-Skip cannot form a runnable cycle.
    Refresh();
    return;
  }
  uint8_t values[4] {};
  draft.CopyTo(values);
  // Two explicit persistence calls in this first trial; a combined atomic
  // Save+Select should be evaluated before final release.
  if (service.SaveCustom(values) && service.Select(Roost::Selection::Custom)) {
    editing = false;
    saveRejected = false;
    ShowEditor(false);
  }
  Refresh();
}

// [READER NOTE WQB038] The intentional physical button exit calls service.Stop and returns false for InfiniTime normal back navigation.
bool RoostScreen::OnButtonPushed() {
  // Aurora-confirmed: intentional physical-button exit ends the session.
  // Return false so DisplayApp performs its usual back navigation.
  editing = false; // Discard unsaved draft on exit.
  service.Stop();
  return false;
}

// [READER NOTE WQB038] Refresh reads session time to size the circle; it displays Ready when idle and does not drive haptics.
void RoostScreen::Refresh() {
  if (editing) {
    char text[56] {};
    if (saveRejected) {
      std::snprintf(text, sizeof(text), "Choose at least\none phase");
    } else if (draft.Seconds() == 0) {
      std::snprintf(text, sizeof(text), "Skip\n%s (%u/4)", EditPhaseName(draft.SlotIndex()),
                    static_cast<unsigned>(draft.SlotIndex() + 1));
    } else {
      std::snprintf(text, sizeof(text), "%u s\n%s (%u/4)",
                    static_cast<unsigned>(draft.Seconds()), EditPhaseName(draft.SlotIndex()),
                    static_cast<unsigned>(draft.SlotIndex() + 1));
    }
    lv_label_set_text(editValueLabel, text);
    lv_obj_align(editValueLabel, editValueButton, LV_ALIGN_CENTER, 0, 0);
    return;
  }
  const auto view = service.Observe();
  const uint32_t fraction = view.phaseDurationMs == 0 ? 0 : view.phaseElapsedMs * 51u / view.phaseDurationMs;
  uint32_t radius = 32;
  if (view.running) {
    if (view.phase == Roost::Phase::Inhale) radius += fraction;
    else if (view.phase == Roost::Phase::HoldAfterInhale) radius = 83;
    else if (view.phase == Roost::Phase::Exhale) radius = 83 - fraction;
  }
  const auto diameter = static_cast<lv_coord_t>(radius * 2u);
  lv_obj_set_size(circle, diameter, diameter);
  lv_obj_align(circle, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  // Reopening after a call/alarm must show Ready, not a stale Ended state.
  lv_label_set_text(phaseLabel, view.running ? PhaseName(view.phase) : "Ready");
  lv_obj_align(phaseLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 18);
  lv_label_set_text(patternLabel, PatternName(service.Selected()));
  lv_label_set_text(playLabel, service.Running() ? "Stop" : "Start");
}

// [READER NOTE WQB038] OnEvent dispatches long-press editing, tap slot changes, +/− durations and Start/Stop or preset switching.
void RoostScreen::OnEvent(lv_obj_t* obj, lv_event_t event) {
  if (editing) {
    if (event == LV_EVENT_LONG_PRESSED && obj == editValueButton) {
      SaveEdit();
    } else if (event == LV_EVENT_CLICKED && obj == editValueButton) {
      draft.Next();
      saveRejected = false;
      Refresh();
    } else if (event == LV_EVENT_CLICKED && obj == editUpButton) {
      draft.Increase();
      saveRejected = false;
      Refresh();
    } else if (event == LV_EVENT_CLICKED && obj == editDownButton) {
      draft.Decrease();
      saveRejected = false;
      Refresh();
    }
    return;
  }
  if (event == LV_EVENT_LONG_PRESSED && obj == editHotspot) {
    BeginEdit();
    return;
  }
  if (event != LV_EVENT_CLICKED) return;
  if (obj == playButton) {
    if (service.Running()) service.Stop();
    else service.Start();
  } else if (obj == patternButton) {
    // Include saved Custom in the compact selector, with no extra menu.
    auto selection = service.Selected();
    if (selection == Roost::Selection::Box) selection = Roost::Selection::FourSevenEight;
    else if (selection == Roost::Selection::FourSevenEight) selection = Roost::Selection::TwoEight;
    else if (selection == Roost::Selection::TwoEight && service.HasSavedCustom()) selection = Roost::Selection::Custom;
    else selection = Roost::Selection::Box;
    service.Select(selection);
  }
  Refresh();
}
