#ifndef _GUI_BUTTON_H
#define _GUI_BUTTON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_ttf.h>

#include "draw.h"

enum
{
    guiButtonEnum_selected, // 选中状态
    guiButtonEnum_pressed,  // 按下状态
    guiButtonEnum_hide // 隐藏状态
};

enum _guiButtonEnum;

typedef struct _gui_button
{
    int flag;      // 按钮状态
    SDL_Rect Rect; // 按钮位置

    SDL_Renderer *Renderer;     // 渲染器
    SDL_Texture **Texture;      // 按钮纹理
    SDL_Texture *TextureNSP[3]; // 分别为按钮一般纹理, 选中纹理, 按下纹理
} guiButton;

/**
 * \brief 初始化按钮
 * \param gb 按钮指针
 * \param rect 按钮位置
 * \param text 按钮文本
 * \param font 按钮文本字体
 * \param color 按钮文本颜色
 * \param scale 按钮文本高度所占比例
 * \param radius 按钮圆角半径
 * \param colorNSP 分别为一般状态颜色, 选中状态颜色, 按下状态颜色
 * \param textureNSP 分别为按钮一般纹理(可选), 按钮选中纹理(可选), 按钮按下纹理(可选)
 * \param renderer 渲染器
 * \return void
 */
void gbInit(guiButton *gb,
            SDL_Rect *rect, // 按钮位置

            const char *text, // 按钮文本
            TTF_Font *font,   // 按钮文本字体
            SDL_Color *color, // 按钮文本颜色
            float scale,      // 按钮文本高度所占比例

            float radius,                  // 按钮圆角半径
            SDL_Color (*colorNSP)[3][4],   // 一般状态颜色, 选中状态颜色, 按下状态颜色
            SDL_Texture *(*textureNSP)[3], // 按钮一般纹理(可选), 按钮选中纹理(可选), 按钮按下纹理(可选)

            SDL_Renderer *renderer // 渲染器
);

/**
 * \brief 格式化按钮内容
 * \return void
 */
void gbClear(guiButton *gb);
/**
 * \brief 设置按钮隐藏状态
 * \param gb 按钮指针
 * \param hide 是否隐藏
 * \return void
 */
void gbSetHide(guiButton *gb, bool hide);

/**
 * \brief 处理按钮事件
 * \param gb 按钮指针
 * \param e 事件指针
 * \param renderer 渲染器
 * \return 是否被点击
 */
bool gbEvent(guiButton *gb, SDL_Event *e);

/**
 * \brief 渲染按钮
 */
void gbRender(guiButton *gb);

/**
 * \brief 销毁按钮
 */
void gbDestroy(guiButton *gb);

#endif // _GUI_BUTTON_H