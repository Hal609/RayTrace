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
        float refelct;
};

struct light{
        char style;
        float intensity;
        struct vec3 direction;
        struct vec3 position;
};

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
                                        {-2, 5, 0} // position
                                },
                           };


float height = 200;
int scale = 3;

float ratio = 1.2; //1.778 = 16/9

float viewHeight = 1;
float viewZ = 0.9;

int renderToFile = 0;

SDL_Window* window;
SDL_Renderer* renderer;

SDL_Point points[21000000];
Uint32 colours[21000000];

SDL_Point points2[4000000];
Uint32 colours2[4000000];

int pointsLen = 0;
int pointsLen2 = 0;

struct vec3 camPos = {0,1,0};

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
        /* Creates window and renderer of corrent size */
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

struct closeInt{ // Structure to store result of closest intersection calculation
        struct sphere closetSphere;
        float closeHit;
};

struct closeInt closeIntFunc(struct vec3 home, struct vec3 hit, float lowLim, float highLim){
        float sol1;
        float sol2;
        struct sphere closeSphere2;
        float closeHit = INFINITY;

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

struct vec3 reflectRay(struct vec3 inRay, struct vec3 mirrorRay){
        inRay = multVec(-1, inRay);
        return subVect(multVec(2*dotProd(mirrorRay, inRay),mirrorRay),inRay);
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
        // float closeHit = INFINITY;
        // float maxHit;
        float shine;

        for (int i = 0; i < (sizeof(lights)/sizeof(lights[0])); i++){                
                if (lights[i].style == 'a'){
                        intTotal += lights[i].intensity;
                } else {
                        if (lights[i].style == 'd'){
                                directionResult = subVect(lights[i].direction, intersect);
                        } else if (lights[i].style == 'p'){
                                directionResult = subVect(lights[i].position, intersect);
                        }
                        
                        // Calc shadow
                        struct closeInt cloCalc = closeIntFunc(intersect, directionResult, 0.000001, 1);
                        if (cloCalc.closetSphere.radius != 0){
                                continue;
                        }

                        dotRes = dotProd(directionResult, normal);
                        if (dotRes <= 0){
                                continue;
                        }

                        // Diffuse Light
                        intTotal += lights[i].intensity * dotRes/(lenVec(directionResult)*lenVec(normal)); 

                        // Specular light
                        struct vec3 refRay = reflectRay(directionResult, normal);
                        struct vec3 viewRay = subVect(intersect, vectToView);
                        
                        if (closeSphere.specExp > 0){
                                shine = pow(dotProd(refRay, viewRay)/(lenVec(refRay)*lenVec(viewRay)), closeSphere.specExp);
                                intTotal += lights[i].intensity * shine; 
                        }
                        
                }
        }
        return intTotal;
}

Uint32 multiplyCol(Uint32 inputCol, float factor){
        Uint16 red      = (inputCol & 0xff0000) >> 16;
        Uint16 green    = (inputCol & 0x00ff00) >> 8;
        Uint16 blue     = inputCol & 0x0000ff;

        red     = fmin(red * factor, 255);
        green   = fmin(green * factor, 255);
        blue    = fmin(blue * factor, 255);

        return (red << 16) ^ (green << 8) ^ blue;
}

Uint32 addCol(Uint32 col1, Uint32 col2){
        Uint16 red1      = (col1 & 0xff0000) >> 16;
        Uint16 green1    = (col1 & 0x00ff00) >> 8;
        Uint16 blue1     = col1 & 0x0000ff;

        Uint16 red2      = (col2 & 0xff0000) >> 16;
        Uint16 green2    = (col2 & 0x00ff00) >> 8;
        Uint16 blue2     = col2 & 0x0000ff;

        Uint16 red     = fmin(red1 + red2, 255);
        Uint16 green   = fmin(green1 + green2, 255);
        Uint16 blue    = fmin(blue1 + blue2, 255);

        return (red << 16) ^ (green << 8) ^ blue;
}

Uint32 traceRay(struct vec3 rayOrigin, struct vec3 vectToView, float lowLim, float highLim, int depth){
        float closeHit = INFINITY;
        Uint32 backgroundCol = 0x111111;

        struct vec3 blank = {0,0,0};
        subVect(blank, blank); // Does literally nothing but the program breaks without it... ¯\_(ツ)_/¯
        
        struct closeInt cloCalc = closeIntFunc(rayOrigin, vectToView, lowLim, highLim);
        closeHit = cloCalc.closeHit;

        if (cloCalc.closetSphere.radius == 0) {
                return backgroundCol;
        }

        struct vec3 intersect = addVect(rayOrigin, multVec(closeHit, vectToView));

        struct vec3 normal = subVect(intersect, cloCalc.closetSphere.centre);
        normal = multVec((1/lenVec(normal)), normal);

        float lightInten = compLight(intersect, normal, vectToView, cloCalc.closetSphere);

        Uint32 finCol = multiplyCol(cloCalc.closetSphere.color, lightInten);

        float reflect = cloCalc.closetSphere.refelct;

        if (depth <= 0 || reflect <= 0){
                return finCol;
        }
        
        //Reflection
        struct vec3 bounceRay = reflectRay(vectToView, normal);
        Uint32 reflectCol = traceRay(intersect, bounceRay, 0.001, INFINITY, depth - 1);

        return addCol(multiplyCol(finCol, 1 - reflect), multiplyCol(reflectCol, reflect));

}


void placePixels(struct vec3 camPos, int frame){
        /* Calculates and stores the required colour values for each pixel*/
        int x;
        int y;

        for (int iy = height/2; iy >= -height/2; iy-=1){
                for (int ix = -(height * ratio)/2; ix <= (height *ratio)/2; ix += 1){
                        
                        struct vec3 vectToView = viewportCoord(ix,iy);
                        
                        float dMin = sqrt(dotProd(subVect(vectToView, camPos),subVect(vectToView, camPos)));
                        Uint32 col = traceRay(camPos, vectToView, dMin, INFINITY, 3);
                        
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

void save_texture(const char* file_name, SDL_Renderer* renderer, SDL_Texture* texture) {
    SDL_Texture* target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, texture);
    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch);
    SDL_SaveBMP(surface, file_name);
    SDL_FreeSurface(surface);
    SDL_SetRenderTarget(renderer, target);
}

void *renderTex1(void *vargp) { // Draws pixel colours to renderer
        for (int i = 0; i <= pointsLen; i++){
                SDL_SetRenderDrawColor(renderer, ((colours[i] & 0xff0000) >> 16 ), ((colours[i] & 0x00ff00) >> 8 ), (colours[i] & 0x0000ff), 255);
                SDL_RenderDrawPoint(renderer, points[i].x, points[i].y);
        }
        SDL_RenderPresent(renderer);
        return NULL;
}

void *renderTex2(void *vargp) { // Draws pixel colours to renderer
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

        pthread_t thread_id1;

        int count = 0;

        SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGR888, SDL_TEXTUREACCESS_TARGET, height*ratio, height);
        if (renderToFile){
                SDL_SetRenderTarget(renderer, texture);
        }

        while (!quit) {

                while (SDL_PollEvent(&event)) {
                        if (event.type == SDL_QUIT) {
                                quit = true;
                        }
                }


                if (renderToFile){
                        camPos = addVect(camPos, camStep);
                        pointsLen = 0;
                        placePixels(camPos, 1);
                        for (int i = 0; i <= pointsLen; i++){
                                SDL_SetRenderDrawColor(renderer, ((colours[i] & 0xff0000) >> 16 ), ((colours[i] & 0x00ff00) >> 8 ), (colours[i] & 0x0000ff), 255);
                                SDL_RenderDrawPoint(renderer, points[i].x, points[i].y);
                        }
                } else {
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
                }
                
                // spheres[1].centre.x += 0.002;
                // spheres[1].centre.y += 0.01;
                // spheres[1].centre.z += 0.01;

                // spheres[2].centre.x -= 0.02;
                // spheres[2].centre.y += 0.012; 

                // spheres[0].centre.z += 0.008;

                printf("%i  - %i\n",(SDL_GetTicks() - lastTicks),count);
                lastTicks = SDL_GetTicks();

                if (renderToFile){
                        char str[80];
                        if (count < 10){
                                sprintf(str, "img-00%i.bmp", count);
                        } else if (count < 100){
                                sprintf(str, "img-0%i.bmp", count);
                        } else {
                                sprintf(str, "img-%i.bmp", count);
                        }
                        save_texture(str, renderer, texture);
                }
                
                count++;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}