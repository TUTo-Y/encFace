#include "draw.h"

/**
 * \brief 生成一个圆的texture
 * \param r 半径
 * \param color 颜色
 * \param renderer 渲染器
 * \param SR 超级分辨率
 */
SDL_Texture *drawCircle(int r, SDL_Color color, SDL_Renderer *renderer, bool SR)
{
    if (SR)
        r *= 2;
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, r * 2, r * 2, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);

    for (int i = 0; i < surface->w; i++)
    {
        for (int j = 0; j < surface->h; j++)
        {
            int dx = i - r;
            int dy = j - r;
            if (dx * dx + dy * dy <= r * r)
            {
                ((Uint32 *)surface->pixels)[j * surface->w + i] = SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a);
            }
            else
            {
                ((Uint32 *)surface->pixels)[j * surface->w + i] = 0x00000000;
            }
        }
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(surface);
    return texture;
}

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
void drawCircle8(SDL_FPoint *points, float R, float r, float O, int a[8], SDL_Texture *circle, SDL_Renderer *renderer)
{
    Uint8 Alpha;

    // 保存原来的透明度
    SDL_GetTextureAlphaMod(circle, &Alpha);
    for (int o = 0; o < 8; o++)
    {
        // 设置透明度
        SDL_SetTextureAlphaMod(circle, a[o]);

        // 绘制圆
        float x = points->x + R * cosf(O + 2.0f * (float)o * PI / (float)8.0f);
        float y = points->y + R * sinf(O + 2.0f * (float)o * PI / (float)8.0f);
        SDL_Rect dstrect = {x - r, y - r, r * 2, r * 2};
        SDL_RenderCopy(renderer, circle, NULL, &dstrect);
    }

    // 恢复透明度
    SDL_SetTextureAlphaMod(circle, Alpha);
}

int checkPointInCircle(SDL_FPoint point, float x, float y, float r)
{
    return (point.x - x) * (point.x - x) + (point.y - y) * (point.y - y) <= r * r;
}

void resizeImage(const SDL_Rect *total, SDL_FRect *dRect, int w, int h)
{
    if ((float)w / (float)h > (float)total->w / (float)total->h)
    {
        dRect->w = (float)total->w;
        dRect->h = (float)h * (float)total->w / (float)w;
        dRect->x = (float)0;
        dRect->y = ((float)total->h - dRect->h) / 2.0f;
    }
    else
    {
        dRect->h = (float)total->h;
        dRect->w = (float)w * (float)total->h / (float)h;
        dRect->y = (float)0;
        dRect->x = ((float)total->w - dRect->w) / 2.0f;
    }
}

void freeEBO(ebo *ebo)
{
    if (ebo->vertex)
    {
        Free(ebo->vertex);
        ebo->vertex = NULL;
    }
    if (ebo->index)
    {
        Free(ebo->index);
        ebo->index = NULL;
    }
}

void getRoundedBorder(SDL_Texture *texture, int textureW, int textureH, const SDL_FRect *dstrect, float radius, ebo *ebo, SDL_Color *color)
{

    unsigned int colorDefault = 0xFFFFFFFF; // 默认颜色
    float Radius;                           // 目标区域半径
    float radiusX;                          // 纹理半径
    float radiusY;                          // 纹理半径
    int vertexNum;                          // 圆角上的顶点数量

    // 检查参数
    if (!(texture || (textureW && textureH)) || !dstrect || !ebo)
        return;

    if (!color)
        color = (SDL_Color *)&colorDefault;

    // 获取纹理宽高
    if (texture)
    {
        // 获取纹理宽高
        SDL_QueryTexture(texture, NULL, NULL, &textureW, &textureH);
    }

    // 计算出radius
    float r = MIN(textureW, textureH) * radius;
    radiusX = r / textureW;
    radiusY = r / textureH;

    // 计算出Radius
    Radius = MIN(dstrect->w, dstrect->h) * radius;

    // 实际大小
    vertexNum = TO32((int)(2.0f * PI * Radius));

    // 申请顶点缓存和索引缓存的空间
    freeEBO(ebo);
    ebo->vertex = Malloc(sizeof(SDL_Vertex) * (vertexNum + 4 + 4));
    ebo->index = Malloc(sizeof(int) * (vertexNum + 6) * 3);

    /* 计算四个圆角的中心点 */
    // 右上角
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 0].color = *color;
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 0].position.x = dstrect->x + dstrect->w - Radius;
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 0].position.y = dstrect->y + Radius;
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 0].tex_coord.x = 1.0f - radiusX;
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 0].tex_coord.y = radiusY;

    // 左上角
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 1].color = *color;
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 1].position.x = dstrect->x + Radius;
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 1].position.y = dstrect->y + Radius;
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 1].tex_coord.x = radiusX;
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 1].tex_coord.y = radiusY;

    // 左下角
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 2].color = *color;
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 2].position.x = dstrect->x + Radius;
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 2].position.y = dstrect->y + dstrect->h - Radius;
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 2].tex_coord.x = radiusX;
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 2].tex_coord.y = 1.0f - radiusY;

    // 右下角
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 3].color = *color;
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 3].position.x = dstrect->x + dstrect->w - Radius;
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 3].position.y = dstrect->y + dstrect->h - Radius;
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 3].tex_coord.x = 1.0f - radiusX;
    ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + 3].tex_coord.y = 1.0f - radiusY;

    /* 计算所有圆角的顶点和索引 */
    int *indexP = (int *)(ebo->index->data);
    SDL_Vertex *vertexP = ((SDL_Vertex *)(ebo->vertex->data));
    for (int i = 0; i < 4; i++)
    {
        int j = i * vertexNum / 4;
        for (; j < (i + 1) * vertexNum / 4; j++, vertexP++)
        {
            // 设置点
            vertexP->color = *color;
            vertexP->position.x = ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + i].position.x + Radius * cosf(2.0f * PI * (float)j / (float)vertexNum);
            vertexP->position.y = ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + i].position.y - Radius * sinf(2.0f * PI * (float)j / (float)vertexNum);
            vertexP->tex_coord.x = ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + i].tex_coord.x + radiusX * cosf(2.0f * PI * (float)j / (float)vertexNum);
            vertexP->tex_coord.y = ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + i].tex_coord.y - radiusY * sinf(2.0f * PI * (float)j / (float)vertexNum);

            // 设置索引
            *(indexP++) = vertexNum + 4 + i;
            *(indexP++) = (*(indexP++) = vertexP - ((SDL_Vertex *)(ebo->vertex->data))) + 1;
        }
        vertexP->color = *color;
        vertexP->position.x = ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + i].position.x + Radius * cosf(2.0f * PI * (float)j / (float)vertexNum);
        vertexP->position.y = ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + i].position.y - Radius * sinf(2.0f * PI * (float)j / (float)vertexNum);
        vertexP->tex_coord.x = ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + i].tex_coord.x + radiusX * cosf(2.0f * PI * (float)j / (float)vertexNum);
        vertexP->tex_coord.y = ((SDL_Vertex *)(ebo->vertex->data))[vertexNum + 4 + i].tex_coord.y - radiusY * sinf(2.0f * PI * (float)j / (float)vertexNum);
        vertexP++;
    }

    int tmp[] = {
        0 * (vertexNum / 4) + 0,
        1 * (vertexNum / 4) + 0,
        1 * (vertexNum / 4) + 1,
        2 * (vertexNum / 4) + 1,
        2 * (vertexNum / 4) + 2,
        3 * (vertexNum / 4) + 2,
        3 * (vertexNum / 4) + 3,
        4 * (vertexNum / 4) + 3,

        vertexNum + 4 + 0,
        vertexNum + 4 + 1,
        vertexNum + 4 + 2,
        vertexNum + 4 + 3};
    *(indexP++) = tmp[0];
    *(indexP++) = tmp[3];
    *(indexP++) = tmp[4];
    *(indexP++) = tmp[4];
    *(indexP++) = tmp[7];
    *(indexP++) = tmp[0];
    *(indexP++) = tmp[1];
    *(indexP++) = tmp[2];
    *(indexP++) = tmp[9];
    *(indexP++) = tmp[9];
    *(indexP++) = tmp[8];
    *(indexP++) = tmp[1];
    *(indexP++) = tmp[11];
    *(indexP++) = tmp[10];
    *(indexP++) = tmp[5];
    *(indexP++) = tmp[5];
    *(indexP++) = tmp[6];
    *(indexP++) = tmp[11];
}

/**
 * \brief 渲染一个矩形
 * \param w 矩形的宽度
 * \param h 矩形的高度
 * \param color 矩形的颜色[左上开始，顺时针]
 * \param renderer 渲染器
 * \param texture 纹理(可选)
 * \return 返回渲染的纹理
 */
SDL_Texture *drawRect(int w, int h, SDL_Color color[4], SDL_Renderer *renderer, SDL_Texture *texture)
{
    // 创建目标纹理
    SDL_Texture *Texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, w, h);
    SDL_SetTextureBlendMode(Texture, SDL_BLENDMODE_BLEND);

    // 渲染颜色及其纹理
    SDL_SetRenderTarget(renderer, Texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_Vertex vertex[4] = {
        {.color = color[0], .position = {0, 0}, .tex_coord = {0.0f, 0.0f}},
        {.color = color[1], .position = {w, 0}, .tex_coord = {1.0f, 0.0f}},
        {.color = color[2], .position = {w, h}, .tex_coord = {1.0f, 1.0f}},
        {.color = color[3], .position = {0, h}, .tex_coord = {0.0f, 1.0f}},
    };
    int index[6] = {0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(renderer, texture, vertex, 4, index, 6);
    SDL_SetRenderTarget(renderer, NULL);

    return Texture;
}

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
SDL_Texture *drawRoundRect(int w, int h, SDL_Color color[4], float radius, SDL_Renderer *renderer, SDL_Texture *texture)
{
    // 创建目标纹理
    SDL_Texture *TextureColor = drawRect(w, h, color, renderer, texture);

    // 渲染圆角
    SDL_SetRenderTarget(renderer, TextureColor);
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, surface->pixels, surface->pitch);

    // 计算圆角半径
    int r = (float)MIN(w, h) * radius;
    int r_squared = r * r;

    // 优化后的圆角处理
    Uint32 *pixels = (Uint32 *)surface->pixels;
    for (int y = 0; y < r; y++)
    {
        for (int x = 0; x < r; x++)
        {
            int dx = x - r;
            int dy = y - r;
            if (dx * dx + dy * dy > r_squared)
            {
                // 左上角
                pixels[y * w + x] = 0x00000000;
                // 右上角
                pixels[y * w + (w - 1 - x)] = 0x00000000;
                // 左下角
                pixels[(h - 1 - y) * w + x] = 0x00000000;
                // 右下角
                pixels[(h - 1 - y) * w + (w - 1 - x)] = 0x00000000;
            }
        }
    }

    // 计算出最终纹理
    SDL_Texture *Texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(Texture, SDL_BLENDMODE_BLEND);

    // 清理资源
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(TextureColor);
    SDL_SetRenderTarget(renderer, NULL);

    return Texture;
}