/**
 * 渲染和绘制
 */
#ifndef _DRAW_H
#define _DRAW_H

#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "data.h"

#define PI 3.1415926f
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

// 获取最大长度
#define MAX_OF_THREE(a, b, c) MAX(MAX(a, b), c)
// 获取最小长度
#define MIN_OF_THREE(a, b, c) MIN(MIN(a, b), c)

// EBO
typedef struct _ebo ebo;
struct _ebo
{
    data *vertex; // 顶点数据(VAO)
    data *index;  // 索引数据(VBO)
};

// 通过EBO渲染
#define drawFromEBO(renderer, texture, ebo) SDL_RenderGeometry(renderer, texture, (SDL_Vertex *)((ebo).vertex->data), (ebo).vertex->size / sizeof(SDL_Vertex), (int *)((ebo).index->data), (ebo).index->size / sizeof(int));

/**
 * \brief 绘制抗锯齿圆
 * \param renderer 渲染器
 * \param centerX 圆心x
 * \param centerY 圆心y
 * \param radius 半径
 * \param color 颜色
 */
void drawCircle(SDL_Renderer *renderer, float centerX, float centerY, float radius, SDL_Color color);

/**
 * \brief 绘制n个对称小球
 * \param centerX 大圆心x
 * \param centerY 大圆心y
 * \param R 大圆的半径
 * \param r 小圆的半径
 * \param color 颜色指针
 * \param O 小球的初始化旋转角度
 * \param n 小球个数
 */
void drawCircleN(SDL_Renderer *renderer, float centerX, float centerY, float R, float r, SDL_Color *color, float O, int n);

/**
 * \brief 检测点位置是否在圆上
 * \param point 点
 * \param x 圆的x
 * \param y 圆的y
 * \param r 圆的z
 * \return 是否在圆形区域内
 */
bool checkPointInCircle(SDL_FPoint point, float x, float y, float r);

/**
 * \brief 将图片缩放到指定区间
 * \param total 总区间
 * \param dRect 目标区间
 * \param w 图片宽度
 * \param h 图片高度
 */
void resizeImage(const SDL_Rect *total, SDL_FRect *dRect, int w, int h);

/**
 * \brief 删除渲染数据
 * \param ebo 渲染数据
 */
void freeEBO(ebo *ebo);

/**
 * \brief 生成一个圆角边框的texture的渲染数据
 * \param texture (可选) 纹理
 * \param textureW (可选) 纹理宽度
 * \param textureH (可选) 纹理高度
 * \param dstrect 渲染目标矩形
 * \param radius 半径, 在0-1之间
 * \param ebo 渲染数据
 * \param color (可选)边框颜色
 * \warning texture和textureW/H至少存在一个
 * \warning radius会根据最小边长自动调整
 */
void getRoundedBorder(SDL_Texture *texture, int textureW, int textureH, const SDL_FRect *dstrect, float radius, ebo *ebo, SDL_Color *color);

#endif // _DRAW_H