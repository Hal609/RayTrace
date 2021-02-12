#include "SDL2/SDL.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <pthread.h> 

struct vec3 {
        float x;
        float y;
        float z;            
};

struct sphere{
        float radius;
        struct vec3 centre;
        Uint32 color;
        int specExp;
};

struct light{
        char style;
        float intensity;
        struct vec3 direction;
        struct vec3 position;
        float refelct;
};

struct sphere spheres[] = {
                                {
                                        1, //radius
                                        {0, -1, 3}, // centre coords
                                        0xff0000, // colour
                                        500, // specular exponent (shininess)
                                        0.2 // Reflectiveness
                                },
                                {
                                        1, //radius
                                        {-2, 0, 4}, // centre coords
                                        0x14DC00, // colour
                                        500, // specular exponent (shininess)
                                        0.3 // Reflectiveness
                                },
                                {
                                        1, //radius
                                        {2, 0, 4}, // centre coords
                                        0x0000ff, // colour
                                        500, // specular exponent (shininess)
                                        0.4 // Reflectiveness
                                },
                                {
                                        5000, //radius
                                        {0, -5001, 0}, // centre coords
                                        0xffff00, // colour
                                        0, // specular exponent (shininess)
                                        0.5 // Reflectiveness
                                }
                           };


struct light lights[] = {
                                {
                                        'a', // type (a- ambient, d - directional, p - point)
                                        0.2, // intensity
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
                                        0.6, // intensity
                                        {0, 0, 0}, // direction
                                        {2, 1, 0} // position
                                },
                           };


float height = 720;
int scale = 1;

float ratio = 1.2; //1.778 = 16/9

float viewHeight = 1;
float viewZ = 0.9;

SDL_Window* window;
SDL_Renderer* renderer;

SDL_Point points[21000000];
Uint32 colours[21000000];

SDL_Point points2[4000000];
Uint32 colours2[4000000];

int pointsLen = 0;
int pointsLen2 = 0;

struct vec3 camPos = {0,0,0};

//======= VECTOR FUNCTIONS========

struct vec3 addVect(struct vec3 a, struct vec3 b){
        struct vec3 result = {a.x + b.x, a.y + b.y, a.z + b.z};
        return result;
}

struct vec3 subVect(struct vec3 a, struct vec3 b){
        struct vec3 result = {a.x - b.x, a.y - b.y, a.z - b.z};
        return result;
}

float dotProd(struct vec3 a, struct vec3 b){
        float result = a.x*b.x + a.y*b.y + a.z*b.z;
        return result;
}

struct vec3 multVec(float m, struct vec3 a){
        struct vec3 result = {m*a.x, m*a.y, m*a.z};
        return result;
}

float lenVec(struct vec3 a){
        return sqrt(dotProd(a,a));
}


void init(){
        window = SDL_CreateWindow("Hal_Raytrace",
                                  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, // Window position
                                  height*ratio * scale, height * scale,  // Window size (x,y)
                                  SDL_WINDOW_RESIZABLE); // Window flags

        Uint32 renderFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;

        renderer = SDL_CreateRenderer(window, -1, renderFlags); // Create renderer

        SDL_RenderSetScale(renderer, scale, scale);
}

struct vec3 viewportCoord(int x, int y){
        // Translates pixel coordinates to viewport coordinates

        struct vec3 viewpos;
        viewpos.x = x * viewHeight/height;
        viewpos.y = y * viewHeight/height;
        viewpos.z = viewZ;

        return viewpos;
}

struct closeInt{
        struct sphere closetSphere;
        float closeHit;
};

float solveRaySphere(struct vec3 vecStart, struct vec3 vecEnd, struct sphere sphere){
        /* Takes coords on viewport, a sphere and the camera position.
        Calculates if the ray from camera to viewport position intersects with sphere using quadratic formula
         Returns the distances along the ray that it intersects the sphere. */

        float r = sphere.radius;
        struct vec3 centre = subVect(vecStart, sphere.centre);

        float a = dotProd(vecEnd, vecEnd);
        float b = 2* dotProd(centre, vecEnd);
        float c = dotProd(centre, centre) - r*r;

        float discr = b*b - 4*a*c;

        if (discr < 0) {
                return INFINITY, INFINITY;
        }

        float sol1 = (-b + sqrt(discr)) / (2*a);
        float sol2 = (-b - sqrt(discr)) / (2*a);

        return sol1, sol2;
}

struct closeInt traceRayFunc(struct vec3 home, struct vec3 hit, float lowLim, float highLim, float closeHit){
        float sol1 = 0;
        float sol2 = 0;
        // float closeHit = INFINITY;
        struct sphere closeSphere2;
        closeSphere2.radius = 0;

        for (int i = 0; i < (sizeof(spheres)/sizeof(spheres[0])); i++){
                float sol1, sol2 = solveRaySphere(home, hit, spheres[i]);

                if (sol1 > lowLim && sol1 < highLim && sol1 < closeHit) {
                        closeHit = sol1;
                        closeSphere2 = spheres[i];
                }
                if (sol2 > lowLim && sol2 < highLim && sol2 < closeHit) {
                        closeHit = sol2;
                        closeSphere2 = spheres[i];
                }
        }
        struct closeInt result = {closeSphere2, closeHit};
        return result;
}

float compLight(struct vec3 intersect, struct vec3 normal, struct vec3 vectToView, struct sphere closeSphere){
        /* Computes the total light intensity at the point the ray met the sphere (intersect)
        Takes the normal to the sphere, the position on the viewport (vector from origin to viewport)
        and the struct of the sphere which was hit (to get its specular exponant).*/ 

        float intTotal = 0.0; // Total light intensity
        float dotRes;  // Result of the dot product
        struct vec3 directionResult;  // Calculated (or given) direction of the ray
        float sol1 = 0;
        float sol2 = 0;
        float closeHit = INFINITY;
        float shine;

        for (int i = 0; i < (sizeof(lights)/sizeof(lights[0])); i++){                
                if (lights[i].style == 'a'){
                        intTotal += lights[i].intensity;
                } else {
                        if (lights[i].style == 'd'){
                                directionResult = subVect(lights[i].direction, intersect);
                                closeHit = 0.1;
                        } else if (lights[i].style == 'p'){
                                directionResult = subVect(lights[i].position, intersect);
                                // printf("x: %f  y: %f  z: %f\n", directionResult.x, directionResult.y, directionResult.z);
                                closeHit = 1;
                        }
                        
                        // Calc shadow

                        // struct closeInt cloCalc = traceRayFunc(intersect, directionResult, 0.0001, INFINITY);
                        // closeSphere2 = cloCalc.closetSphere;
                        // closeHit = cloCalc.closeHit;

                        struct vec3 home;
                        struct vec3 hit;
                        home = intersect;
                        hit = directionResult;
                        float lowLim = 0.0001;
                        float highLim = INFINITY;

                        sol1 = 0;
                        sol2 = 0;
                        struct sphere closeSphere2;
                        closeSphere2.radius = 0;

                        for (int i = 0; i < (sizeof(spheres)/sizeof(spheres[0])); i++){
                                float sol1, sol2 = solveRaySphere(home, hit, spheres[i]);

                                if (sol1 > lowLim && sol1 < highLim && sol1 < closeHit) {
                                        closeHit = sol1;
                                        closeSphere2 = spheres[i];
                                }
                                if (sol2 > lowLim && sol2 < highLim && sol2 < closeHit) {
                                        closeHit = sol2;
                                        closeSphere2 = spheres[i];
                                }
                        }
                        // struct closeInt cloCalc = traceRayFunc(home, hit, lowLim, highLim, closeHit);;
                        
                        // closeSphere2 = cloCalc.closetSphere;
                        // closeHit = cloCalc.closeHit;

                        // printf("%f\n", closeSphere2.radius);
                        if (closeSphere2.radius != 0){
                                continue;
                        }

                        dotRes = dotProd(directionResult, normal);
                        
                        if (dotRes > 0){
                                struct vec3 refRay = subVect(multVec(2*dotProd(normal, directionResult),normal),directionResult);
                                struct vec3 viewRay = subVect(intersect, vectToView);
                                
                                
                                shine = pow(dotProd(refRay, viewRay)/(lenVec(refRay)*lenVec(viewRay)), closeSphere.specExp);

                                intTotal += lights[i].intensity * dotRes/(lenVec(directionResult)*lenVec(normal)); // Diffuse Light

                                if (closeSphere.specExp > 0){
                                        intTotal += lights[i].intensity * shine; // Specular light
                                }
                        }
                }
        }
        return intTotal;
}

Uint32 traceRay(struct vec3 vectToView){
        float closeHit = INFINITY;
        struct sphere closestSphere;
        Uint32 backgroundCol = 0x4A515B;
        closestSphere.radius = 0;

        for (int i = 0; i < (sizeof(spheres)/sizeof(spheres[0])); i++){
                float sol1, sol2 = solveRaySphere(camPos ,vectToView, spheres[i]);
                float dMin = sqrt(dotProd(subVect(vectToView, camPos),subVect(vectToView, camPos)));

                if (sol1 > dMin && sol1 < INFINITY && sol1 < closeHit) {
                        closeHit = sol1;
                        closestSphere = spheres[i];
                }
                if (sol2 > dMin && sol2 < INFINITY && sol2 < closeHit) {
                        closeHit = sol2;
                        closestSphere = spheres[i];
                }
        }
        if (closestSphere.radius == 0) {
                return backgroundCol;
        }

        struct vec3 intersect = addVect(camPos, multVec(closeHit, vectToView));

        struct vec3 normal = subVect(intersect, closestSphere.centre);
        normal = multVec((1/lenVec(normal)), normal);

        float lightInten = compLight(intersect, normal, multVec(-1, vectToView), closestSphere);


        Uint16 red      = (closestSphere.color & 0xff0000) >> 16;
        Uint16 green    = (closestSphere.color & 0x00ff00) >> 8;
        Uint16 blue     = closestSphere.color & 0x0000ff;

        red     = (Uint16) fmin(red * lightInten, 255);
        green   = (Uint16) fmin(green * lightInten, 255);
        blue    = (Uint16) fmin(blue * lightInten, 255);

        return (red << 16) ^ (green << 8) ^ blue;
}


void placePixels(struct vec3 camPos, int frame){
        int x;
        int y;

        for (int iy = height/2; iy >= -height/2; iy-=1){
                for (int ix = -(height * ratio)/2; ix <= (height *ratio)/2; ix += 1){
                        
                        struct vec3 vectToView = viewportCoord(ix,iy);
                        Uint32 col = traceRay(vectToView);
                        
                        x = (height * ratio)/2 + ix;
                        y = height/2 - iy;
                        
                        SDL_Point newPoint = {x, y};
                        if (frame == 1){
                                points[pointsLen] = newPoint;
                                colours[pointsLen] = col;
                                pointsLen += 1;
                        } else {
                                points2[pointsLen2] = newPoint;
                                colours2[pointsLen2] = col;
                                pointsLen2 += 1;
                        }
                }
        }
}


void *renderTex1(void *vargp) { 
        for (int i = 0; i <= pointsLen; i++){
                SDL_SetRenderDrawColor(renderer, ((colours[i] & 0xff0000) >> 16 ), ((colours[i] & 0x00ff00) >> 8 ), (colours[i] & 0x0000ff), 255);
                SDL_RenderDrawPoint(renderer, points[i].x, points[i].y);
        }
         SDL_RenderPresent(renderer);
        return NULL;
}

void *renderTex2(void *vargp) { 
        for (int i = 0; i <= pointsLen2; i++){
                SDL_SetRenderDrawColor(renderer, ((colours2[i] & 0xff0000) >> 16 ), ((colours2[i] & 0x00ff00) >> 8 ), (colours2[i] & 0x0000ff), 255);
                SDL_RenderDrawPoint(renderer, points2[i].x, points2[i].y);
        }
         SDL_RenderPresent(renderer);
        return NULL;
}


int main(int argc, char *argv[]){

    init();
    
    typedef enum {false, true} bool;
    bool quit = false;
    SDL_Event event;

    unsigned int lastTicks = SDL_GetTicks();

    struct vec3 camStep = {0.005, 0.005, 0.01};
//     struct vec3 camStep = {0.00, 0.00, 0.0};     

    pthread_t thread_id1;

    int count = 0;

    while (!quit) {
        count++;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        if (count % 2 == 0){
                pthread_create(&thread_id1, NULL, renderTex1, NULL);
                camPos = addVect(camPos, camStep);
                pointsLen2 = 0;
                placePixels(camPos, 2);
                pthread_join(thread_id1, NULL);
        }
        else {
                pthread_create(&thread_id1, NULL, renderTex2, NULL);
                camPos = addVect(camPos, camStep);
                pointsLen = 0;
                placePixels(camPos, 1);
                pthread_join(thread_id1, NULL);
        }

        printf("%i\n",(SDL_GetTicks() - lastTicks));
        lastTicks = SDL_GetTicks();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}