#ifndef THEME_H
#define THEME_H
#include <stdint.h>

/* Indices de palette utilisés par toute l'UI (mode 8 bpp de GraphX). */
enum {
    COL_BG = 0,
    COL_SURFACE,
    COL_TEXT,
    COL_MUTED,
    COL_ACCENT,
    COL_ON_ACCENT,
    COL_COUNT
};
#define COL_TRANSPARENT 254   /* index réservé pour le fond de texte transparent */
#define ACCENT_COUNT    4

void theme_apply(uint8_t dark, uint8_t accent);
#endif
