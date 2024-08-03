#ifndef _GUI_TEXT_H
#define _GUI_TEXT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "draw.h"

enum
{
    guiTextEnum_selected,       // 选中状态
    guiTextEnum_selected_start, // 刚进入选中状态
    guiTextEnum_selected_end,   // 结束选中状态
    guiTextEnum_hide,           // 隐藏状态
    guiTextEnum_center          // 居中状态
};

typedef struct _gui_text
{
    int flag; // 文本框状态

    SDL_Rect rect; // 文本框位置
    float scale;   // 文本框文本高度所占比例
    int lineWidth; // 文本框横线宽度

    TTF_Font *font;          // 文本框字体
    char *textInit;          // 初始文本内容
    char *textIn;            // 已输入文本内容
    char *textSub;           // 候选文本内容
    SDL_Color textInitColor; // 初始化文本颜色
    SDL_Color textColor;     // 已输入和候选文本颜色

    SDL_Color lineNormalColor;   // 一般横线颜色
    SDL_Color lineSelectedColor; // 选中横线颜色
    int start_time;              // 开始时动画时间(ms)
    int end_time;                // 结束时动画时间(ms)

    SDL_Texture *texture; // 文本纹理

    SDL_Renderer *renderer; // 渲染器

    int time; // 用于计时
} guiText;

/**
 * \brief 初始化文本框
 * \param gt 文本框指针
 * \param rect 文本框位置
 * \param scale 文本框文本高度所占比例
 * \param lineWidth 文本框横线宽度
 * \param font 文本框字体
 * \param text 初始文本内容
 * \param textInitColor 初始化文本颜色
 * \param textColor 已输入和候选文本颜色
 * \param lineNormalColor 一般横线颜色
 * \param lineSelectedColor 选中横线颜色
 * \param start_time 开始时动画时间(ms)
 * \param end_time 结束时动画时间(ms)
 * \param renderer 渲染器
 * \return void
 */
void gtInit(guiText *gt,
            SDL_Rect rect,
            float scale,             // 文本框文本高度所占比例
            int lineWidth,           // 文本框横线宽度
            TTF_Font *font,          // 文本框字体
            char *text,              // 初始文本内容
            SDL_Color textInitColor, // 初始化文本颜色
            SDL_Color textColor,     // 已输入和候选文本颜色

            SDL_Color lineNormalColor,   // 一般横线颜色
            SDL_Color lineSelectedColor, // 选中横线颜色
            int start_time,              // 开始时动画时间(ms)
            int end_time,                // 结束时动画时间(ms)
            SDL_Renderer *renderer);

/**
 * \brief 设置文本
 * \param gt 文本框指针
 * \return void
 */
void gtSetText(guiText *gt, char *text);

/**
 * \brief 重新渲染文本
 * \param gt 文本框指针
 * \return void
 * \note 通常情况下会自动调用
 */
void gtReRender(guiText *gt);

/**
 * \brief 设置文本框中心对齐
 * \param gt 文本框指针
 * \param center 是否居中
 * \return void
 */
void gtSetCenter(guiText *gt, bool center);

/**
 * \brief 设置文本框隐藏状态
 * \param gt 文本框指针
 * \param hide 隐藏状态
 * \return void
 */
void gtSetHide(guiText *gt, bool hide);

/**
 * \brief 处理文本框事件
 */
void gtEvent(guiText *gt, SDL_Event *e);

/**
 * \brief 渲染文本
 */
void gtRender(guiText *gt);

/**
 * \brief 释放文本框
 */
void gtDestroy(guiText *gt);

#endif // _GUI_TEXT_H