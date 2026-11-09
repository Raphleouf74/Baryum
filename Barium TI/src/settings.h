#ifndef SETTINGS_H
#define SETTINGS_H
#include <stdint.h>

typedef struct {
    uint8_t dark;      /* 1 = thème sombre */
    uint8_t accent;    /* index de couleur d'accent */
} settings_t;

void settings_load(settings_t *s);   /* valeurs par défaut si absent/invalide */
void settings_save(const settings_t *s);
#endif
