#include "SDL.h"
#include "SDL_gpu.h"
#include "SDL_mixer.h"

#include <time.h>
#include <sys/param.h>
#include <vector>

#define SCREEN_WIDTH 1334
#define SCREEN_HEIGHT 750

GPU_Target *screen  = NULL;
GPU_Image  *image   = NULL;

float minX = 0;
float maxX = 0;
float minY = 0;
float maxY = 0;

typedef struct {
    float x,xv,y,yv,r,rv;
} Bunny;

std::vector<Bunny> bunnies;

void initBunny(Bunny *bunny);


int
randomInt(int min, int max)
{
    return min + rand() % (max - min + 1);
}

void
moreBunnies(int more) {
    Bunny bunny;

    for (int i = 0; i < more; i++) {
        initBunny(&bunny);
        bunnies.push_back(bunny);
    }
}

void
lessBunnies() {
    if (bunnies.size() <= 1000) {
        bunnies.clear();
    } else {
        bunnies.erase(bunnies.end() - 1000, bunnies.end());
    }
}

void
initBunny(Bunny *bunny)
{
    bunny->x = minX + randomInt( (int)minX,  (int)maxX );
    bunny->y = minY + randomInt( (int)minY,  (int)maxY );

    do {
        bunny->xv = 100 - randomInt(0,200);
    } while (bunny->xv < 10 && bunny->xv > -10);

    do {
        bunny->yv = 100 - randomInt(0,200);
    } while (bunny->yv < 10 && bunny->yv > -10);

    bunny->r = randomInt(0, 360);

    do {
        bunny->rv = 50 - randomInt(0,100);
    } while (bunny->rv < 10 && bunny->rv > -10);
}

void
renderBunnies(float dt)
{
    GPU_Clear(screen);

    for (auto bunny = bunnies.begin(); bunny != bunnies.end(); bunny++) {
        bunny->x += dt * bunny->xv;
        if (bunny->x < minX) { bunny->x = minX; bunny->xv = -bunny->xv; }
        if (bunny->x > maxX) { bunny->x = maxX; bunny->xv = -bunny->xv; }

        bunny->y += dt * bunny->yv;
        if (bunny->y < minY) { bunny->y = minY; bunny->yv = -bunny->yv; }
        if (bunny->y > maxY) { bunny->y = maxY; bunny->yv = -bunny->yv; }

        bunny->r = ((int)round(bunny->r + dt * bunny->rv)) % 360;

        float s = 1; //(0.1f + fabs(sin(bunny->r * (180.0 / 3.14159265359))));

        GPU_BlitTransform(image, NULL, screen,
                          bunny->x, bunny->y, bunny->r,
                          s, s);
    }

    GPU_Flip(screen);
}

float fpsTime  = 0;
int   fpsCount = 0;

int
main(int argc, char *argv[])
{
    // seed random number generator
    srand(time(NULL));

    // init sdl_gpu
    GPU_SetPreInitFlags(GPU_INIT_ENABLE_VSYNC | GPU_INIT_DISABLE_DOUBLE_BUFFER);
    screen = GPU_Init(320, 480, GPU_DEFAULT_INIT_FLAGS);
    if (!screen) {
        printf("Unable to create screen: %s\n", SDL_GetError());
        return 1;
    }
    
    image = GPU_LoadImage("wabbit_alpha.bmp");
    if (!image) {
        printf("Unable to load image: %s\n", SDL_GetError());
        return 1;
    }
    
    GPU_SetSnapMode(image, GPU_SNAP_NONE);

    // initialise bunnies
    minX = 0;
    maxX = SCREEN_WIDTH - image->w;
    minY = 0;
    maxY = SCREEN_HEIGHT - image->h;

    moreBunnies(30000);

    /* Enter render loop, waiting for user to quit */
    const float MILLIS_PER_FRAME = 1000.0f / 60.0f;
    float prevTime = SDL_GetTicks();

    int done = 0;
    while (!done) {
        float currTime  = SDL_GetTicks();
        float deltaTime = (currTime - prevTime) / 1000.0f;
        prevTime = currTime;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    done = 1;
                    break;

                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_q:
                            lessBunnies();
                            break;
                        case SDLK_w:
                            moreBunnies(1000);
                            break;
                        case SDLK_a:
                            moreBunnies(10000);
                            break;
                    }
                    break;

                case SDL_FINGERUP:
                    moreBunnies(100);
                    break;
            }
        }

        renderBunnies(deltaTime);

        fpsTime += deltaTime;
        fpsCount++;
        if (fpsTime > 1.0) {
            printf("%d bunnies %.2f FPS\n", bunnies.size(), (((float)fpsCount) / fpsTime));
            fpsTime  = 0;
            fpsCount = 0;
        }

//        if (deltaTime < MILLIS_PER_FRAME) {
//            SDL_Delay( (Uint32) floor(MILLIS_PER_FRAME - deltaTime) );
//        }
    }

    /* shutdown SDL */
    GPU_FreeImage(image);
    GPU_Quit();
    return 0;
}
