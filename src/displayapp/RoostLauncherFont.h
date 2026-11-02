#pragma once
#include <lvgl/lvgl.h>
// Existing 20px launcher font with a private-use Roost glyph (U+EB00).
// Other app icons and text are delegated unchanged.
const lv_font_t* RoostLauncherFont();
