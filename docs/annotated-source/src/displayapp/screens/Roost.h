#pragma once

#include "components/roost/RoostService.h"
#include "components/roost/RoostEditorDraft.hpp"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
// [READER NOTE WQB038] The screen owns only LVGL widgets and edit draft; it references the persistent service instead of owning haptic timing.
      class RoostScreen : public Screen {
      public:
        explicit RoostScreen(Controllers::RoostService& service);
        ~RoostScreen() override;
        void Refresh() override;
        void OnEvent(lv_obj_t* obj, lv_event_t event);
        bool OnButtonPushed() override;

      private:
        void BeginEdit();
        void SaveEdit();
        void ShowEditor(bool show);

        Controllers::RoostService& service;
        bool editing = false;
        bool saveRejected = false;
        uint8_t editSeed[4] {4, 4, 4, 4};
        Roost::EditorDraft draft {editSeed};
        lv_obj_t* background = nullptr;
        lv_obj_t* circle = nullptr;
        lv_obj_t* phaseLabel = nullptr;
        lv_obj_t* patternButton = nullptr;
        lv_obj_t* patternLabel = nullptr;
        lv_obj_t* playButton = nullptr;
        lv_obj_t* playLabel = nullptr;
        lv_obj_t* editHotspot = nullptr;
        lv_obj_t* editValueButton = nullptr;
        lv_obj_t* editValueLabel = nullptr;
        lv_obj_t* editUpButton = nullptr;
        lv_obj_t* editUpLabel = nullptr;
        lv_obj_t* editDownButton = nullptr;
        lv_obj_t* editDownLabel = nullptr;
        lv_task_t* taskRefresh = nullptr;
      };
    }

    template <>
// [READER NOTE WQB038] AppTraits registers the app with the InfiniTime launcher and routes construction to the RoostService controller.
    struct AppTraits<Apps::Roost> {
      static constexpr Apps app = Apps::Roost;
      static constexpr const char* icon = Screens::Symbols::roost;
      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::RoostScreen(controllers.roostService);
      }
      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) { return true; }
    };
  }
}
