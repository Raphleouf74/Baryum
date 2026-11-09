#include "ui.h"
#include "theme.h"
#include "apps.h"
#include <graphx.h>
#include <stdio.h>
#include <string.h>

#define VIEW_TOP     27
#define VIEW_BOTTOM  223

int ui_cell_x(int col) { return GRID_X0 + col * (CARD_W + CARD_GAP); }
int ui_cell_y(int row) { return row * ROW_PITCH; }

void ui_init_text(void)
{
    gfx_SetTextBGColor(COL_TRANSPARENT);
    gfx_SetTextTransparentColor(COL_TRANSPARENT);
    gfx_SetTextConfig(gfx_text_clip);   /* indispensable : les cartes sortent de l'écran en défilant */
    gfx_SetTextScale(1, 1);
}

static void text(const char *s, int x, int y, uint8_t color)
{
    gfx_SetTextFGColor(color);
    gfx_PrintStringXY(s, x, y);
}

/* Rectangle arrondi plein (rayon r). */
static void fill_round(int x, int y, int w, int h, int r, uint8_t color)
{
    gfx_SetColor(color);
    gfx_FillRectangle(x + r, y, w - 2 * r, h);
    gfx_FillRectangle(x, y + r, w, h - 2 * r);
    gfx_FillCircle(x + r,         y + r,         r);
    gfx_FillCircle(x + w - 1 - r, y + r,         r);
    gfx_FillCircle(x + r,         y + h - 1 - r, r);
    gfx_FillCircle(x + w - 1 - r, y + h - 1 - r, r);
}

static void draw_card(int x, int y, const app_t *a)
{
    char letter[2] = { a->name[0], '\0' };
    uint8_t icon = (a->kind == APP_NATIVE) ? COL_ACCENT : COL_MUTED;
    const char *kind = (a->type != 5) ? "PROTEGE"
                     : (a->kind == APP_NATIVE ? "ASM/C" : "BASIC");

    fill_round(x, y, CARD_W, CARD_H, CARD_RADIUS, COL_SURFACE);
    fill_round(x + 8, y + 8, 32, 32, 8, icon);

    gfx_SetTextScale(2, 2);
    text(letter, x + 8 + 8, y + 8 + 8, COL_ON_ACCENT);
    gfx_SetTextScale(1, 1);

    text(a->name, x + 8, y + 50, COL_TEXT);
    text(kind, x + 8, y + 64, COL_MUTED);
}

static void draw_bars(void)
{
    char buf[16];

    /* barre haute */
    gfx_SetColor(COL_BG);
    gfx_FillRectangle_NoClip(0, 0, 320, VIEW_TOP);
    text("BARIUM", 8, 10, COL_TEXT);
    sprintf(buf, "%d prgm", app_count);
    text(buf, 320 - 8 - (int)gfx_GetStringWidth(buf), 10, COL_MUTED);
    gfx_SetColor(COL_SURFACE);
    gfx_FillRectangle_NoClip(0, VIEW_TOP - 1, 320, 1);

    /* barre basse */
    gfx_SetColor(COL_BG);
    gfx_FillRectangle_NoClip(0, VIEW_BOTTOM, 320, 240 - VIEW_BOTTOM);
    gfx_SetColor(COL_SURFACE);
    gfx_FillRectangle_NoClip(0, VIEW_BOTTOM, 320, 1);
    text("2nd:Run Mode:Theme Y=:Color", 8, 229, COL_MUTED);
    text("Clear:Quit", 320 - 8 - 80, 229, COL_MUTED);
}

static void draw_scrollbar(int scroll, int total_rows)
{
    int track_h = GRID_ROWS * CARD_H + (GRID_ROWS - 1) * CARD_GAP;
    int max_scroll = (total_rows - GRID_ROWS) * ROW_PITCH;
    int thumb_h, thumb_y;

    if (total_rows <= GRID_ROWS) return;
    thumb_h = track_h * GRID_ROWS / total_rows;
    thumb_y = GRID_Y0 + (track_h - thumb_h) * scroll / max_scroll;

    fill_round(313, GRID_Y0, 4, track_h, 2, COL_SURFACE);
    fill_round(313, thumb_y, 4, thumb_h, 2, COL_ACCENT);
}

void ui_draw_scene(int sel_x, int sel_y, int scroll)
{
    int total_rows = (app_count + GRID_COLS - 1) / GRID_COLS;
    int r, c, r0;

    gfx_FillScreen(COL_BG);

    if (app_count == 0) {
        const char *msg = "Aucun programme trouve";
        text(msg, (320 - (int)gfx_GetStringWidth(msg)) / 2, 110, COL_MUTED);
        draw_bars();
        return;
    }

    /* cadre de sélection : dessiné DERRIÈRE les cartes, il en dépasse de 3 px */
    fill_round(sel_x - 3, GRID_Y0 + sel_y - scroll - 3,
               CARD_W + 6, CARD_H + 6, CARD_RADIUS + 3, COL_ACCENT);

    r0 = scroll / ROW_PITCH;
    for (r = r0; r < r0 + GRID_ROWS + 2 && r < total_rows; r++) {
        int sy = GRID_Y0 + ui_cell_y(r) - scroll;
        if (sy + CARD_H <= 0 || sy >= 240) continue;     /* hors écran */
        for (c = 0; c < GRID_COLS; c++) {
            int i = r * GRID_COLS + c;
            if (i >= app_count) break;
            draw_card(ui_cell_x(c), sy, &apps[i]);
        }
    }

    draw_bars();                       /* recouvre les cartes qui débordent */
    draw_scrollbar(scroll, total_rows);
}

void ui_draw_popup(const char *title, const char *l1, const char *l2)
{
    const int w = 264, h = 104;
    int x = (320 - w) / 2, y = (240 - h) / 2;

    fill_round(x - 2, y - 2, w + 4, h + 4, 12, COL_ACCENT);
    fill_round(x, y, w, h, 10, COL_SURFACE);
    text(title, x + 14, y + 14, COL_ACCENT);
    text(l1, x + 14, y + 38, COL_TEXT);
    text(l2, x + 14, y + 52, COL_TEXT);
    text("OK : fermer", x + 14, y + h - 22, COL_MUTED);
}
