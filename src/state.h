#ifndef STATE_H
#define STATE_H
#include <stdbool.h>

/* Marqueur de lancement ("breadcrumb") stocké dans l'archive : il survit
 * à un reset de la calculatrice. Posé juste avant de lancer un programme,
 * effacé quand le launcher reprend la main normalement. */
bool state_mark_launching(const char *name);   /* false si impossible */
bool state_read_launching(char name_out[9]);   /* true si un lancement était en cours */
void state_clear(void);
#endif
