#include "settings.h"
#include "theme.h"
#include <fileioc.h>

#define CFG_NAME  "LNCHCFG"
#define CFG_MAGIC 0x4C43
#define CFG_VER   3

typedef struct {
    uint16_t magic;
    uint8_t  version;
    settings_t s;
} cfg_file_t;

void settings_load(settings_t *s)
{
    cfg_file_t f;
    uint8_t h;

    s->dark = 1;
    s->accent = 0;
    s->launch_mode = 0;   /* mode le plus simple par defaut (diagnostic) */
    s->auto_return = 0;   /* desactive par defaut tant que le retour n'est pas valide sur materiel */

    h = ti_Open(CFG_NAME, "r");
    if (!h) return;
    if (ti_Read(&f, sizeof f, 1, h) == 1 &&
        f.magic == CFG_MAGIC && f.version == CFG_VER &&
        f.s.accent < ACCENT_COUNT && f.s.launch_mode <= 3) {
        *s = f.s;
    }
    ti_Close(h);
}

void settings_save(const settings_t *s, bool to_archive)
{
    cfg_file_t f = { CFG_MAGIC, CFG_VER, *s };
    uint8_t h = ti_Open(CFG_NAME, "w");   /* cree/ecrase l'AppVar en RAM */
    if (!h) return;
    ti_Write(&f, sizeof f, 1, h);
    /* Archive seulement s'il y a de la place sans garbage collect. */
    if (to_archive && ti_ArchiveHasRoom(sizeof f + 32))
        ti_SetArchiveStatus(true, h);
    ti_Close(h);
}
