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
#include "draw.h"

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
void resizeImage(const SDL_Rect *total, SDL_FRect *dRect, int w, int h);

/**
 * 获取当前按钮标准大小
*/
float getButtonRect();

/**
 * \brief 根据按钮位置设置十字
*/
void setCross();

/**
 * \brief 重置按钮位置
*/
void resetButton();

/**
 * \brief 检测当前位置是否在按钮上
 */
bool checkButton(SDL_FPoint point, float x, float y, float r);

#endif // _GUI_H