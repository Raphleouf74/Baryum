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
/* Del est lu a part (bit supplementaire) */
enum { EV2_RETURN = 1, EV2_MODE = 2, EV2_INFO = 4 };
uint8_t input_poll2(void);   /* evenements secondaires (EV2_*), a appeler apres input_poll */
uint8_t input_poll(void);
#endif
