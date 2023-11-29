#ifndef RENDER_H
#define RENDER_H

#include "structures.h"
#include "vector.h"
#include <SDL2/SDL.h>

// Structure to store the result of the closest intersection calculation
struct closeInt {
    struct sphere closestSphere;
    float closeHit;
};

struct solutions {
                float sol1;
                float sol2;
            };

// typedef struct {
//     struct vec3 camPos;
//     int frame;
//     int startX, endX; // Start and end positions for x-coordinate
//     int startY, endY; // Start and end positions for y-coordinate
// } ThreadData;


// Translates pixel coordinates to viewport coordinates
struct vec3 viewportCoord(int x, int y);

// Calculates if a ray from a point intersects with a sphere
struct solutions solveRaySphere(struct vec3 vecStart, struct vec3 vecEnd, struct sphere sphere);

// Finds the closest intersection of a ray with any sphere
struct closeInt closeIntFunc(struct vec3 home, struct vec3 hit, float lowLim, float highLim);

// Computes the total light intensity at a point
float compLight(struct vec3 intersect, struct vec3 normal, struct vec3 vectToView, struct sphere closeSphere);

// Traces a ray from the camera to the viewport
Uint32 traceRay(struct vec3 rayOrigin, struct vec3 vectToView, float lowLim, float highLim, int depth);

// Calculates and stores the required colour values for each pixel
void placePixels(struct vec3 camPos, int frame);

#endif
