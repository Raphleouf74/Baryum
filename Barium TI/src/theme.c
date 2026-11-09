#include "theme.h"
#include <graphx.h>

#define RGB(r,g,b) gfx_RGBTo1555(r,g,b)

static const uint16_t accents[ACCENT_COUNT] = {
    RGB(66, 133, 244),   /* bleu */
    RGB(52, 199, 89),    /* vert */
    RGB(255, 149, 0),    /* orange */
    RGB(175, 82, 222),   /* violet */
};

void theme_apply(uint8_t dark, uint8_t accent)
{
    uint16_t pal[COL_COUNT];

    if (accent >= ACCENT_COUNT) accent = 0;

    if (dark) {
        pal[COL_BG]      = RGB(18, 18, 22);
        pal[COL_SURFACE] = RGB(40, 40, 48);
        pal[COL_TEXT]    = RGB(240, 240, 245);
        pal[COL_MUTED]   = RGB(140, 140, 150);
    } else {
        pal[COL_BG]      = RGB(242, 242, 247);
        pal[COL_SURFACE] = RGB(255, 255, 255);
        pal[COL_TEXT]    = RGB(20, 20, 24);
        pal[COL_MUTED]   = RGB(120, 120, 130);
    }
    pal[COL_ACCENT]    = accents[accent];
    pal[COL_ON_ACCENT] = RGB(255, 255, 255);

    gfx_SetPalette(pal, sizeof pal, COL_BG);
}
