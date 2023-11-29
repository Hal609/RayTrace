#include "config.h"
#include "render.h"
#include "vector.h"
#include <math.h>
#include <float.h> // For FLT_MAX
#include "sdl_setup.h"
#include "structures.h"
#include "render_utils.h"
#include <stdbool.h>

struct sphere spheres[] = {
                                {
                                        1, //radius
                                        {2, 1, 4}, // centre coords
                                        0xff0000, // colour
                                        500, // specular exponent (shininess)
                                        0.2 // Reflectiveness
                                },
                                {
                                        0.4, //radius
                                        {0, 0.4, 3}, // centre coords
                                        0xffffff, // colour
                                        500, // specular exponent (shininess)
                                        0.2 // Reflectiveness
                                },
                                {
                                        0.3, //radius
                                        {-0.7, 0.3, 5.5}, // centre coords
                                        0x14DC00, // colour
                                        500, // specular exponent (shininess)
                                        0.2 // Reflectiveness
                                },
                                {
                                        0.9, //radius
                                        {-2, 0.9, 7}, // centre coords
                                        0xba05fc, // colour
                                        500, // specular exponent (shininess)
                                        0.2 // Reflectiveness
                                },
                                {
                                        0.7, //radius
                                        {-1.5, 0.7, 3}, // centre coords
                                        0x0000ff, // colour
                                        500, // specular exponent (shininess)
                                        0.2 // Reflectiveness
                                },
                                {
                                        0.35, //radius
                                        {-1.3, 0.35, 4.5}, // centre coords
                                        0xff0000, // colour
                                        500, // specular exponent (shininess)
                                        0.2 // Reflectiveness
                                },
                                {
                                        0.3, //radius
                                        {1, 0.3, 3.3}, // centre coords
                                        0xba05fc, // colour
                                        500, // specular exponent (shininess)
                                        0.2 // Reflectiveness
                                },
                                {
                                        0.32, //radius
                                        {2.7, 0.32, 3}, // centre coords
                                        0x00ff00, // colour
                                        500, // specular exponent (shininess)
                                        0.2 // Reflectiveness
                                },
                                {
                                        0.8, //radius
                                        {1.2, 0.8, 7}, // centre coords
                                        0x0000ff, // colour
                                        1000, // specular exponent (shininess)
                                        0.2 // Reflectiveness
                                },
                                {
                                        5000, //radius
                                        {0, -5000, 0}, // centre coords
                                        0xeeeedd, // colour
                                        500, // specular exponent (shininess)
                                        0.3 // Reflectiveness
                                }
                           };


struct light lights[] = {
                                {
                                        'a', // type (a- ambient, d - directional, p - point)
                                        0.05, // intensity
                                        {0, 0, 0}, // direction
                                        {0, 0, 0} // position
                                },
                                {
                                        'd', // type (a- ambient, d - directional, p - point)
                                        0.1, // intensity
                                        {1, 4, 4}, // direction
                                        {0, 0, 0} // position
                                },
                                {
                                        'p', // type (a- ambient, d - directional, p - point)
                                        0.7, // intensity
                                        {0, 0, 0}, // direction
                                        {-2, 5, 0} // position
                                },
                           };

// Translates pixel coordinates to viewport coordinates
struct vec3 viewportCoord(int x, int y) {
    return (struct vec3) {
        .x = x * viewHeight / height,
        .y = y * viewHeight / height,
        .z = viewZ
    };
}

#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct ThreadData {
    int startX, endX, startY, endY;
    struct vec3 camPos;
    int frame;
};

void* placePixelsThreaded(void* arg) {
    struct ThreadData* data = (struct ThreadData*)arg;

    for (int iy = data->startY; iy >= data->endY; iy--) {
        for (int ix = data->startX; ix <= data->endX; ix++) {
            struct vec3 vectToView = viewportCoord(ix, iy);

            Uint32 col = traceRay(data->camPos, vectToView, 0.001, INFINITY, 3);

            int x = (height * ratio) / 2 + ix;
            int y = height / 2 - iy;

            SDL_Point newPoint = {x, y};

            pthread_mutex_lock(&mutex);
            points[pointsLen] = newPoint;
            colours[pointsLen] = col;
            pointsLen += 1;
            pthread_mutex_unlock(&mutex);
        }
    }

    return NULL;
}

void placePixels(struct vec3 camPos, int frame) {
    int quarterHeight = height / 4;
    int quarterWidth = (height * ratio) / 4;

    struct ThreadData data[16];
    pthread_t threads[16];

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            data[i * 4 + j] = (struct ThreadData){
                .startX = -quarterWidth * (2 - j),
                .endX = -quarterWidth * (1 - j),
                .startY = quarterHeight * (2 - i),
                .endY = quarterHeight * (1 - i),
                .camPos = camPos,
                .frame = frame
            };
            pthread_create(&threads[i * 4 + j], NULL, placePixelsThreaded, &data[i * 4 + j]);
        }
    }

    for (int i = 0; i < 16; i++) {
        pthread_join(threads[i], NULL);
    }
}

// Function to solve for ray-sphere intersection using the quadratic formula
struct solutions solveRaySphere(const struct vec3 vecStart, const struct vec3 vecEnd, const struct sphere sphere) {
    struct solutions sol = {INFINITY, INFINITY};

    const float r = sphere.radius;
    const struct vec3 centre = subVect(vecStart, sphere.centre);

    const float a = dotProd(vecEnd, vecEnd);
    const float inv_2a = 1.0f / (2 * a);  // Precompute the division
    const float b = 2 * dotProd(centre, vecEnd);
    const float c = dotProd(centre, centre) - r * r;

    const float discr = b * b - 4 * a * c;

    if (discr < 0) {
        return sol;
    }

    const float sqrt_discr = sqrtf(discr);  // Compute square root once
    sol.sol1 = (-b + sqrt_discr) * inv_2a;
    sol.sol2 = (-b - sqrt_discr) * inv_2a;
    return sol;
}

struct closeInt closeIntFunc(struct vec3 home, struct vec3 hit, float lowLim, float highLim) {
    struct closeInt result;
    result.closeHit = INFINITY;
    result.closestSphere.radius = 0;
    int numSpheres = sizeof(spheres) / sizeof(spheres[0]);

    for (int i = 0; i < numSpheres; i++) {
        struct solutions sols = solveRaySphere(home, hit, spheres[i]);

        if (sols.sol1 > lowLim && sols.sol1 < highLim && sols.sol1 < result.closeHit) {
            result.closeHit = sols.sol1;
            result.closestSphere = spheres[i];
        }
        if (sols.sol2 > lowLim && sols.sol2 < highLim && sols.sol2 < result.closeHit) {
            result.closeHit = sols.sol2;
            result.closestSphere = spheres[i];
        }
    }

    return result;
}

// Computes the total light intensity at a point
float compLight(struct vec3 intersect, struct vec3 normal, struct vec3 vectToView, struct sphere closeSphere) {
    float intTotal = 0.0;
    int numLights = sizeof(lights) / sizeof(lights[0]);

    for (int i = 0; i < numLights; i++) {
        if (lights[i].style == 'a') {
            intTotal += lights[i].intensity;
            continue;
        }

        struct vec3 directionResult = (lights[i].style == 'd') ? 
                                      subVect(lights[i].direction, intersect) : 
                                      subVect(lights[i].position, intersect);

        struct closeInt cloCalc = closeIntFunc(intersect, directionResult, 0.000001, 1);
        if (cloCalc.closestSphere.radius != 0) continue;

        float dotRes = dotProd(directionResult, normal);
        if (dotRes <= 0) continue;

        float lenDirection = lenVec(directionResult);
        float lenNormal = lenVec(normal);
        intTotal += lights[i].intensity * dotRes / (lenDirection * lenNormal);  // Diffuse Light

        if (closeSphere.specExp > 0) {
            struct vec3 refRay = reflectRay(directionResult, normal);
            struct vec3 viewRay = subVect(intersect, vectToView);
            float shine = pow(dotProd(refRay, viewRay) / (lenVec(refRay) * lenVec(viewRay)), closeSphere.specExp);
            intTotal += lights[i].intensity * shine;  // Specular light
        }
    }

    return intTotal;
}


// Traces a ray from the camera to the viewport
Uint32 traceRay(struct vec3 rayOrigin, struct vec3 vectToView, float lowLim, float highLim, int depth) {
    const Uint32 backgroundCol = 0x111111;

    struct closeInt cloCalc = closeIntFunc(rayOrigin, vectToView, lowLim, highLim);

    if (cloCalc.closestSphere.radius == 0) {
        return backgroundCol;  // Early return if no sphere is hit
    }

    struct vec3 intersect = addVect(rayOrigin, multVec(cloCalc.closeHit, vectToView));
    struct vec3 normal = subVect(intersect, cloCalc.closestSphere.centre);
    float inverseNormLength = 1.0f / lenVec(normal);
    normal = multVec(inverseNormLength, normal);

    float lightInten = compLight(intersect, normal, vectToView, cloCalc.closestSphere);
    Uint32 finCol = multiplyCol(cloCalc.closestSphere.color, lightInten);

    float reflect = cloCalc.closestSphere.reflect;
    if (depth <= 0 || reflect <= 0) {
        return finCol;  // Return final color if reflection isn't needed
    }

    // Reflection
    struct vec3 bounceRay = reflectRay(vectToView, normal);
    Uint32 reflectCol = traceRay(intersect, bounceRay, 0.001, INFINITY, depth - 1);

    return addCol(multiplyCol(finCol, 1 - reflect), multiplyCol(reflectCol, reflect));
}


