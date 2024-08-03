#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "main.h"
#include "gui_button.h"

#if 0

SDL_Surface *applyGaussianBlur(SDL_Surface *surface, int radius)
{
    int width = surface->w;
    int height = surface->h;
    SDL_Surface *tempSurface = SDL_CreateRGBSurface(0, width, height, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    SDL_Surface *blurredSurface = SDL_CreateRGBSurface(0, width, height, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

    if (tempSurface == NULL || blurredSurface == NULL)
    {
        printf("SDL_CreateRGBSurface failed: %s\n", SDL_GetError());
        if (tempSurface)
            SDL_FreeSurface(tempSurface);
        if (blurredSurface)
            SDL_FreeSurface(blurredSurface);
        return NULL;
    }

    // 高斯核
    int kernelSize = 2 * radius + 1;
    float sigma = radius / 2.0f;
    float *kernel = (float *)malloc(kernelSize * sizeof(float));
    float sum = 0.0;

    for (int i = 0; i < kernelSize; i++)
    {
        int x = i - radius;
        kernel[i] = expf(-(x * x) / (2 * sigma * sigma));
        sum += kernel[i];
    }

    for (int i = 0; i < kernelSize; i++)
    {
        kernel[i] /= sum;
    }

    // 水平模糊
    Uint32 *pixels = (Uint32 *)surface->pixels;
    Uint32 *tempPixels = (Uint32 *)tempSurface->pixels;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float r = 0, g = 0, b = 0, a = 0;

            for (int k = -radius; k <= radius; k++)
            {
                int ix = x + k;

                if (ix >= 0 && ix < width)
                {
                    int index = y * width + ix;
                    Uint32 pixel = pixels[index];

                    float kernelValue = kernel[k + radius];

                    r += ((pixel & 0x00FF0000) >> 16) * kernelValue;
                    g += ((pixel & 0x0000FF00) >> 8) * kernelValue;
                    b += (pixel & 0x000000FF) * kernelValue;
                    a += ((pixel & 0xFF000000) >> 24) * kernelValue;
                }
            }

            int index = y * width + x;
            tempPixels[index] = ((Uint32)a << 24) | ((Uint32)r << 16) | ((Uint32)g << 8) | (Uint32)b;
        }
    }

    // 垂直模糊
    Uint32 *blurredPixels = (Uint32 *)blurredSurface->pixels;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float r = 0, g = 0, b = 0, a = 0;

            for (int k = -radius; k <= radius; k++)
            {
                int iy = y + k;

                if (iy >= 0 && iy < height)
                {
                    int index = iy * width + x;
                    Uint32 pixel = tempPixels[index];

                    float kernelValue = kernel[k + radius];

                    r += ((pixel & 0x00FF0000) >> 16) * kernelValue;
                    g += ((pixel & 0x0000FF00) >> 8) * kernelValue;
                    b += (pixel & 0x000000FF) * kernelValue;
                    a += ((pixel & 0xFF000000) >> 24) * kernelValue;
                }
            }

            int index = y * width + x;
            blurredPixels[index] = ((Uint32)a << 24) | ((Uint32)r << 16) | ((Uint32)g << 8) | (Uint32)b;
        }
    }

    free(kernel);
    SDL_FreeSurface(tempSurface);
    return blurredSurface;
}

int main()
{

    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();

    // 设置纹理缩放模式为线性插值（模糊）
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    SDL_Window *window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font *font = TTF_OpenFont("ttf.ttc", 32);

    SDL_Surface *surface = IMG_Load("test/1.jpg");
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    SDL_Surface *surface2 = SDL_CreateRGBSurfaceWithFormat(0, 1000, 600, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, surface2->pixels, surface2->pitch);

    SDL_Surface *blurredSurface = NULL;
    blurredSurface = applyGaussianBlur(surface2, 50);

    SDL_Texture *texture2 = SDL_CreateTextureFromSurface(renderer, blurredSurface);

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

        SDL_RenderCopy(renderer, texture2, NULL, NULL);

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 30);
    }

    IMG_Quit();
    SDL_Quit();
    return 0;
}
#endif