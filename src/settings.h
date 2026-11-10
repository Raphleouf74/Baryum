#ifndef SETTINGS_H
#define SETTINGS_H
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t dark;         /* 1 = theme sombre */
    uint8_t accent;       /* index de couleur d'accent */
    uint8_t launch_mode;  /* 0 = lancement nu, 1 = + nettoyage ecran BASIC, 2 = complet (marqueur + retour) */
    uint8_t auto_return;  /* 1 = revenir au launcher apres un programme (callback os_RunPrgm) */
} settings_t;

void settings_load(settings_t *s);                 /* valeurs par defaut si absent/invalide */
void settings_save(const settings_t *s, bool to_archive);
#endif
