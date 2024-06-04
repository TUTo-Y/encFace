#include <math.h>
#include <time.h>
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

#define INT2RGBA(x) ((SDL_Color){(x) & 0xff, ((x) & 0xff00) >> 8, ((x) & 0xff0000) >> 16, ((x) & 0xff000000) >> 24})

/**
 * \brief 扰动函数
 * \param window_width 窗口宽度
 * \param window_height 窗口高度
 * \param x 鼠标 x 坐标
 * \param y 鼠标 y 坐标
 * \param stonesize 石头大小
 * \param stoneweight 石头力度
 * \param buf 波幅数组
 */
static void disturb(int window_width, int window_height, int x, int y, int stonesize, int stoneweight, short *buf)
{
    if ((x >= window_width - stonesize) ||
        (x < stonesize) ||
        (y >= window_height - stonesize) ||
        (y < stonesize))
        return;

    int stonesize_squared = stonesize * stonesize;
    for (int posx = x - stonesize; posx < x + stonesize; posx++)
    {
        int dx_squared = (posx - x) * (posx - x);
        for (int posy = y - stonesize; posy < y + stonesize; posy++)
        {
            int dy_squared = (posy - y) * (posy - y);
            if (dx_squared + dy_squared < stonesize_squared)
            {
                buf[window_width * posy + posx] += stoneweight;
            }
        }
    }
}

/**
 * \brief 计算下一帧波幅
 * \param window_width 窗口宽度
 * \param window_height 窗口高度
 * \param buf 当前波幅
 * \param buf2 下一帧波幅
 */
static void nextFrame(int window_width, int window_height, short **buf, short **buf2)
{
    for (int i = window_width; i < window_height * (window_width - 1); i++)
    {
        // 公式：X0'= (X1+X2+X3+X4) / 2 - X0
        (*buf2)[i] = (((*buf)[i - window_width] + (*buf)[i + window_width] + (*buf)[i - 1] + (*buf)[i + 1]) >> 1) - (*buf2)[i];

        // 波能衰减
        (*buf2)[i] -= (*buf2)[i] >> 5;
    }

    // short *temp = *buf;
    // *buf = *buf2;
    // *buf2 = temp;

    for (int i = window_width; i < window_height * (window_width - 1); i++)
    {
        // 公式：X0'= (X1+X2+X3+X4) / 2 - X0
        (*buf)[i] = (((*buf2)[i - window_width] + (*buf2)[i + window_width] + (*buf2)[i - 1] + (*buf2)[i + 1]) >> 1) - (*buf)[i];

        // 波能衰减
        (*buf)[i] -= (*buf)[i] >> 5;
    }
}

/**
 * \brief 渲染波纹
 * \param window_width 窗口宽度
 * \param window_height 窗口高度
 * \param buf 波幅数组
 * \param color_src 原图片内存指针
 * \param color_tar 处理后显示的位图内存指针
 */
static void RenderRipple(int window_width, int window_height, short *buf, SDL_Color *color_src, SDL_Color *color_tar)
{
    int i = 0;
    for (int y = 0; y < window_height; y++)
    {
        int offset_y = y - window_height / 2;
        for (int x = 0; x < window_width; x++)
        {
            short data = 1024 - buf[i];
            int offset_x = x - window_width / 2;

            int a = (offset_x * data / 1024) + window_width / 2;
            int b = (offset_y * data / 1024) + window_height / 2;

            if (a >= window_width)
                a = window_width - 1;
            if (a < 0)
                a = 0;
            if (b >= window_height)
                b = window_height - 1;
            if (b < 0)
                b = 0;

            color_tar[i] = color_src[a + (b * window_width)];
            i++;
        }
    }
}

void Play();
void Play()
{
    int WINDOW_LOGIN_DEFAULT_WIDTH = 800;
    int WINDOW_LOGIN_DEFAULT_HEIGHT = 600;

    SDL_Window *window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Surface *tmp = IMG_Load("login.png");
    SDL_Surface *surface = SDL_ConvertSurfaceFormat(tmp, SDL_PIXELFORMAT_RGBA32, 0);

    WINDOW_LOGIN_DEFAULT_WIDTH = surface->w;
    WINDOW_LOGIN_DEFAULT_HEIGHT = surface->h;

    SDL_Surface *surface_src = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_SetSurfaceBlendMode(surface_src, SDL_BLENDMODE_BLEND);
    SDL_BlitSurface(surface, NULL, surface_src, NULL);

    SDL_Surface *surface_tar = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_SetSurfaceBlendMode(surface_tar, SDL_BLENDMODE_BLEND);
    SDL_BlitSurface(surface, NULL, surface_tar, NULL);

    SDL_SetWindowSize(window, WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT);

    SDL_Color *color_src = surface_src->pixels; // 原图片片内存指针
    SDL_Color *color_tar = surface_tar->pixels; // 处理后显示的位图内存指针

    // 以下两个 buf 为每一个点的波幅，前者为当前波幅，后者为下一个时刻的波幅。
    short *buf = malloc((WINDOW_LOGIN_DEFAULT_HEIGHT * WINDOW_LOGIN_DEFAULT_WIDTH + WINDOW_LOGIN_DEFAULT_WIDTH) * sizeof(short));
    short *buf2 = malloc((WINDOW_LOGIN_DEFAULT_HEIGHT * WINDOW_LOGIN_DEFAULT_WIDTH + WINDOW_LOGIN_DEFAULT_WIDTH) * sizeof(short));

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT);

    // 渲染
    int a = 1;

    // test
    SDL_Color color1[] = {
        INT2RGBA(0xffee9ca7),
        INT2RGBA(0xff2193b0),
        INT2RGBA(0xffa8ff78),
        INT2RGBA(0xff11998e),
        INT2RGBA(0xff74ebd5),
        INT2RGBA(0xff43c6ac),
        INT2RGBA(0xffc0c0aa)};
    SDL_Color color2[] = {
        INT2RGBA(0xffffdde1),
        INT2RGBA(0xff6dd5ed),
        INT2RGBA(0xff78ffd6),
        INT2RGBA(0xff38ef7d),
        INT2RGBA(0xffacb6e5),
        INT2RGBA(0xfff8ffae),
        INT2RGBA(0xff1cefff)};
    SDL_Color *colorp1A = color1;
    SDL_Color *colorp1B = color1 + 1;

    SDL_Color *colorp2A = color2;
    SDL_Color *colorp2B = color2 + 1;

    SDL_Vertex vertex[4] = {
        {.color = {0x74, 0xEB, 0xD5, 255},
         .position = {0, 0},
         .tex_coord = {0, 0}},
        {.color = {0xAC, 0xB6, 0xE5, 255},
         .position = {WINDOW_LOGIN_DEFAULT_WIDTH, 0},
         .tex_coord = {0, 0}},
        {.color = {0xAC, 0xB6, 0xE5, 255},
         .position = {WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT},
         .tex_coord = {0, 0}},
        {.color = {0x74, 0xEB, 0xD5, 255},
         .position = {0, WINDOW_LOGIN_DEFAULT_HEIGHT},
         .tex_coord = {0, 0}},
    };
    int index[6] = {0, 1, 2, 0, 2, 3};

    bool quit = false;
    int time1 = 0, time2 = 0;
    time2 = time1 = SDL_GetTicks();
    while (!quit)
    {
        time1 = SDL_GetTicks();
        SDL_Event event = {0};
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                quit = true;
                break;

                // case SDL_MOUSEMOTION: // 移动事件
                //     if (event.motion.state == SDL_BUTTON_LMASK)
                //     {
                //         disturb(WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, event.motion.x, event.motion.y, 5, 256, buf);
                //     }
                //     break;

                // case SDL_MOUSEBUTTONDOWN:                       // 按下事件
                //     if (event.button.button == SDL_BUTTON_LEFT) // 左键按下
                //         disturb(WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, event.button.x, event.button.y, 5, 2560, buf);
                //     break;
            }
        }

        // 清空屏幕
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        {
            if (time1 - time2 > 1000)
            {
                colorp1A = colorp1B;
                colorp2A = colorp2B;

                colorp1B = (colorp1B == color1 + 6) ? color1 : colorp1B + 1;
                colorp2B = (colorp2B == color2 + 6) ? color2 : colorp2B + 1;

                time2 = time1;
            }
            float t = (time1 - time2) / 1000.0f;
            // A->B
            vertex[0].color.r = vertex[3].color.r = colorp1A->r + (colorp1B->r - colorp1A->r) * (powf((t - 0.5f), 2.0f) + 0.5f);
            vertex[0].color.g = vertex[3].color.g = colorp1A->g + (colorp1B->g - colorp1A->g) * (powf((t - 0.5f), 2.0f) + 0.5f);
            vertex[0].color.b = vertex[3].color.b = colorp1A->b + (colorp1B->b - colorp1A->b) * (powf((t - 0.5f), 2.0f) + 0.5f);

            vertex[1].color.r = vertex[2].color.r = colorp2A->r + (colorp2B->r - colorp2A->r) * (powf((t - 0.5f), 2.0f) + 0.5f);
            vertex[1].color.g = vertex[2].color.g = colorp2A->g + (colorp2B->g - colorp2A->g) * (powf((t - 0.5f), 2.0f) + 0.5f);
            vertex[1].color.b = vertex[2].color.b = colorp2A->b + (colorp2B->b - colorp2A->b) * (powf((t - 0.5f), 2.0f) + 0.5f);
        }

        // // 计算下一帧波幅
        // nextFrame(WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, &buf, &buf2);
        // // 渲染波纹
        // RenderRipple(WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, buf, color_src, color_tar);

        // // 更新纹理
        // SDL_LockTexture(texture, NULL, (void **)&color_tar, &surface->pitch);
        // SDL_UpdateTexture(texture, NULL, color_tar, surface->pitch);
        // SDL_UnlockTexture(texture);

        SDL_RenderGeometry(renderer, NULL, vertex, 4, index, 6);

        SDL_Delay(1000 / 60);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);

    free(buf);
    free(buf2);

    SDL_FreeSurface(surface);
    SDL_FreeSurface(surface_tar);
    SDL_FreeSurface(surface_src);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

// gcc main.c -o main -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lm
#undef main
void main()
{
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP);
    TTF_Init();

    Play();

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return;
}