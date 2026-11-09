#include "settings.h"
#include "theme.h"
#include <fileioc.h>

#define CFG_NAME  "LNCHCFG"
#define CFG_MAGIC 0x4C43

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

    h = ti_Open(CFG_NAME, "r");
    if (!h) return;
    if (ti_Read(&f, sizeof f, 1, h) == 1 &&
        f.magic == CFG_MAGIC && f.version == 1 &&
        f.s.accent < ACCENT_COUNT) {
        *s = f.s;
    }
    ti_Close(h);
}

void settings_save(const settings_t *s)
{
    cfg_file_t f = { CFG_MAGIC, 1, *s };
    uint8_t h = ti_Open(CFG_NAME, "w");   /* crée/écrase l'AppVar en RAM */
    if (!h) return;
    ti_Write(&f, sizeof f, 1, h);
    ti_Close(h);
}
