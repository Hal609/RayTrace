#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include "vector.h"
#include "structures.h"

// Reflects a ray around a given normal
struct vec3 reflectRay(struct vec3 inRay, struct vec3 mirrorRay);

// Multiplies a color value by a factor
Uint32 multiplyCol(Uint32 inputCol, float factor);

// Adds two color values
Uint32 addCol(Uint32 col1, Uint32 col2);

#endif