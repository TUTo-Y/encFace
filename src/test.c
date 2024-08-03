#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "main.h"
#include "gui_button.h"

#if 0
int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font *font = TTF_OpenFont("ttf.ttc", 32);


    SDL_Texture *texture = drawCircle(100, (SDL_Color){255, 0, 0, 128}, renderer, true);
    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    printf("w: %d, h: %d\n", w, h);

    int quit = 1;
    while (quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, texture, NULL, &(SDL_Rect){100, 100, 300, 300});



        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 30);
    }

    IMG_Quit();
    SDL_Quit();
    return 0;
}
#endif