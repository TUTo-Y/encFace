#ifndef _MAIN_H
#define _MAIN_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <stdbool.h>
#include <winsock2.h>
#include <windows.h>
#include <commdlg.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "config.h"
#include "data.h"
#include "vector.h"
#include "gui.h"
#include "enc.h"
#include "web.h"

struct _Window
{
    SDL_Window *window;     // 窗口
    SDL_Renderer *renderer; // 渲染器

    SDL_Rect windowRect;   // 窗口区域
    SDL_FRect surfaceRect; // 图片区域

    SDL_Surface *surface; // 照片Surface
    SDL_Texture *texture; // 照片Texture

    float scale;  // 图片缩放比例
    float scale2; // 图片二次缩放比例

    list *face;        // 人脸列表
    list *faceSurface; // 人脸Surface列表
    list *faceTexture; // 人脸Texture列表
};
extern struct _Window Global;

/**
 * \brief 选择图片
 */
bool select_image(char *path, size_t size);

#endif // _MAIN_H