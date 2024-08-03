/**
 * 渲染和绘制
 */
#ifndef _DRAW_H
#define _DRAW_H

#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "config.h"
#include "data_base.h"

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
 * \brief 生成一个圆的texture
 * \param r 半径
 * \param color 颜色
 * \param renderer 渲染器
 * \param SR 超级分辨率
 */
SDL_Texture *drawCircle(int r, SDL_Color color, SDL_Renderer *renderer, bool SR);

/**
 * \brief 绘制8个圆
 * \param points 圆心
 * \param R 大圆半径
 * \param r 小圆半径
 * \param O 旋转角度
 * \param a 圆的透明度
 * \param circle 圆的纹理
 * \param renderer 渲染器
 */
void drawCircle8(SDL_FPoint *points, float R, float r, float O, int a[8], SDL_Texture *circle, SDL_Renderer *renderer);

/**
 * \brief 检测点位置是否在圆上
 * \param point 点
 * \param x 圆的x
 * \param y 圆的y
 * \param r 圆的z
 * \return 0不在圆上
 */
int checkPointInCircle(SDL_FPoint point, float x, float y, float r);

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


/**
 * \brief 渲染一个矩形
 * \param w 矩形的宽度
 * \param h 矩形的高度
 * \param color 矩形的颜色[左上开始，顺时针]
 * \param renderer 渲染器
 * \param texture 纹理(可选)
 * \return 返回渲染的纹理
 */
SDL_Texture *drawRect(int w, int h, SDL_Color color[4], SDL_Renderer *renderer, SDL_Texture *texture);

/**
 * \brief 渲染一个圆角矩形
 * \param w 矩形的宽度
 * \param h 矩形的高度
 * \param color 矩形的颜色[左上开始，顺时针]
 * \param radius 矩形的圆角半径, 百分比
 * \param renderer 渲染器
 * \param texture 纹理(可选)
 * \return 返回渲染的纹理
 */
SDL_Texture *drawRoundRect(int w, int h, SDL_Color color[4], float radius, SDL_Renderer *renderer, SDL_Texture *texture);

#endif // _DRAW_H