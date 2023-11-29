#ifndef VECTOR_H
#define VECTOR_H

struct vec3 {
    float x;
    float y;
    float z;
};

struct vec3 addVect(struct vec3 a, struct vec3 b);
struct vec3 subVect(struct vec3 a, struct vec3 b);
float dotProd(struct vec3 a, struct vec3 b);
struct vec3 multVec(float m, struct vec3 a);
float lenVec(struct vec3 a);

#endif
