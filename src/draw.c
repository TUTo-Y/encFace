#include "draw.h"

void drawCircle(SDL_Renderer *renderer, float centerX, float centerY, float radius, SDL_Color color)
{
    int *index = NULL;
    SDL_Vertex *vertex = NULL;
    int vertexNum = (int)(2.0f * PI * radius); // 顶点数

    // 检查Renderer是否存在
    if (!renderer)
        return;

    vertex = (SDL_Vertex *)malloc(sizeof(SDL_Vertex) * vertexNum);
    index = (int *)malloc(sizeof(int) * (vertexNum - 2) * 3);

    // 计算所有顶点
    for (int i = 0; i < vertexNum; i++)
    {
        vertex[i].color = color;
        vertex[i].position.x = centerX + radius * cosf(2.0f * PI * (float)i / (float)vertexNum); // i * PI / 180.0f
        vertex[i].position.y = centerY + radius * sinf(2.0f * PI * (float)i / (float)vertexNum); // i * PI / 180.0f
    }
    for (int i = 0; i < vertexNum - 2; i++)
    {
        index[i * 3] = 0;
        index[i * 3 + 1] = i + 1;
        index[i * 3 + 2] = i + 2;
    }

    SDL_RenderGeometry(renderer, NULL, vertex, vertexNum, index, (vertexNum - 2) * 3);

    free(vertex);
    free(index);
}

void drawCircleN(SDL_Renderer *renderer, float centerX, float centerY, float R, float r, SDL_Color *color, float O, int n)
{
    if (!renderer || !color)
        return;
    for (int o = 0; o < n; o++)
        drawCircle(renderer,
                   centerX + R * cosf(O + 2.0f * (float)o * PI / (float)n),
                   centerY + R * sinf(O + 2.0f * (float)o * PI / (float)n),
                   r, color[o]);
}

bool checkPointInCircle(SDL_FPoint point, float x, float y, float r)
{
    if ((point.x - x) * (point.x - x) + (point.y - y) * (point.y - y) <= r * r)
        return true;
    return false;
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

    if(!color)
        color = (SDL_Color*)&colorDefault;

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
