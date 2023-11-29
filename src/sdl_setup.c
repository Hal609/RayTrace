#include "sdl_setup.h"
#include <stdio.h>
#include "SDL2/SDL.h"
#include "config.h"


// Define the global SDL variables
SDL_Window* window;
SDL_Renderer* renderer;

// Dynamic arrays for storing points and colors
SDL_Point* points = NULL;
Uint32* colours = NULL;

// Define the lengths of the points arrays
int pointsLen = 0;

// Initializes the SDL window and renderer
void init() {
    // Initializes SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    // Creates an SDL window
    window = SDL_CreateWindow("Hal_Raytrace",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, // Window position
                              height * ratio * scale, height * scale, // Window size (x, y)
                              SDL_WINDOW_RESIZABLE); // Window flags
    if (!window) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    // Creates an SDL renderer
    Uint32 renderFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    renderer = SDL_CreateRenderer(window, -1, renderFlags);
    if (!renderer) {
        fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }

    // Sets the scaling of the renderer
    SDL_RenderSetScale(renderer, scale, scale);
}

// Call this function with the number of points you need to allocate
void allocateResources(int numPoints) {
    pointsLen = numPoints;
    points = (SDL_Point*)malloc(sizeof(SDL_Point) * pointsLen);
    colours = (Uint32*)malloc(sizeof(Uint32) * pointsLen);
    if (!points || !colours) {
        fprintf(stderr, "Failed to allocate memory for points or colours\n");
        exit(1);
    }
}

void freeResources(SDL_Texture *texture) {
    free(points);
    free(colours);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


// Renders the first set of pixel colors to the renderer
void renderToTexture(SDL_Texture *texture, int frameCount) {
    SDL_SetRenderTarget(renderer, texture);  // Set the texture as the current render target
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set background color
    SDL_RenderClear(renderer); // Clear the render target

    for (int i = 0; i < pointsLen; ++i) {
        SDL_SetRenderDrawColor(renderer, 
            (colours[i] >> 16) & 0xFF, // Red
            (colours[i] >> 8) & 0xFF,  // Green
            colours[i] & 0xFF,         // Blue
            255);                      // Alpha
        SDL_RenderDrawPoint(renderer, points[i].x, points[i].y);
    }


    SDL_SetRenderTarget(renderer, NULL); // Reset render target to the default (screen)
    SDL_RenderCopy(renderer, texture, NULL, NULL); // Copy the texture to the screen
    SDL_RenderPresent(renderer); // Update the screen
}

