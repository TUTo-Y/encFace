#ifndef _GUI_TEXT_H
#define _GUI_TEXT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "draw.h"

typedef struct _gui_text
{
    SDL_Rect rect;     // 文本框位置
    SDL_Rect textRect; // 文本位置

    char *textIn;        // 已输入文本内容
    char *textSub;       // 候选文本内容
    SDL_Color textColor; // 文本颜色

    SDL_Surface *surface; // 文本表面
    SDL_Texture *texture; // 文本纹理

    bool isRun; // 是否正在处理文本框
} guiText;

/**
 * \brief 初始化文本框
 */
void gtInit(guiText *gt, SDL_Rect rect, SDL_Rect textRect, SDL_Color textColor);

/**
 * \brief 处理文本框事件
 */
void gtEvent(guiText *gt, SDL_Event *e, SDL_Renderer *renderer);

/**
 * \brief 渲染文本
 */
void gtRender(guiText *gt, SDL_Renderer *renderer);

/**
 * \brief 释放文本框
 */
void gtFree(guiText *gt);

#endif // _GUI_TEXT_H