#ifndef _MAIN_H
#define _MAIN_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <gtk-3.0/gtk/gtk.h>
#include <pthread.h>

#include "config.h"
#include "data.h"
#include "gui_play.h"
#include "enc.h"
#include "web.h"
#include "draw.h"

/**
 * \brief 选择图片
 */
bool selectImageFile(char *path, size_t size);

// 加载人脸数据
bool choiceImage(
                 const SDL_Rect *windowRect,
                 SDL_Renderer *renderer,
                 SDL_Surface **surface, SDL_Texture **texture,
                 SDL_FRect *surfaceRect, float *scale, float *scale2,
                 list **face, list **faceSurface, list **faceTexture);
                 
/**
 * \brief 使用多线程加载数据
 */
void *th(void *arg);

// 检查线程是否存在
bool getThread();

// 设置线程状态
void setThread(bool status);

#endif // _MAIN_H