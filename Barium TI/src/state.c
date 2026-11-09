#include "state.h"
#include <fileioc.h>
#include <string.h>
#include <stdint.h>

#define ST_NAME  "BARSTAT"
#define ST_MAGIC 0x4253

typedef struct {
    uint16_t magic;
    char     name[9];
} st_file_t;

bool state_mark_launching(const char *name)
{
    st_file_t f;
    uint8_t h;
    bool archived = false;

    f.magic = ST_MAGIC;
    memset(f.name, 0, sizeof f.name);
    strncpy(f.name, name, 8);

    h = ti_Open(ST_NAME, "w");
    if (!h) return false;
    ti_Write(&f, sizeof f, 1, h);
    /* On n'archive que s'il y a de la place SANS garbage collect : un GC
     * afficherait un écran de l'OS en plein lancement. */
    if (ti_ArchiveHasRoom(sizeof f + 32))
        archived = ti_SetArchiveStatus(true, h) != 0;
    ti_Close(h);
    return archived;
}

bool state_read_launching(char name_out[9])
{
    st_file_t f;
    uint8_t h = ti_Open(ST_NAME, "r");
    bool ok;
    if (!h) return false;
    ok = (ti_Read(&f, sizeof f, 1, h) == 1) && f.magic == ST_MAGIC;
    ti_Close(h);
    if (ok) { memcpy(name_out, f.name, 8); name_out[8] = '\0'; }
    return ok;
}

void state_clear(void)
{
    ti_Delete(ST_NAME);
}
