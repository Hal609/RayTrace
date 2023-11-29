#ifndef SDL_SETUP_H
#define SDL_SETUP_H

#include "vector.h"
#include "render.h"
#include "SDL2/SDL.h"

// SDL window and renderer declarations
extern SDL_Window* window;
extern SDL_Renderer* renderer;

// Function declarations

// Initializes the SDL window and renderer
void init();

// Call this function with the number of points you need to allocate
void allocateResources(int numPoints);

// Renders the first set of pixel colors to the renderer
void renderToTexture(SDL_Texture *texture, int frameCount);

void freeResources(SDL_Texture *texture);

#endif
