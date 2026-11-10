#include "apps.h"
#include <fileioc.h>
#include <ti/vars.h>
#include <graphx.h>
#include <keypadc.h>
#include <ti/screen.h>
#include <ti/getcsc.h>
#include "state.h"
#include <string.h>
#include <stdbool.h>

app_t apps[APPS_MAX];
int   app_count;

/* Marqueur d'un programme natif (C/ASM) : 0xEF 0x7B. Voir tExtTok /
 * tAsm84CeCmp dans cesium-master/include/ti84pceg.inc. */
#define TOK_EXT      0xEF
#define TOK_ASM84CE  0x7B

#define TOK_STOP 0xD9
#define STOP_SCAN_MAX 2048   /* on ne scanne que le debut du programme */

static void classify(app_t *a)
{
    uint8_t buf[64];
    uint8_t h = ti_OpenVar(a->name, "r", a->type);
    unsigned total, got, k;

    a->kind = APP_BASIC;
    a->flags = 0;
    if (!h) return;

    if (ti_IsArchived(h)) a->flags |= APP_F_ARCHIVED;

    got = ti_Read(buf, 1, 2, h);
    if (got == 2 && buf[0] == TOK_EXT && buf[1] == TOK_ASM84CE) {
        a->kind = APP_NATIVE;
    } else {
        /* BASIC : recherche approximative d'un jeton Stop (faux positifs possibles) */
        total = got;
        for (k = 0; k < got; k++) if (buf[k] == TOK_STOP) a->flags |= APP_F_STOP;
        while (total < STOP_SCAN_MAX && !(a->flags & APP_F_STOP)) {
            got = ti_Read(buf, 1, sizeof buf, h);
            if (got == 0) break;
            for (k = 0; k < got; k++) if (buf[k] == TOK_STOP) { a->flags |= APP_F_STOP; break; }
            total += got;
        }
    }
    ti_Close(h);
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
        if (strcmp(name, self_name) == 0) continue;
        if (strcmp(name, "BARTMP") == 0) continue;  /* ne pas s'afficher soi-même */

        a = &apps[app_count];
        strncpy(a->name, name, 8);
        a->name[8] = '\0';
        a->type = type;
        classify(a);
        app_count++;
    }
    sort_apps();
}

/* Programmes qu'os_RunPrgm ne sait pas lancer tels quels : on les copie dans un
 * programme temporaire non protege et non archive (comme Cesium le fait pour
 * les BASIC archives). Les natifs archives sont geres directement par l'OS. */
static bool needs_temp_copy(const app_t *a)
{
    return a->type == OS_TYPE_PROT_PRGM ||
           (a->kind == APP_BASIC && (a->flags & APP_F_ARCHIVED));
}

#define TMP_NAME "BARTMP"

void apps_cleanup_temp(void)
{
    ti_DeleteVar(TMP_NAME, OS_TYPE_PRGM);
}

static int make_temp_copy(const app_t *a)
{
    uint8_t src, dst;
    uint8_t buf[256];
    uint16_t size, done = 0;
    void *freeptr;

    src = ti_OpenVar(a->name, "r", a->type);
    if (!src) return LAUNCH_MISSING;
    size = ti_GetSize(src);

    if (os_MemChk(&freeptr) < (size_t)size + 2048u) {
        ti_Close(src);
        return LAUNCH_NO_MEMORY;
    }

    dst = ti_OpenVar(TMP_NAME, "w", OS_TYPE_PRGM);   /* ecrase une ancienne copie, cree en RAM */
    if (!dst) { ti_Close(src); return LAUNCH_COPY_FAILED; }

    while (done < size) {
        size_t n = ti_Read(buf, 1, sizeof buf, src);
        if (n == 0) break;
        if (ti_Write(buf, 1, n, dst) != n) { done = 0; break; }
        done += (uint16_t)n;
    }
    ti_Close(dst);
    ti_Close(src);

    if (done != size) { apps_cleanup_temp(); return LAUNCH_COPY_FAILED; }
    return LAUNCH_OK;
}

int apps_preflight(const app_t *a)
{
    uint8_t h;
    uint16_t size;

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
    case LAUNCH_EMPTY:     return "Programme vide";
    case LAUNCH_COPY_FAILED: return "Copie temporaire echouee";
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

/* Attend que toutes les touches soient relachees (evite que la touche de
 * validation soit lue par le programme lance). Borne pour ne jamais bloquer. */
static void wait_keys_released(void)
{
    int guard = 0;
    do { kb_Scan(); } while (kb_AnyKey() && ++guard < 30000);
}

/* mode 0 : lancement nu (aucune ecriture en flash, aucun appel OS superflu)
 * mode 1 : + os_ClrHome pour les programmes BASIC
 * mode 2 : + marqueur de plantage et retour automatique (si auto_return) */
int apps_launch(const app_t *a, int selected_index, int mode, bool auto_return)
{
    return_ctx_t ctx;
    const char *run_name;
    int pf, ret;
    bool use_cb = (mode >= 2) && auto_return;

    ctx.selected = selected_index;
    pf = apps_preflight(a);
    if (pf != LAUNCH_OK) return pf;      /* rien n'a ete touche : on reste dans le launcher */

    if (use_cb)
        state_mark_launching(a->name);   /* marqueur de plantage (archive) */

    run_name = a->name;
    if (needs_temp_copy(a)) {
        pf = make_temp_copy(a);
        if (pf != LAUNCH_OK) return pf;
        run_name = TMP_NAME;
    }

    wait_keys_released();
    gfx_SetDrawScreen();                 /* on dessine/affiche le meme buffer */
    gfx_Wait();                          /* plus aucun echange d'ecran en attente */
    gfx_End();                           /* rend l'ecran a l'OS */
    if (mode >= 1 && a->kind == APP_BASIC)
        os_ClrHome();

    if (mode == 3) {   /* DRY RUN : tout sauf os_RunPrgm */
        os_ClrHome();
        os_PutStrFull("DRY RUN BARIUM");
        os_SetCursorPos(1, 0);
        os_PutStrFull(a->name);
        os_SetCursorPos(3, 0);
        os_PutStrFull("Touche = retour");
        while (!os_GetCSC()) { }
        apps_cleanup_temp();
        gfx_Begin();
        gfx_SetDrawBuffer();
        return LAUNCH_OK + 100;   /* code special : ne pas afficher d'erreur */
    }

    if (use_cb)
        ret = os_RunPrgm(run_name, &ctx, sizeof ctx, on_return);
    else
        ret = os_RunPrgm(run_name, NULL, 0, NULL);

    /* On n'arrive ici que si le lancement a echoue (ret < 0). */
    if (use_cb) state_clear();
    apps_cleanup_temp();
    gfx_Begin();
    gfx_SetDrawBuffer();
    return (ret == OS_RUN_PRGM_NOT_FOUND) ? LAUNCH_MISSING
         : (ret == OS_RUN_PRGM_ERR_MEMORY) ? LAUNCH_NO_MEMORY
         : LAUNCH_FAILED;
}
