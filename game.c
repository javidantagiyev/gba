#include "myLib.h"
#include "game.h"
#include "firstCollision.h"
#include "pelletMap.h"
#include "spritesheet.h"
#include "firstLevelBackground.h"

#define GHOSTCOUNT 4

// Variables
OBJ_ATTR shadowOAM[128];
ANISPRITE pacman;
GHOST ghost[4];
unsigned char* firstCollision = firstCollisionBitmap;

int collisionCheck;
int ghostCollide;
int lives;
int score;
int powerUpTime;
int pelletsRemaining = 259;

// Pacman and ghost animation states for aniState
enum {PACRIGHT, PACLEFT, PACUP, PACDOWN, PACIDLE};
enum {RIGHT, LEFT, UP, DOWN};

// Initialize the game
void initGame() {
    initPlayer();
    initGhost();
    //pelletCollisionCheck(0,0,0);
    ghostCollide = 0;
    lives = 3;
    score = 0;
    DMANow(3, pelletMapMap, &SCREENBLOCK[30], pelletMapMapLen / 2);
    DMANow(3, pelletMapTiles, &CHARBLOCK[2], pelletMapTilesLen / 2);
}

// Updates the game each frame
void updateGame() {

	updatePlayer();
    for (int i = 0; i < GHOSTCOUNT; i++) {
        updateGhost(&ghost[i]);
    }

    if (pelletsRemaining == 0) {
        DMANow(3, pelletMapMap, &SCREENBLOCK[30], pelletMapMapLen / 2);
        DMANow(3, pelletMapTiles, &CHARBLOCK[2], pelletMapTilesLen / 2);
        DMANow(3, firstLevelBackgroundTiles, &CHARBLOCK[0], firstLevelBackgroundTilesLen / 2);
        DMANow(3, firstLevelBackgroundMap, &SCREENBLOCK[31], firstLevelBackgroundMapLen / 2);
    }
}

// Draws the game each frame
void drawGame() {

    drawPlayer();
    drawGhost();
    drawLives();
    drawScore();

    waitForVBlank();
    DMANow(3, shadowOAM, OAM, 128 * 4);

}

// Initialize the player
void initPlayer() {

    pacman.width = 8;
    pacman.height = 8;
    pacman.rdel = 1;
    pacman.cdel = 1;

    //Place in desired location
    pacman.screenRow = 128;
    pacman.screenCol = 120;
    pacman.aniCounter = 0;
    pacman.curFrame = 0;
    pacman.numFrames = 3;
    pacman.aniState = PACLEFT;
    pacman.lives = 3;
}

// Handle every-frame actions of the player
void updatePlayer() {

    if (powerUpTime > 0) {
        powerUpTime--;
    }

    if (SCREENBLOCK[30].tilemap[OFFSET((pacman.screenCol + 3) / 8, (pacman.screenRow + 3) / 8, 256 / 8)] == 0x000A) {
        pelletCollisionCheck((pacman.screenCol + 3) / 8, (pacman.screenRow + 3) / 8, 0);
        pelletsRemaining--;
    } else if (SCREENBLOCK[30].tilemap[OFFSET((pacman.screenCol + 3) / 8, (pacman.screenRow + 3) / 8, 256 / 8)] == 0x000B) {
        pelletCollisionCheck((pacman.screenCol + 3) / 8, (pacman.screenRow + 3) / 8, 1);
        pelletsRemaining--;
    }
    
    if(BUTTON_HELD(BUTTON_UP)) {
        if (firstCollision[OFFSET(pacman.screenCol, pacman.screenRow - pacman.rdel, MAPWIDTH)] //top left
                && firstCollision[OFFSET(pacman.screenCol + pacman.width - 1, pacman.screenRow - pacman.rdel, MAPWIDTH)]) //top right
            {
            
            // Update pacman's screen position if the above is true
            pacman.screenRow -= pacman.rdel;
        }
    }
    if(BUTTON_HELD(BUTTON_DOWN)) {
        if (firstCollision[OFFSET(pacman.screenCol, pacman.screenRow + pacman.rdel + pacman.height - 1, MAPWIDTH)] //bottom left
            && firstCollision[OFFSET(pacman.screenCol + pacman.width - 1, pacman.screenRow + pacman.rdel + pacman.height - 1, MAPWIDTH)]) //bottom right
            {

            // Update pacman's screen position if the above is true
            pacman.screenRow += pacman.rdel;
        }
    }
    if(BUTTON_HELD(BUTTON_LEFT)) {
        if (firstCollision[OFFSET(pacman.screenCol - pacman.cdel, pacman.screenRow, MAPWIDTH)]
            && firstCollision[OFFSET(pacman.screenCol - pacman.cdel, pacman.screenRow + pacman.height - 1, MAPWIDTH)])
            {

            // Update pacman's screen position if the above is true
            pacman.screenCol -= pacman.cdel;
        }
    }
    if(BUTTON_HELD(BUTTON_RIGHT)) {
        if (firstCollision[OFFSET(pacman.screenCol + pacman.width - 1 - pacman.cdel, pacman.screenRow, MAPWIDTH)]
            && firstCollision[OFFSET(pacman.screenCol + pacman.cdel + pacman.width - 1, pacman.screenRow + pacman.height - 1, MAPWIDTH)])
            {

            // Update pacman's screen position if the above is true
            pacman.screenCol += pacman.cdel;
        }
    }

     animatePlayer();

     //collision check
     for (int i = 0; i < GHOSTCOUNT; i++) {
         if (collision(pacman.screenCol, pacman.screenRow, pacman.width, pacman.height, 
         ghost[i].screenCol, ghost[i].screenRow, ghost[i].width, ghost[i].height)) {
             if (powerUpTime) {
                 ghost[i].screenCol = 120;
                 ghost[i].screenRow = 80;
                 score += 50;
             } else {
                pacman.lives--;
                pacman.screenRow = 128;
                pacman.screenCol = 120;
             }
         }
     }
}

// Handle player animation states
void animatePlayer() {

    // Set previous state to current state
    pacman.prevAniState = pacman.aniState;
    pacman.aniState = PACIDLE;

    // Change the animation frame every 5 frames of gameplay
    if(pacman.aniCounter % 5 == 0) {
        pacman.curFrame = (pacman.curFrame + 1) % pacman.numFrames;
    }

    // Control movement and change animation state
    if(BUTTON_HELD(BUTTON_UP))
        pacman.aniState = PACUP;
    if(BUTTON_HELD(BUTTON_DOWN))
        pacman.aniState = PACDOWN;
    if(BUTTON_HELD(BUTTON_LEFT))
        pacman.aniState = PACLEFT;
    if(BUTTON_HELD(BUTTON_RIGHT))
        pacman.aniState = PACRIGHT;

    // If the pacman aniState is idle, frame is pacman standing
    if (pacman.aniState == PACIDLE) {
        pacman.curFrame = 0;
        pacman.aniCounter = 0;
        pacman.aniState = pacman.prevAniState;
    } else {
        pacman.aniCounter++;
    }
}

// Draw the player
void drawPlayer() {

    if (pacman.hide) {
        shadowOAM[0].attr0 |= ATTR0_HIDE;
    } else {
        shadowOAM[0].attr0 = (ROWMASK & pacman.screenRow) | ATTR0_SQUARE;
        shadowOAM[0].attr1 = (COLMASK & pacman.screenCol) | ATTR1_TINY;
        shadowOAM[0].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(pacman.aniState, pacman.curFrame);
    }
}

void initGhost() {
    for (int i = 0; i < GHOSTCOUNT; i++) {
        ghost[i].screenRow = 80;
        ghost[i].screenCol = 104 + (i + 8);
        ghost[i].rdel = -1;
        ghost[i].cdel = 0;
        ghost[i].width = 8;
        ghost[i].height = 8;
        ghost[i].aniCounter = 0;
        ghost[i].aniState = UP;
        ghost[i].prevAniState = 0;
        ghost[i].currFrame = 0;
        ghost[i].numFrames = 2;
        ghost[i].active = 1;
        ghost[i].color = i;
        ghost[i].collisionCheck = 0;
    }
    // ghost[0].screenRow = 64;
    // ghost[0].screenCol = 120;
}

void drawGhost() {
    for (int i = 0; i < GHOSTCOUNT; i++) {
        shadowOAM[i + 1].attr0 = (ROWMASK & ghost[i].screenRow) | ATTR0_SQUARE;
        shadowOAM[i + 1].attr1 = (COLMASK & ghost[i].screenCol) | ATTR1_TINY;

        if (powerUpTime != 0) {
            shadowOAM[i + 1].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(12, ghost[i].currFrame);
        } else {
            shadowOAM[i + 1].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(4 + (ghost[i].currFrame) + ghost[i].aniState * 2, ghost[i].color);
        }
    }
}

void updateGhost(GHOST* ghost) {
    if (ghost->aniState == DOWN && !(firstCollision[OFFSET(ghost->screenCol, ghost->screenRow + ghost->height, MAPWIDTH)]
        && firstCollision[OFFSET(ghost->screenCol + ghost->width - 1, ghost->screenRow + ghost->height, MAPWIDTH)])) {
            ghost->collisionCheck = 1;
        }
    if (ghost->aniState == UP && !(firstCollision[OFFSET(ghost->screenCol, ghost->screenRow - 1, MAPWIDTH)]
        && firstCollision[OFFSET(ghost->screenCol + ghost->width - 1, ghost->screenRow - 1, MAPWIDTH)])) {
            ghost->collisionCheck = 1;
        }
    if (ghost->aniState == LEFT && !(firstCollision[OFFSET(ghost->screenCol - 1, ghost->screenRow, MAPWIDTH)]
        && firstCollision[OFFSET(ghost->screenCol - 1, ghost->screenRow + ghost->height - 1, MAPWIDTH)])) {
            ghost->collisionCheck = 1;
        }
    if (ghost->aniState == RIGHT && !(firstCollision[OFFSET(ghost->screenCol + ghost->width, ghost->screenRow, MAPWIDTH)]
        && firstCollision[OFFSET(ghost->screenCol + ghost->width, ghost->screenRow + ghost->height - 1, MAPWIDTH)])) {
            ghost->collisionCheck = 1;
        }
    
    while (ghost->collisionCheck) {
        ghost->aniState = rand() % 4;
        if (ghost->aniState == UP && firstCollision[OFFSET(ghost->screenCol, ghost->screenRow - 1, MAPWIDTH)]
            && firstCollision[OFFSET(ghost->screenCol + ghost->width - 1, ghost->screenRow - 1, MAPWIDTH)]) {
                ghost->aniState = UP;
                ghost->collisionCheck = 0;
                ghost->rdel = -1;
                ghost->cdel = 0;
        }

        if (ghost->aniState == DOWN && firstCollision[OFFSET(ghost->screenCol, ghost->screenRow + ghost->height, MAPWIDTH)]
            && firstCollision[OFFSET(ghost->screenCol + ghost->width - 1, ghost->screenRow + ghost->height, MAPWIDTH)]) {
                ghost->aniState = DOWN;
                ghost->collisionCheck = 0;
                ghost->rdel = 1;
                ghost->cdel = 0;
        }

        if (ghost->aniState == LEFT && firstCollision[OFFSET(ghost->screenCol - 1, ghost->screenRow, MAPWIDTH)]
            && firstCollision[OFFSET(ghost->screenCol - 1, ghost->screenRow + ghost->height - 1, MAPWIDTH)]) {
                ghost->aniState = LEFT;
                ghost->collisionCheck = 0;
                ghost->rdel = 0;
                ghost->cdel = -1;
        }
        
        if (ghost->aniState == RIGHT && firstCollision[OFFSET(ghost->screenCol + ghost->width, ghost->screenRow, MAPWIDTH)]
            && firstCollision[OFFSET(ghost->screenCol + ghost->width, ghost->screenRow + ghost->height - 1, MAPWIDTH)]) {
                ghost->aniState = RIGHT;
                ghost->collisionCheck = 0;
                ghost->rdel = 0;
                ghost->cdel = 1;
        }
    }
    ghost->aniCounter++;
    if (ghost->aniCounter % 10 == 0) {
        ghost->currFrame++;
        if (ghost->currFrame == ghost->numFrames) {
            ghost->currFrame = 0;
        }
    }

    ghost->screenRow += ghost->rdel;
    ghost->screenCol += ghost->cdel;
}

void pelletCollisionCheck(int tileCol, int tileRow, int power) {
    SCREENBLOCK[31].tilemap[OFFSET(tileCol, tileRow, 256 / 8)] = 0x0000; //NormalMap empty tile entry
    SCREENBLOCK[30].tilemap[OFFSET(tileCol, tileRow, 256 / 8)] = 0x0000; //PelMap entry tile entry

    if (power) {
        score += 20;
        powerUpTime = 300;
    } else {
        score += 10;
    }
}

void drawLives() {
    shadowOAM[5].attr0 = 3 | ATTR0_REGULAR | ATTR0_SQUARE;
    shadowOAM[5].attr1 = 50 | ATTR1_TINY;
    shadowOAM[5].attr2 = ATTR2_PALROW(0) | ATTR2_TILEID(pacman.lives, 4);
}

void drawScore() {
    for (int i = 0; i < 4; i++) {
        shadowOAM[6 + i].attr0 = 2 | ATTR0_REGULAR | ATTR0_SQUARE;
        shadowOAM[6 + i].attr1 = 207 + (8 * i) | ATTR1_TINY;
    }
    shadowOAM[6].attr2 = ATTR2_TILEID(score / 1000, 4);
    shadowOAM[7].attr2 = ATTR2_TILEID((score % 1000) / 100, 4);
    shadowOAM[8].attr2 = ATTR2_TILEID((score % 100) / 10, 4);
    shadowOAM[9].attr2 = ATTR2_TILEID(score % 10, 4);
}
