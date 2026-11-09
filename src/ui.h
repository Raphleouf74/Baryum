#ifndef UI_H
#define UI_H

/* --- Mise en page (ajustable) ---------------------------------------- */
#define GRID_COLS  3
#define GRID_ROWS  2      /* lignes entièrement visibles */
#define CARD_W     96
#define CARD_H     84
#define CARD_GAP   8
#define GRID_X0    8
#define GRID_Y0    34
#define CARD_RADIUS 8
#define ROW_PITCH  (CARD_H + CARD_GAP)

/* Coordonnées "contenu" (avant défilement) d'une case. */
int ui_cell_x(int col);
int ui_cell_y(int row);

void ui_init_text(void);

/* Dessine toute la scène : fond, cadre de sélection (sel_x/sel_y en
 * coordonnées contenu), cartes, barres, scrollbar. scroll en pixels. */
void ui_draw_scene(int sel_x, int sel_y, int scroll);

/* Fenêtre modale d'information / d'erreur. */
void ui_draw_popup(const char *title, const char *l1, const char *l2);
#endif
