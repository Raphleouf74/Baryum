#include "input.h"
#include <keypadc.h>

#define REPEAT_DELAY 14   /* images avant la 1re répétition */
#define REPEAT_RATE  4    /* images entre répétitions */

static uint8_t prev_raw;
static uint8_t prev_del, prev_graph, prev_alpha;
static uint8_t ev2;
static uint8_t hold;

static uint8_t read_raw(void)
{
    uint8_t r = 0;
    kb_Scan();
    if (kb_Data[7] & kb_Up)    r |= EV_UP;
    if (kb_Data[7] & kb_Down)  r |= EV_DOWN;
    if (kb_Data[7] & kb_Left)  r |= EV_LEFT;
    if (kb_Data[7] & kb_Right) r |= EV_RIGHT;
    if ((kb_Data[1] & kb_2nd) || (kb_Data[6] & kb_Enter)) r |= EV_OK;
    if (kb_Data[1] & kb_Mode)  r |= EV_THEME;
    if (kb_Data[1] & kb_Yequ)  r |= EV_ACCENT;
    if (kb_Data[6] & kb_Clear) r |= EV_QUIT;
    return r;
}

uint8_t input_poll2(void) { return ev2; }

uint8_t input_poll(void)
{
    uint8_t raw = read_raw();
    uint8_t del = (kb_Data[1] & kb_Del) ? 1 : 0;   /* kb_Scan deja fait dans read_raw */
    uint8_t graph = (kb_Data[1] & kb_Graph) ? 1 : 0;
    uint8_t alpha = (kb_Data[2] & kb_Alpha) ? 1 : 0;
    ev2 = ((del && !prev_del) ? EV2_RETURN : 0) | ((graph && !prev_graph) ? EV2_MODE : 0)
        | ((alpha && !prev_alpha) ? EV2_INFO : 0);
    prev_del = del;
    prev_graph = graph;
    prev_alpha = alpha;
    uint8_t ev = raw & (uint8_t)~prev_raw;           /* front montant */
    uint8_t arrows = EV_UP | EV_DOWN | EV_LEFT | EV_RIGHT;

    if (raw & arrows) {
        if (raw == prev_raw) {
            hold++;
            if (hold >= REPEAT_DELAY && ((hold - REPEAT_DELAY) % REPEAT_RATE) == 0)
                ev |= raw & arrows;
        } else {
            hold = 0;
        }
    } else {
        hold = 0;
    }
    prev_raw = raw;
    return ev;
}
