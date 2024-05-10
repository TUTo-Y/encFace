#ifndef _GUI_H
#define _GUI_H

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <commdlg.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "main.h"
#include "vector.h"


// 获取最大长度，用于处理人物名片
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX_OF_THREE(a, b, c) (MAX(MAX(a, b), c))

#define FPS 60              // 刷新率
#define FPS_MS (1000 / FPS) // 每一帧的持续时间（毫秒）

/**
 * \brief gui界面
 */
void play();

/**
 * \brief 将图片缩放到指定区间
 * \param total 区间
 * \param dRect 目标区间
 * \param w 图片宽度
 * \param h 图片高度
 */
void resize(const SDL_Rect *total, SDL_FRect *dRect, int w, int h);

/**
 * \brief 将用户信息渲染成名片
 */
bool render_info();

#endif // _GUI_H