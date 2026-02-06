/*******************************************************************************************
*
*   raylib - classic game: floppy
*
*   Sample game developed by Ian Eito, Albert Martos and Ramon Santamaria
*
*   This game has been created using raylib v1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define MAX_TUBES 100
#define FLOPPY_RADIUS 24
#define TUBES_WIDTH 70
#define MAX_COINS 30
#define COIN_RADIUS 8

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum GameState {
    MENU = 0,
    PLAYING = 1,
    GAME_OVER = 2
} GameState;

typedef struct Floppy {
    Vector2 position;
    float velocity;
    int radius;
    Color color;
} Floppy;

typedef struct Tubes {
    Rectangle rec;
    Color color;
    bool active;
} Tubes;

typedef struct Coin {
    Vector2 position;
    int radius;
    bool active;
} Coin;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

static GameState gameState = MENU;
static bool pause = false;
static int score = 0;
static int hiScore = 0;
static int coinsCollected = 0;

static Floppy floppy = { 0 };
static Tubes tubes[MAX_TUBES*2] = { 0 };
static Vector2 tubesPos[MAX_TUBES] = { 0 };
static Coin coins[MAX_COINS] = { 0 };
static int tubesSpeedX = 0;
static bool superfx = true;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "classic game: floppy");

    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update and Draw
        //----------------------------------------------------------------------------------
        UpdateDrawFrame();
        //----------------------------------------------------------------------------------
    }
#endif
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadGame();         // Unload loaded data (textures, sounds, models...)

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
//------------------------------------------------------------------------------------
// Module Functions Definitions (local)
//------------------------------------------------------------------------------------

// Initialize game variables
void InitGame(void)
{
    floppy.radius = FLOPPY_RADIUS;
    floppy.position = (Vector2){80, screenHeight/2 - floppy.radius};
    floppy.velocity = 0.0f;
    tubesSpeedX = 3.2;

    for (int i = 0; i < MAX_TUBES; i++)
    {
        tubesPos[i].x = 400 + 280*i;
        tubesPos[i].y = -GetRandomValue(0, 120);
    }

    for (int i = 0; i < MAX_TUBES*2; i += 2)
    {
        tubes[i].rec.x = tubesPos[i/2].x;
        tubes[i].rec.y = tubesPos[i/2].y;
        tubes[i].rec.width = TUBES_WIDTH;
        tubes[i].rec.height = 200;

        tubes[i+1].rec.x = tubesPos[i/2].x;
        tubes[i+1].rec.y = 600 + tubesPos[i/2].y - 255;
        tubes[i+1].rec.width = TUBES_WIDTH;
        tubes[i+1].rec.height = 200;

        tubes[i/2].active = true;
    }

    score = 0;
    coinsCollected = 0;

    
    for (int i = 0; i < MAX_COINS; i++)
    {
        coins[i].radius = COIN_RADIUS;
        coins[i].position.x = 400 + 280*i + 175 + GetRandomValue(-80, 80);  
        coins[i].position.y = 80 + GetRandomValue(100, 180); 
        coins[i].active = true;
    }

    
    

    superfx = false;
    pause = false;
}

// Update game (one frame)
void UpdateGame(void)
{
    if (gameState == MENU)
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            InitGame();
            gameState = PLAYING;
        }
    }
    else if (gameState == PLAYING)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            for (int i = 0; i < MAX_TUBES; i++) tubesPos[i].x -= tubesSpeedX;

            for (int i = 0; i < MAX_TUBES*2; i += 2)
            {
                tubes[i].rec.x = tubesPos[i/2].x;
                tubes[i+1].rec.x = tubesPos[i/2].x;
            }

            // Move coins with tubes
            for (int i = 0; i < MAX_COINS; i++)
            {
                coins[i].position.x -= tubesSpeedX;
            }

            
            if (IsKeyPressed(KEY_SPACE) && gameState == PLAYING) floppy.velocity = -6.5f;

            // Gravity
            floppy.velocity += 0.35f;

            
            floppy.position.y += floppy.velocity;

            // Clamp to top of screen
            if (floppy.position.y < floppy.radius)
            {
                floppy.position.y = floppy.radius;
                floppy.velocity = 0.0f;
            }

            // If floppy hits the ground, end game (optional)
            if (floppy.position.y > screenHeight - floppy.radius)
            {
                floppy.position.y = screenHeight - floppy.radius;
                gameState = GAME_OVER;
            }

            // Check Collisions
            for (int i = 0; i < MAX_TUBES*2; i++)
            {
                if (CheckCollisionCircleRec(floppy.position, floppy.radius, tubes[i].rec))
                {
                    gameState = GAME_OVER;
                    pause = false;
                }
                else if ((tubesPos[i/2].x < floppy.position.x) && tubes[i/2].active && gameState == PLAYING)
                {
                    score += 100;
                    tubes[i/2].active = false;

                    superfx = true;

                    if (score > hiScore) hiScore = score;
                }
            }

            
            for (int i = 0; i < MAX_COINS; i++)
            {
                if (coins[i].active && CheckCollisionCircles(floppy.position, floppy.radius, coins[i].position, coins[i].radius))
                {
                    coins[i].active = false;
                    coinsCollected++;
                }
            }
        }
    }
    else if (gameState == GAME_OVER)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            gameState = MENU;
        }
    }
}

// Draw game (one frame)
void DrawGame(void)
{
    BeginDrawing();

        ClearBackground(RAYWHITE);

        if (gameState == MENU)
        {
            DrawText("FLOPPY BIRD", screenWidth/2 - MeasureText("FLOPPY BIRD", 60)/2, 50, 60, DARKGRAY);
            DrawText("Press SPACE to Start", screenWidth/2 - MeasureText("Press SPACE to Start", 40)/2, 200, 40, GRAY);
            DrawText("Controls: SPACE to jump, P to pause", screenWidth/2 - MeasureText("Controls: SPACE to jump, P to pause", 20)/2, 300, 20, LIGHTGRAY);
            DrawText(TextFormat("High Score: %04i", hiScore), screenWidth/2 - MeasureText(TextFormat("High Score: %04i", hiScore), 30)/2, 380, 30, GRAY);
        }
        else if (gameState == PLAYING)
        {
            DrawCircle(floppy.position.x, floppy.position.y, floppy.radius, DARKGRAY);

            // Draw tubes
            for (int i = 0; i < MAX_TUBES; i++)
            {
                DrawRectangle(tubes[i*2].rec.x, tubes[i*2].rec.y, tubes[i*2].rec.width, tubes[i*2].rec.height, GRAY);
                DrawRectangle(tubes[i*2 + 1].rec.x, tubes[i*2 + 1].rec.y, tubes[i*2 + 1].rec.width, tubes[i*2 + 1].rec.height, GRAY);
            }

            // Draw coins
            for (int i = 0; i < MAX_COINS; i++)
            {
                if (coins[i].active)
                {
                    DrawCircle(coins[i].position.x, coins[i].position.y, coins[i].radius, GOLD);
                }
            }

            

            DrawText(TextFormat("%04i", score), 20, 20, 40, GRAY);
            DrawText(TextFormat("HI-SCORE: %04i", hiScore), 20, 70, 20, LIGHTGRAY);
            DrawText(TextFormat("COINS: %04i", coinsCollected), 20, 120, 20, GOLD);

            if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, GRAY);
        }
        else if (gameState == GAME_OVER)
        {
            DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 150});
            DrawText("GAME OVER", screenWidth/2 - MeasureText("GAME OVER", 60)/2, 80, 60, RED);
            DrawText(TextFormat("Score: %04i", score), screenWidth/2 - MeasureText(TextFormat("Score: %04i", score), 40)/2, 200, 40, GRAY);
            DrawText(TextFormat("Coins: %04i", coinsCollected), screenWidth/2 - MeasureText(TextFormat("Coins: %04i", coinsCollected), 40)/2, 240, 40, GOLD);
            DrawText(TextFormat("High Score: %04i", hiScore), screenWidth/2 - MeasureText(TextFormat("High Score: %04i", hiScore), 40)/2, 280, 40, GRAY);
            DrawText("Press ENTER to return to menu", screenWidth/2 - MeasureText("Press ENTER to return to menu", 20)/2, 380, 20, LIGHTGRAY);
        }

    EndDrawing();
}

// Unload game variables
void UnloadGame(void)
{
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}

// Update and Draw (one frame)
void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}