// Roost launcher glyph derived from the SVG supplied by Aurora.
// Original SVG SHA-256: 21348ede866c135f4980b7ebfeec31c207eb11006fb3634658289ce85a348c3f
// This is an 18x18 1-bpp bitmap (41 bytes) inside a wrapper for the regular
// jetbrains_mono_bold_20 font. No new font file or font-generation dependency.
#include "displayapp/RoostLauncherFont.h"
#include <lvgl/lvgl.h>

namespace {
constexpr uint32_t roostCodepoint = 0xeb00; // Unicode Private Use; Symbols::roost
constexpr uint8_t roostGlyphBitmap[] = {
    0x7f, 0xff, 0xa0, 0x00, 0x18, 0x00, 0x06, 0x07, 0x81, 0x87,
    0xf8, 0x63, 0xff, 0x18, 0xff, 0xc6, 0x7f, 0xf9, 0x9f, 0xfe,
    0x67, 0xff, 0x99, 0xff, 0xe6, 0x3f, 0xf1, 0x8f, 0xfc, 0x61,
    0xfe, 0x18, 0x1e, 0x06, 0x00, 0x01, 0x80, 0x00, 0x5f, 0xff,
    0xe0,
};
static_assert(sizeof(roostGlyphBitmap) == (18 * 18 + 7) / 8);

bool GetGlyphDescriptor(const lv_font_t* /*font*/, lv_font_glyph_dsc_t* out,
                        uint32_t letter, uint32_t following) {
  if (letter == roostCodepoint) {
    // Same 20 px text advance as the stock launcher icon font. Align the
    // 18 px image to the glyph baseline, with 1 px of horizontal inset.
    out->adv_w = 20;
    out->box_w = 18;
    out->box_h = 18;
    out->ofs_x = 1;
    out->ofs_y = 0;
    out->bpp = 1;
    return true;
  }
  return lv_font_get_glyph_dsc(&jetbrains_mono_bold_20, out, letter, following);
}

const uint8_t* GetGlyphBitmap(const lv_font_t* /*font*/, uint32_t letter) {
  if (letter == roostCodepoint) return roostGlyphBitmap;
  return lv_font_get_glyph_bitmap(&jetbrains_mono_bold_20, letter);
}
}  // namespace

const lv_font_t* RoostLauncherFont() {
  // Only the app-launcher button matrix uses the wrapper. All other glyphs
  // delegate to the existing font; no copying of its bitmap tables occurs.
  static lv_font_t wrapped = [] {
    lv_font_t font = jetbrains_mono_bold_20;
    font.get_glyph_dsc = GetGlyphDescriptor;
    font.get_glyph_bitmap = GetGlyphBitmap;
    return font;
  }();
  return &wrapped;
}
