#include "sdl_setup.h"
#include "render.h"
#include <SDL2/SDL.h>
#include "structures.h"
#include "config.h"

// Global variables
struct vec3 camPos = {0, 1, 0}; // Example camera position
int frameCount = 0;

float height = 400;
int scale = 1;
float ratio = 1.2; // Aspect ratio
float viewHeight = 1;
float viewZ = 0.9;

// Initialize SDL and create window and renderer
void initSDL() {
    init(); // This calls the initialization from sdl_setup.h

    // Allocate resources for points and colors
    allocateResources(height * (height * ratio)); // Replace YOUR_NUMBER_OF_POINTS with actual value
}

int main(int argc, char *argv[]) {
    initSDL();

    // Main loop flag and event handler
    int quit = 0;
    SDL_Event e;

    // Camera step for movement
    struct vec3 camStep = {0.005, 0.005, 0.01};

    // Texture for rendering to file
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, height*ratio, height);
    SDL_SetRenderTarget(renderer, texture);

    // Main loop
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }

        // Update camera position
        camPos = addVect(camPos, camStep);

        // Render logic
        pointsLen = 0;
        placePixels(camPos, 1);
        renderToTexture(texture, frameCount); // Use the optimized rendering function

        // Increment frame count
        frameCount++;
    }

    // Free resources and close SDL
    freeResources(texture);

    return 0;
}

