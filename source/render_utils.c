#include "vector.h"
#include "structures.h"

struct vec3 reflectRay(struct vec3 inRay, struct vec3 normal) {
    float dotProduct = 2 * dotProd(normal, inRay);
    struct vec3 reflectedRay = multVec(dotProduct, normal);
    return subVect(reflectedRay, inRay);
}   


// Multiplies a color value by a factor
Uint32 multiplyCol(Uint32 inputCol, float factor) {
    Uint8 red   = (inputCol >> 16) & 0xFF;
    Uint8 green = (inputCol >> 8) & 0xFF;
    Uint8 blue  = inputCol & 0xFF;

    red   = fmin(red * factor, 255);
    green = fmin(green * factor, 255);
    blue  = fmin(blue * factor, 255);

    return (red << 16) | (green << 8) | blue;
}



// Adds two color values
Uint32 addCol(Uint32 col1, Uint32 col2) {
    Uint8 red1   = (col1 >> 16) & 0xFF;
    Uint8 green1 = (col1 >> 8) & 0xFF;
    Uint8 blue1  = col1 & 0xFF;

    Uint8 red2   = (col2 >> 16) & 0xFF;
    Uint8 green2 = (col2 >> 8) & 0xFF;
    Uint8 blue2  = col2 & 0xFF;

    Uint8 red   = fmin(red1 + red2, 255);
    Uint8 green = fmin(green1 + green2, 255);
    Uint8 blue  = fmin(blue1 + blue2, 255);

    return (red << 16) | (green << 8) | blue;
}
