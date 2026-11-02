#pragma once
#include <lvgl/lvgl.h>
// Existing 20px launcher font with a private-use Roost glyph (U+EB00).
// Other app icons and text are delegated unchanged.
// [READER NOTE WQB038] Declaration for the icon wrapper used only in the app-launcher button matrix.
const lv_font_t* RoostLauncherFont();
