#include "vector.h"
#include <math.h>

// Adds two vectors and returns the resulting vector
inline struct vec3 addVect(const struct vec3 a, const struct vec3 b) {
    return (struct vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

// Subtracts vector b from vector a and returns the resulting vector
inline struct vec3 subVect(const struct vec3 a, const struct vec3 b) {
    return (struct vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

// Calculates the dot product of two vectors and returns the result
inline float dotProd(const struct vec3 a, const struct vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Multiplies a vector by a scalar and returns the resulting vector
inline struct vec3 multVec(const float m, const struct vec3 a) {
    return (struct vec3){m * a.x, m * a.y, m * a.z};
}

// Calculates the length of a vector and returns the result
inline float lenVec(const struct vec3 a) {
    return sqrt(dotProd(a, a));
}
