#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#define PI 3.1415926f
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

void Play(SDL_Window *window, SDL_Renderer *renderer);

// gcc main.c -o main -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lm
#undef main
void main()
{
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    Play(window, renderer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return;
}

void Play(SDL_Window *window, SDL_Renderer *renderer)
{
    SDL_Event event;
    bool quit = false;

    SDL_Surface *surface = IMG_Load("1.jpg");
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
        }

        // clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // renderer
        // const SDL_FPoint center = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
        // SDL_RenderCopyExF(renderer, texture, NULL, NULL, ((SDL_GetTicks()/1000) % 10) * 10, &center, SDL_FLIP_NONE);

        // SDL_Vertex vertices[3] = {0};
        // vertices[0].color.r = 255;
        // vertices[0].color.g = 255;
        // vertices[0].color.b = 255;
        // vertices[0].color.a = 255;
        // vertices[0].position.x = 0;
        // vertices[0].position.y = 0;
        // vertices[0].tex_coord.x = 0.0f;
        // vertices[0].tex_coord.y = 0.0f;

        // vertices[1].color.r = 255;
        // vertices[1].color.g = 255;
        // vertices[1].color.b = 255;
        // vertices[1].color.a = 255;
        // vertices[1].position.x = 0;
        // vertices[1].position.y = WINDOW_HEIGHT;
        // vertices[1].tex_coord.x = 0;
        // vertices[1].tex_coord.y = 1.0f;

        // vertices[2].color.r = 255;
        // vertices[2].color.g = 255;
        // vertices[2].color.b = 255;
        // vertices[2].color.a = 255;
        // vertices[2].position.x = WINDOW_WIDTH;
        // vertices[2].position.y = WINDOW_HEIGHT;
        // vertices[2].tex_coord.x = 1.0f;
        // vertices[2].tex_coord.y = 1.0f;

        // int indices[] = {0, 1, 2};

        // SDL_RenderGeometry(renderer, texture, vertices, 3, indices, 3);
        float radius = 0.3f;

        SDL_Rect size = {20, 20, WINDOW_WIDTH - 20, WINDOW_HEIGHT - 20};
        SDL_Rect *dstrect = &size;

        int sWidth, sHeight; // 纹理宽高
        float radiusW;       // 宽度半径
        float radiusH;       // 高度半径

        // 获取纹理宽高
        SDL_QueryTexture(texture, NULL, NULL, &sWidth, &sHeight);
        radiusW = 50;
        radiusH = 50;

        // // 渲染四个顶角
        int *index = NULL;
        SDL_Vertex *vertex = NULL;
        int vertexNum = 16; // 边缘顶点数

        vertex = (SDL_Vertex *)malloc(sizeof(SDL_Vertex) * (vertexNum + 4));
        index = (int *)malloc(sizeof(int) * (vertexNum - 4) * 3);

        // 右上角
        vertex[vertexNum + 0].position.x = dstrect->x + dstrect->w - radiusW;
        vertex[vertexNum + 0].position.y = dstrect->y + radiusH;
        vertex[vertexNum + 0].tex_coord.x = 1.0f - radius;
        vertex[vertexNum + 0].tex_coord.y = radius;

        // 左上角
        vertex[vertexNum + 1].position.x = dstrect->x + radiusW;
        vertex[vertexNum + 1].position.y = dstrect->y + radiusH;
        vertex[vertexNum + 1].tex_coord.x = radius;
        vertex[vertexNum + 1].tex_coord.y = radius;

        // 左下角
        vertex[vertexNum + 2].position.x = dstrect->x + radiusW;
        vertex[vertexNum + 2].position.y = dstrect->y + dstrect->h - radiusH;
        vertex[vertexNum + 2].tex_coord.x = radius;
        vertex[vertexNum + 2].tex_coord.y = 1.0f - radius;

        // 右下角
        vertex[vertexNum + 3].position.x = dstrect->x + dstrect->w - radiusW;
        vertex[vertexNum + 3].position.y = dstrect->y + dstrect->h - radiusH;
        vertex[vertexNum + 3].tex_coord.x = 1.0f - radius;
        vertex[vertexNum + 3].tex_coord.y = 1.0f - radius;

        int *p;
        p = index;
        for (int i = 0; i < 4; i++)
        {
            for (int j = i * vertexNum / 4; j < (i + 1) * vertexNum / 4; j++)
            {
                vertex[j].color = (SDL_Color){0xFF, 0, 0xFF, 0xFF};
                vertex[j].position.x = vertex[vertexNum + i].position.x + radiusW * cosf(2.0f * PI * (float)j / (float)vertexNum);
                vertex[j].position.y = vertex[vertexNum + i].position.y - radiusH * sinf(2.0f * PI * (float)j / (float)vertexNum);
                vertex[j].tex_coord.x = vertex[vertexNum + i].tex_coord.x + radius * cosf(2.0f * PI * (float)j / (float)vertexNum);
                vertex[j].tex_coord.y = vertex[vertexNum + i].tex_coord.y - radius * sinf(2.0f * PI * (float)j / (float)vertexNum);

                if (j < (i + 1) * vertexNum / 4 - 1)
                {
                    *(p++) = vertexNum + i;
                    *(p++) = j;
                    *(p++) = j + 1;
                }

                // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                // SDL_RenderDrawPoint(renderer, vertex[j].position.x, vertex[j].position.y);
            }
        }
        SDL_RenderGeometry(renderer, NULL, vertex, vertexNum + 4, index, (vertexNum - 4) * 3);
        free(vertex);
        free(index);

        SDL_RenderPresent(renderer);
    }
    SDL_FreeSurface(surface);

    SDL_DestroyTexture(texture);
}
