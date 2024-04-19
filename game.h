// Constants
#define MAPHEIGHT 160
#define MAPWIDTH 240

// Variables
extern OBJ_ATTR shadowOAM[128];
extern ANISPRITE pacman;
extern int lives;
extern int score;

// Prototypes
void initGame();
void updateGame();
void drawGame();
void initPlayer();
void updatePlayer();
void animatePlayer();
void drawPlayer();

//struct for ghost
typedef struct {
    int screenRow;
    int screenCol;
    int rdel;
    int cdel;
    int width;
    int height;
    int aniCounter;
    int aniState;
    int prevAniState;
    int currFrame;
    int numFrames;
    int active;
    int color;
    int collisionCheck;
} GHOST;