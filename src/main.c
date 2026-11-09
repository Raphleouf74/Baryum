/* Barium v0.2 - lancement sécurisé, cartes arrondies, défilement animé. */
#include "anim.h"
#include "apps.h"
#include "input.h"
#include "settings.h"
#include "state.h"
#include "theme.h"
#include "ui.h"
#include <graphx.h>
#include <stdbool.h>
#include <stdio.h>

#define SELF_NAME   "BARIUM"   /* doit correspondre à NAME dans le makefile */
#define ANIM_FRAMES 8

typedef struct {
    int sel;               /* index absolu sélectionné */
    int first_row;         /* première ligne visible (cible du défilement) */
    int fx, fy, tx, ty;    /* cadre de sélection : départ / cible (coord. contenu) */
    int sf, st;            /* défilement : départ / cible (pixels) */
    int t;                 /* image courante de l'animation */
} view_t;

typedef struct {
    bool active;
    char title[24], l1[36], l2[36];
} popup_t;

static settings_t cfg;
static view_t v;
static popup_t popup;

static void show_popup(const char *title, const char *l1, const char *l2)
{
    popup.active = true;
    snprintf(popup.title, sizeof popup.title, "%s", title);
    snprintf(popup.l1, sizeof popup.l1, "%s", l1);
    snprintf(popup.l2, sizeof popup.l2, "%s", l2);
}

static void current_pos(int *x, int *y, int *scroll)
{
    int p = ease_out_cubic(v.t, ANIM_FRAMES);
    *x = lerp256(v.fx, v.tx, p);
    *y = lerp256(v.fy, v.ty, p);
    *scroll = lerp256(v.sf, v.st, p);
}

/* Sélectionne idx et lance l'animation depuis la position affichée. */
static void select_index(int idx, bool animate)
{
    int row, x, y, sc;
    if (app_count == 0) return;
    if (idx < 0) idx = 0;
    if (idx >= app_count) idx = app_count - 1;

    current_pos(&x, &y, &sc);
    v.fx = x; v.fy = y; v.sf = sc;

    v.sel = idx;
    row = idx / GRID_COLS;
    if (row < v.first_row) v.first_row = row;
    if (row >= v.first_row + GRID_ROWS) v.first_row = row - GRID_ROWS + 1;

    v.tx = ui_cell_x(idx % GRID_COLS);
    v.ty = ui_cell_y(row);
    v.st = v.first_row * ROW_PITCH;

    if (!animate) { v.fx = v.tx; v.fy = v.ty; v.sf = v.st; v.t = ANIM_FRAMES; }
    else          { v.t = 0; }
}

static void render(void)
{
    int x, y, sc;
    current_pos(&x, &y, &sc);
    ui_draw_scene(x, y, sc);
    if (popup.active) ui_draw_popup(popup.title, popup.l1, popup.l2);
    gfx_SwapDraw();
}

static void reinit_screen(void)
{
    theme_apply(cfg.dark, cfg.accent);
    ui_init_text();
}

/* Point d'entrée commun : démarrage normal (returned = false) ou reprise
 * après un programme lancé (returned = true, retval = code de sortie). */
static int launcher_run(int start_sel, int retval, bool returned)
{
    bool dirty = true;
    char last[9];
    bool was_launching;

    gfx_Begin();
    gfx_SetDrawBuffer();
    settings_load(&cfg);
    reinit_screen();
    apps_scan(SELF_NAME);

    popup.active = false;
    v.first_row = 0;
    v.t = ANIM_FRAMES;
    v.fx = v.fy = v.tx = v.ty = v.sf = v.st = 0;
    select_index(start_sel, false);

    /* --- Auto-diagnostic au retour / au démarrage ---------------------- */
    was_launching = state_read_launching(last);
    state_clear();
    if (was_launching) {
        char l2[36];
        if (returned) {
            if (retval != 0) {
                snprintf(l2, sizeof l2, "a retourne le code %d", retval);
                show_popup("Erreur", last, l2);
            }
        } else {
            show_popup("Lancement interrompu", last, "ne s'est pas termine bien");
        }
    }

    for (;;) {
        uint8_t ev = input_poll();

        if (popup.active) {
            if (ev & (EV_OK | EV_QUIT)) { popup.active = false; dirty = true; }
        } else {
            if (ev & EV_QUIT) break;
            if (ev & EV_THEME)  { cfg.dark ^= 1; reinit_screen(); settings_save(&cfg); dirty = true; }
            if (ev & EV_ACCENT) { cfg.accent = (cfg.accent + 1) % ACCENT_COUNT; reinit_screen(); settings_save(&cfg); dirty = true; }

            if (app_count > 0) {
                int idx = v.sel;
                if (ev & EV_LEFT)  idx--;
                if (ev & EV_RIGHT) idx++;
                if (ev & EV_UP)    idx -= GRID_COLS;
                if (ev & EV_DOWN)  idx += GRID_COLS;
                if (idx != v.sel) { select_index(idx, true); dirty = true; }

                if (ev & EV_OK) {
                    int code = apps_launch(&apps[v.sel], v.sel);
                    /* On n'arrive ici qu'en cas d'échec : on reste dans le launcher. */
                    reinit_screen();
                    show_popup("Lancement impossible", apps[v.sel].name, apps_error_text(code));
                    dirty = true;
                }
            }
        }

        if (v.t < ANIM_FRAMES) { v.t++; dirty = true; }
        if (dirty) { render(); dirty = false; }
    }

    gfx_End();
    return 0;
}

/* Appelé par le callback de os_RunPrgm quand un programme se termine. */
int launcher_resume(int selected, int retval)
{
    return launcher_run(selected, retval, true);
}

int main(void)
{
    return launcher_run(0, 0, false);
}
