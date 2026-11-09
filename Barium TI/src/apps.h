#ifndef APPS_H
#define APPS_H
#include <stdint.h>

typedef enum { APP_BASIC = 0, APP_NATIVE = 1 } app_kind_t;

typedef struct {
    char    name[9];   /* 8 caractères max + '\0' */
    uint8_t kind;      /* app_kind_t */
    uint8_t type;      /* OS_TYPE_PRGM ou OS_TYPE_PROT_PRGM */
} app_t;

#define APPS_MAX 96

/* Codes de résultat du lancement (0 = pas d'erreur). */
enum {
    LAUNCH_OK = 0,
    LAUNCH_MISSING,      /* programme introuvable (supprimé ?) */
    LAUNCH_PROTECTED,    /* programme protégé : os_RunPrgm ne le trouve pas */
    LAUNCH_EMPTY,        /* programme vide */
    LAUNCH_NO_MEMORY,    /* pas assez de RAM */
    LAUNCH_FAILED        /* échec inattendu */
};

extern app_t apps[APPS_MAX];
extern int   app_count;

void apps_scan(const char *self_name);

/* Auto-test avant lancement : renvoie LAUNCH_OK ou un code d'erreur. */
int apps_preflight(const app_t *a);

/* Lance le programme. Ne revient que sur échec (code != 0). En cas de succès,
 * le launcher reprend via le callback (launcher_resume). */
int apps_launch(const app_t *a, int selected_index);

/* Texte court (ASCII) pour un code LAUNCH_*. */
const char *apps_error_text(int code);
#endif
