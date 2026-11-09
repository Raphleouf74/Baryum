#ifndef ANIM_H
#define ANIM_H
/* Interpolation en virgule fixe : progression p dans [0..256]. */

/* Progression 0..256 avec ease-out cubique pour t dans [0..d]. */
static inline int ease_out_cubic(int t, int d)
{
    int u;
    if (t >= d) return 256;
    u = 256 - (t * 256) / d;
    return 256 - (((u * u) >> 8) * u >> 8);
}

/* Interpolation linéaire entre a et b avec p dans [0..256]. */
static inline int lerp256(int a, int b, int p)
{
    return a + ((b - a) * p) / 256;
}
#endif
