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

struct sphere spheres[] = {
                                {
                                        1, //radius
                                        {0, -1, 3}, // centre coords
                                        0xff0000, // colour
                                        500 // specular exponent (shininess)
                                },
                                {
                                        1, //radius
                                        {-2, 0, 4}, // centre coords
                                        0x14DC00, // colour
                                        500 // specular exponent (shininess) // colour
                                },
                                {
                                        1, //radius
                                        {2, 0, 4}, // centre coords
                                        0x0000ff, // colour
                                        10 // specular exponent (shininess) // colour
                                },
                                {
                                        50000, //radius
                                        {0, -50001, 0}, // centre coords
                                        0xffff00, // colour
                                        100 // specular exponent (shininess) // colour
                                }
                           };

struct light{
        char style;
        float intensity;
        struct vec3 direction;
        struct vec3 position;
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
                                        0.2, // intensity
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

float width = 600 * 1/1; // resolution * aspect
float height = 600;
int scale = 1;

float viewWidth = 1;
float viewHeight = 1;
float viewZ = 1;

struct vec3 camPosGlo = {0,-0.3,0.2};
struct vec3 camStep = {0.005, 0.01, 0.005};

SDL_Window* window;
SDL_Renderer* renderer;

void putPixel(int x, int y, Uint32 col){
        x = width/2 + x;
        y = height/2 - y;

        SDL_SetRenderDrawColor(renderer, ((col & 0xff0000) >> 16 ), ((col & 0x00ff00) >> 8 ), (col & 0x0000ff), 255);
        SDL_RenderDrawPoint(renderer, x, y);
}


void init(){

        window = SDL_CreateWindow("Hal_Raytrace",
                        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                        width * scale, height * scale, SDL_WINDOW_RESIZABLE);

        Uint32 renderFlags = SDL_RENDERER_ACCELERATED;

        renderer = SDL_CreateRenderer(window, -1, renderFlags);

        SDL_RenderSetScale(renderer, scale, scale);
}

struct vec3 viewportCoord(int x, int y){
        struct vec3 viewpos;
        viewpos.x = x * viewWidth/width;
        viewpos.y = y * viewHeight/height;
        viewpos.z = viewZ;

        return viewpos;
}


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

float solveRaySphere(struct vec3 viewPPos, struct sphere sphere, struct vec3 camPos){
        float r = sphere.radius;
        struct vec3 centre = subVect(camPos, sphere.centre);

        float a = dotProd(viewPPos, viewPPos);
        float b = 2* dotProd(centre, viewPPos);
        float c = dotProd(centre, centre) - r*r;

        float discr = b*b - 4*a*c;

        if (discr < 0) {
                return INFINITY, INFINITY;
        }

        float sol1 = (-b + sqrt(discr)) / (2*a);
        float sol2 = (-b - sqrt(discr)) / (2*a);

        return sol1, sol2;
}

float compLight(struct vec3 intersect, struct vec3 normal, struct vec3 viewPPos, struct sphere closeSphere){
        float intTotal = 0.0;
        float dotRes;
        struct vec3 dirResult;

        for (int i = 0; i < (sizeof(lights)/sizeof(lights[0])); i++){

                if (lights[i].style == 'a'){
                        intTotal += lights[i].intensity;
                } else {
                        if (lights[i].style == 'd'){
                                dirResult = lights[i].direction;
                        } else if (lights[i].style == 'p'){
                                dirResult = subVect(lights[i].position, intersect);
                        }

                        dotRes = dotProd(dirResult, normal);
                        
                        if (dotRes > 0){
                                struct vec3 refRay = subVect(multVec(2*dotProd(normal, dirResult),normal),dirResult);
                                struct vec3 viewRay = subVect(intersect, viewPPos);
                                float shine = pow(dotProd(refRay, viewRay)/(lenVec(refRay)*lenVec(viewRay)), closeSphere.specExp);

                                intTotal += lights[i].intensity * dotRes/(lenVec(dirResult)*lenVec(normal)); // Diffuse Light
                                intTotal += lights[i].intensity * shine; // Specular light
                        }
                }
        }
        return intTotal;
}

Uint32 traceRay(struct vec3 viewPPos, struct vec3 camPos){
        float closeHit = INFINITY;
        struct sphere closestSphere;
        Uint32 backgroundCol = 0x4A515B;
        closestSphere.radius = 0;

        for (int i = 0; i < (sizeof(spheres)/sizeof(spheres[0])); i++){
                float sol1, sol2 = solveRaySphere(viewPPos, spheres[i], camPos);
                float dMin = sqrt(dotProd(subVect(viewPPos, camPos),subVect(viewPPos, camPos)));

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

        struct vec3 intersect = addVect(camPos, multVec(closeHit, viewPPos));

        struct vec3 normal = subVect(intersect, closestSphere.centre);
        normal = multVec((1/lenVec(normal)), normal);

        float lightInten = compLight(intersect, normal, viewPPos, closestSphere);


        Uint16 red      = (closestSphere.color & 0xff0000) >> 16;
        Uint16 green    = (closestSphere.color & 0x00ff00) >> 8;
        Uint16 blue     = closestSphere.color & 0x0000ff;

        red     = (Uint16) fmin(red * lightInten, 255);
        green   = (Uint16) fmin(green * lightInten, 255);
        blue    = (Uint16) fmin(blue * lightInten, 255);

        return (red << 16) ^ (green << 8) ^ blue;
}

void placePixels(){
        for (int iy = height/2; iy >= -height/2; iy-=1){
                for (int ix = -width/2; ix <= width/2; ix += 1){
                        
                        struct vec3 viewPPos = viewportCoord(ix,iy);
                        Uint32 col = traceRay(viewPPos, camPosGlo);
 
                        putPixel(ix, iy, col);
                }
        }
}


void main(int argc, char *argv[]){

    init();
    
    typedef enum { false, true } bool;
    bool quit = false;
    SDL_Event event;

    unsigned int lastTicks = SDL_GetTicks();

    while (!quit) {

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        printf("%i\n",(SDL_GetTicks() - lastTicks));
        lastTicks = SDL_GetTicks();
        
        SDL_RenderClear(renderer);
        placePixels();
        SDL_RenderPresent(renderer);
        
        
        camPosGlo = addVect(camPosGlo, multVec(1, camStep));
        // viewZ -= 0.1;
        
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}