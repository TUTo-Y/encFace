/**
 * 渲染和绘制
 */
#ifndef _DRAW_H
#define _DRAW_H

#include <math.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "config.h"
#include "data.h"

#define PI  3.1415926f

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
// 获取最大长度，用于处理人物名片
#define MAX_OF_THREE(a, b, c) (MAX(MAX(a, b), c))

/**
 * \brief 设置圆角边框
*/
void setSurfaceRoundedBorder(SDL_Surface *surface, int radius, SDL_Color color);

/**
 * \brief 绘制圆
 */
void drawCircle(SDL_Renderer *renderer, float centerX, float centerY, float radius, SDL_Color *color);

/**
 * \brief 绘制八个对称小球
 * \param centerX 圆心x
 * \param centerY 圆心y
 * \param radius 大圆的半径
 * \param r 小圆的半径
 * \param O 八个小球的旋转角度
 * \param color 颜色指针
 */
void drawCircle8(SDL_Renderer *renderer, float centerX, float centerY, float radius, float r, float O, SDL_Color *color);

/**
 * \brief 将用户信息渲染成Surface
 */
bool renderInfo2Surface();

#endif // _DRAW_H