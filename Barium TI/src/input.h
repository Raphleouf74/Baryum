#ifndef INPUT_H
#define INPUT_H
#include <stdint.h>

enum {
    EV_UP     = 1 << 0,
    EV_DOWN   = 1 << 1,
    EV_LEFT   = 1 << 2,
    EV_RIGHT  = 1 << 3,
    EV_OK     = 1 << 4,   /* 2nde ou Entrée */
    EV_THEME  = 1 << 5,   /* Mode */
    EV_ACCENT = 1 << 6,   /* Y= */
    EV_QUIT   = 1 << 7    /* Clear */
};

/* Appeler une fois par image : renvoie les événements (front montant,
 * avec répétition automatique pour les flèches). */
uint8_t input_poll(void);
#endif
