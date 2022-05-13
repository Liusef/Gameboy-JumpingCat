#ifndef MAIN_H
#define MAIN_H

#include "gba.h"

struct player {
    int row;
    int col;
    u8 onGround;
};

struct platform {
    int col;
    int width;
    int height;
};

inline int platCollides(struct player player, struct platform plat, int gh, int platgh);
inline int platRow(int gh, int platgh);

#define SPLASH_INTERVAL (10)
#define SPLASH_WAIT (180)

#define FXP (1 << 8)

#define GRAVITY (1100)
#define SPEED (4)

#define PLAYER_SIZE (10)

#define PLAT_W (24)
#define PLAT_H (3)
#define GH_MULT (16)

#define PLATS_LEN (129)

#define SCROLL_SPEED (4)

inline int platRow(int gh, int platgh) {
    return HEIGHT - GH_MULT * (platgh - gh);
}

inline int platCollides(struct player player, struct platform plat, int gh, int platgh) {
    return    player.col / FXP <= plat.col + plat.width
           && player.col / FXP + PLAYER_SIZE >= plat.col
           && player.row / FXP <= platRow(gh, platgh) + 1
           && player.row / FXP + PLAYER_SIZE >= platRow(gh, platgh);
}


#endif
