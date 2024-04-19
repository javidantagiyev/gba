#include <stdlib.h>
#include <stdio.h>
#include "myLib.h"
#include "start.h"
#include "pause.h"
#include "win.h"
#include "lose.h"
#include "game.h"
#include "spritesheet.h"
#include "firstLevelBackground.h"

// Prototypes
void initialize();

// State Prototypes
void goToStart();
void start();
void goToGame();
void game();
void goToPause();
void pause();
void goToWin();
void win();
void goToLose();
void lose();

// States
enum
{
    START,
    GAME,
    PAUSE,
    WIN,
    LOSE
};
int state;
int highScore;

// Button Variables
unsigned short buttons;
unsigned short oldButtons;

// Shadow OAM
OBJ_ATTR shadowOAM[128];

int main()
{
    initialize();

    while (1)
    {
        // Update button variables
        oldButtons = buttons;
        buttons = BUTTONS;

        // State Machine
        switch (state)
        {
        case START:
            start();
            break;
        case GAME:
            game();
            break;
        case PAUSE:
            pause();
            break;
        case WIN:
            win();
            break;
        case LOSE:
            lose();
            break;
        }
    }
}

// Sets up GBA
void initialize()
{
    REG_DISPCTL = MODE0 | BG1_ENABLE | SPRITE_ENABLE; 
    REG_BG1CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(31) | BG_SIZE_SMALL;

    buttons = BUTTONS;
    oldButtons = 0;

    goToStart();
}

// Sets up the start state
void goToStart() {
    
    DMANow(3, startPal, PALETTE, 256);
    DMANow(3, startTiles, &CHARBLOCK[0], startTilesLen / 2);
    DMANow(3, startMap, &SCREENBLOCK[31], startMapLen / 2);
    state = START;
}

// Runs every frame of the start state
void start() {
    if (BUTTON_PRESSED(BUTTON_START)) {
        initGame();
        goToGame();
    }
}

// Sets up the game state
void goToGame() {
    //sprites
    DMANow(3, spritesheetTiles, &CHARBLOCK[4], spritesheetTilesLen / 2);
    DMANow(3, spritesheetPal, SPRITEPALETTE, spritesheetPalLen / 2);

    //update background to be level 1 
    DMANow(3, firstLevelBackgroundPal, PALETTE, firstLevelBackgroundPalLen / 2);
    DMANow(3, firstLevelBackgroundTiles, &CHARBLOCK[0], firstLevelBackgroundTilesLen / 2);
    DMANow(3, firstLevelBackgroundMap, &SCREENBLOCK[31], firstLevelBackgroundMapLen / 2);

    hideSprites();
    DMANow(3, shadowOAM, OAM, 128 * 4);

    for (int i = 0; i < 30; i++) {
        for (int j = 1; j < 20; j++) {
            if (SCREENBLOCK[30].tilemap[OFFSET(i, j, 256 / 8)] == 0x0000) {
                SCREENBLOCK[31].tilemap[OFFSET(i, j, 256 / 8)] = 0x0000;
            }
        }
    }

    state = GAME;
}

// Runs every frame of the game state
void game() {
    updateGame();
    drawGame();

    if (BUTTON_PRESSED(BUTTON_START)) {
        hideSprites();
        DMANow(3, shadowOAM,OAM, 128 * 4);
        goToPause();
    } else if (BUTTON_PRESSED(BUTTON_A)) { //win condition
        hideSprites();
        DMANow(3, shadowOAM,OAM, 128 * 4);
        goToWin();
    } else if (pacman.lives == 0) { //lose condition
        hideSprites();
        DMANow(3, shadowOAM,OAM, 128 * 4);
        goToLose();
    }
}

// Sets up the pause state
void goToPause() {
    DMANow(3, pausePal, PALETTE, 256);
    DMANow(3, pauseTiles, &CHARBLOCK[0], pauseTilesLen / 2);
    DMANow(3, pauseMap, &SCREENBLOCK[31], pauseMapLen / 2);
    state = PAUSE;
}

// Runs every frame of the pause state
void pause() {
    if (BUTTON_PRESSED(BUTTON_START)) {
        goToGame();
    } else if (BUTTON_PRESSED(BUTTON_SELECT)) {
        goToStart();
    }
}

// Sets up the win state
void goToWin() {
    DMANow(3, winPal, PALETTE, 256);
    DMANow(3, winTiles, &CHARBLOCK[0], winTilesLen / 2);
    DMANow(3, winMap, &SCREENBLOCK[31], winMapLen / 2);
    state = WIN;
}

// Runs every frame of the win state
void win() {
    if (BUTTON_PRESSED(BUTTON_START)) {
        goToStart();
    }
}

// Sets up the lose state
void goToLose() {
    DMANow(3, losePal, PALETTE, 256);
    DMANow(3, loseTiles, &CHARBLOCK[0], loseTilesLen / 2);
    DMANow(3, loseMap, &SCREENBLOCK[31], loseMapLen / 2);
    state = LOSE;

    if (score > highScore) {
        highScore = score;
    }

    for (int i = 0; i < 4; i++) {
        shadowOAM[6 + i].attr0 = 45 | ATTR0_REGULAR | ATTR0_SQUARE;
        shadowOAM[6 + i].attr1 = (105 + (8 * i)) | ATTR1_TINY;
        shadowOAM[10 + i].attr0 = 136 | ATTR0_REGULAR | ATTR0_SQUARE;
        shadowOAM[10 + i].attr1 = (104 + (8 * i)) | ATTR1_TINY;
    }

    shadowOAM[6].attr2 = ATTR2_TILEID(score / 1000, 4);
    shadowOAM[7].attr2 = ATTR2_TILEID((score % 1000) / 100, 4);
    shadowOAM[8].attr2 = ATTR2_TILEID((score % 100) / 10, 4);
    shadowOAM[9].attr2 = ATTR2_TILEID(score % 10, 4);

    shadowOAM[10].attr2 = ATTR2_TILEID(highScore / 1000, 4);
    shadowOAM[11].attr2 = ATTR2_TILEID((highScore % 1000) / 100, 4);
    shadowOAM[12].attr2 = ATTR2_TILEID((highScore % 100) / 10, 4);
    shadowOAM[13].attr2 = ATTR2_TILEID(highScore % 10, 4);

    DMANow(3, shadowOAM, OAM, 128 * 4);
}

// Runs every frame of the lose state
void lose() {
    if (BUTTON_PRESSED(BUTTON_START)) {
        hideSprites();
        DMANow(3, shadowOAM, OAM, 128 * 4);
        goToStart();
    }
}