#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <SDL2/SDL.h>
#include "vector.h"

// Structure for a sphere in the scene
struct sphere {
    float radius;
    struct vec3 centre;
    Uint32 color;
    int specExp;
    float reflect;
};

// Structure for a light source in the scene
struct light {
    char style; // 'a' for ambient, 'd' for directional, 'p' for point
    float intensity;
    struct vec3 direction;
    struct vec3 position;
};

// Arrays for storing points and colors for rendering
extern SDL_Point *points;
extern Uint32 *colours;

// Lengths of the points arrays
extern int pointsLen;

// structures.h
extern struct sphere spheres[];
extern struct light lights[];

#endif
