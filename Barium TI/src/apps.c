#include "apps.h"
#include <fileioc.h>
#include <ti/vars.h>
#include <graphx.h>
#include <ti/screen.h>
#include "state.h"
#include <string.h>
#include <stdbool.h>

app_t apps[APPS_MAX];
int   app_count;

/* Marqueur d'un programme natif (C/ASM) : 0xEF 0x7B. Voir tExtTok /
 * tAsm84CeCmp dans cesium-master/include/ti84pceg.inc. */
#define TOK_EXT      0xEF
#define TOK_ASM84CE  0x7B

static uint8_t classify(const char *name, uint8_t type)
{
    uint8_t buf[2] = {0, 0};
    uint8_t h = ti_OpenVar(name, "r", type);
    if (!h) return APP_BASIC;
    ti_Read(buf, 1, 2, h);
    ti_Close(h);
    return (buf[0] == TOK_EXT && buf[1] == TOK_ASM84CE) ? APP_NATIVE : APP_BASIC;
}

static void sort_apps(void)
{
    int i, j;
    for (i = 1; i < app_count; i++) {
        app_t key = apps[i];
        j = i - 1;
        while (j >= 0 && strcmp(apps[j].name, key.name) > 0) {
            apps[j + 1] = apps[j];
            j--;
        }
        apps[j + 1] = key;
    }
}

void apps_scan(const char *self_name)
{
    void *vat = NULL;
    uint8_t type;
    char *name;

    app_count = 0;
    while (app_count < APPS_MAX &&
           (name = ti_DetectAny(&vat, NULL, &type)) != NULL) {
        app_t *a;
        if (type != OS_TYPE_PRGM && type != OS_TYPE_PROT_PRGM) continue;
        if (name[0] < 'A') continue;                 /* heuristique : programmes cachés */
        if (strcmp(name, self_name) == 0) continue;  /* ne pas s'afficher soi-même */

        a = &apps[app_count];
        strncpy(a->name, name, 8);
        a->name[8] = '\0';
        a->type = type;
        a->kind = classify(a->name, type);
        app_count++;
    }
    sort_apps();
}

int apps_preflight(const app_t *a)
{
    uint8_t h;
    uint16_t size;

    if (a->type == OS_TYPE_PROT_PRGM)
        return LAUNCH_PROTECTED;   /* os_RunPrgm ne cherche que les programmes non protégés */

    h = ti_OpenVar(a->name, "r", a->type);
    if (!h) return LAUNCH_MISSING;
    size = ti_GetSize(h);
    ti_Close(h);
    if (size < 2) return LAUNCH_EMPTY;
    return LAUNCH_OK;
}

const char *apps_error_text(int code)
{
    switch (code) {
    case LAUNCH_MISSING:   return "Programme introuvable";
    case LAUNCH_PROTECTED: return "Programme protege";
    case LAUNCH_EMPTY:     return "Programme vide";
    case LAUNCH_NO_MEMORY: return "Memoire insuffisante";
    default:               return "Echec du lancement";
    }
}

/* Contexte transmis au callback : gardé minimal (stocké sur la pile). */
typedef struct { int selected; } return_ctx_t;

extern int launcher_resume(int selected, int retval);   /* défini dans main.c */

static int on_return(void *data, int retval)
{
    return launcher_resume(((return_ctx_t *)data)->selected, retval);
}

int apps_launch(const app_t *a, int selected_index)
{
    return_ctx_t ctx;
    int pf, ret;

    ctx.selected = selected_index;
    pf = apps_preflight(a);
    if (pf != LAUNCH_OK) return pf;      /* rien n'a été touché : on reste dans le launcher */

    state_mark_launching(a->name);       /* marqueur de plantage (survit à un reset) */

    gfx_End();                           /* rend l'écran à l'OS */
    if (a->kind == APP_BASIC)
        os_ClrHome();                    /* écran d'accueil propre pour un programme BASIC */

    ret = os_RunPrgm(a->name, &ctx, sizeof ctx, on_return);

    /* On n'arrive ici que si le lancement a échoué (ret < 0). */
    state_clear();
    gfx_Begin();
    gfx_SetDrawBuffer();
    return (ret == OS_RUN_PRGM_NOT_FOUND) ? LAUNCH_MISSING
         : (ret == OS_RUN_PRGM_ERR_MEMORY) ? LAUNCH_NO_MEMORY
         : LAUNCH_FAILED;
}
