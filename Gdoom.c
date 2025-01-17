#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

// Define window size
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MAP_WIDTH 24
#define MAP_HEIGHT 24
#define FOV 3.14159 / 3  // 60 degrees field of view

// Map representation (1 = wall, 0 = empty space)
int worldMap[MAP_WIDTH][MAP_HEIGHT] = {
    // A 2D map representing the world (1 = wall, 0 = empty space)
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,0,0,0,0,0,1,0,0,0,1,1,0,0,1},
    {1,0,1,0,0,0,0,0,1,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1},
    {1,0,1,0,1,1,0,0,1,0,0,0,1,1,1,1,1,0,0,0,1,1,0,1},
    {1,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,0,1,1,0,1,1,0,1,1,1,1,0,1,0,0,1,0,1},
    {1,0,0,0,1,1,1,1,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,1},
    {1,0,1,1,0,1,0,1,1,0,1,0,0,0,0,1,0,0,0,0,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,1,1,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,1,0,0,0,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1},
    {1,0,0,1,0,0,0,0,1,0,0,0,1,1,1,1,1,1,0,0,0,0,0,1},
    {1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,0,0,1,1,1},
    {1,0,0,0,1,0,0,0,0,1,0,0,1,0,1,0,0,1,1,1,0,1,1,1},
    {1,0,1,0,0,0,0,0,1,1,0,1,1,1,1,0,0,0,1,1,1,0,0,1},
    {1,0,1,1,1,0,1,0,0,0,1,0,1,0,0,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,1,1,0,0,0,1,0,0,1,1,1,1,1,0,1,0,0,1},
    {1,1,1,1,1,1,1,0,1,1,1,0,0,0,1,1,0,1,0,1,0,1,1,1},
    {1,0,0,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,0,0,0,1,1},
    {1,0,1,0,1,0,1,0,1,1,0,0,1,0,0,1,0,1,0,1,1,1,1,1},
    {1,0,1,0,1,0,0,1,0,0,1,1,0,1,0,1,0,1,1,1,1,0,0,0},
    {1,0,1,0,1,1,0,0,0,1,1,1,0,0,0,1,0,0,0,0,1,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// Player position and direction
float posX = 22, posY = 12;  // Player start position
float dirX = -1, dirY = 0;    // Initial direction vector
float planeX = 0, planeY = 0.66; // Camera plane

// Function to draw the scene
void renderScene(SDL_Renderer* renderer) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        // Calculate ray position and direction
        float cameraX = 2 * x / (float)SCREEN_WIDTH - 1;  // Camera space X
        float rayDirX = dirX + planeX * cameraX;
        float rayDirY = dirY + planeY * cameraX;

        // Which box of the map we're in
        int mapX = (int)posX;
        int mapY = (int)posY;

        // Length of ray from current position to next x or y side
        float sideDistX, sideDistY;

        // Length of ray from one side to next in world space
        float deltaDistX = fabs(1 / rayDirX);
        float deltaDistY = fabs(1 / rayDirY);
        float perpWallDist;

        // Step and initial direction for ray
        int stepX, stepY;
        int hit = 0; // Was a wall hit?
        int side;     // Was the wall hit on the x or y side?

        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (posX - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0 - posX) * deltaDistX;
        }
        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (posY - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0 - posY) * deltaDistY;
        }

        // Perform DDA (Digital Differential Analyzer)
        while (hit == 0) {
            // Jump to next map square
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }

            // Check if ray has hit a wall
            if (worldMap[mapX][mapY] > 0) hit = 1;
        }

        // Calculate distance projected on camera direction (perpendicular distance)
        if (side == 0) perpWallDist = (mapX - posX + (1 - stepX) / 2) / rayDirX;
        else perpWallDist = (mapY - posY + (1 - stepY) / 2) / rayDirY;

        // Calculate height of line to draw on screen
        int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);

        // Calculate lowest and highest pixel to fill in current stripe
        int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;

        // Set color based on wall type
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red walls
        if (side == 1) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue for Y side walls
        }

        // Draw the vertical line of the wall slice
        SDL_RenderDrawLine(renderer, x, drawStart, x, drawEnd);
    }
}

// Handle player input for movement and rotation
void handleInput(SDL_Event* e) {
    const float moveSpeed = 0.1f;  // Movement speed
    const float rotSpeed = 0.05f;  // Rotation speed

    if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
            case SDLK_w:  // Move forward
                if (worldMap[(int)(posX + dirX * moveSpeed)][(int)(posY)] == 0) posX += dirX * moveSpeed;
                if (worldMap[(int)(posX)][(int)(posY + dirY * moveSpeed)] == 0) posY += dirY * moveSpeed;
                break;
            case SDLK_s:  // Move backward
                if (worldMap[(int)(posX - dirX * moveSpeed)][(int)(posY)] == 0) posX -= dirX * moveSpeed;
                if (worldMap[(int)(posX)][(int)(posY - dirY * moveSpeed)] == 0) posY -= dirY * moveSpeed;
                break;
            case SDLK_a:  // Rotate left
                {
                    float oldDirX = dirX;
                    dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
                    dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
                    float oldPlaneX = planeX;
                    planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
                    planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
                }
                break;
            case SDLK_d:  // Rotate right
                {
                    float oldDirX = dirX;
                    dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
                    dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
                    float oldPlaneX = planeX;
                    planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
                    planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
                }
                break;
        }
    }
}

int main(int argc, char* argv[]) {
    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("Raycasting Engine",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Main game loop
    int running = 1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            } else {
                handleInput(&e);  // Handle input
            }
        }

        // Render the scene
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set background color to black
        SDL_RenderClear(renderer);
        renderScene(renderer);
        SDL_RenderPresent(renderer);
    }

    // Clean up and close SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
