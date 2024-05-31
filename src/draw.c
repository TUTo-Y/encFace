#include "draw.h"

/**
 * \brief 绘制圆
 */
void drawCircle(SDL_Renderer *renderer, float centerX, float centerY, float radius, SDL_Color *color)
{
    int *index = NULL;
    SDL_Vertex *vertex = NULL;
    int vertexNum = (int)(2.0f * PI * radius); // 顶点数

    // 检查Renderer是否存在
    if (!renderer || !color)
        return;

    vertex = (SDL_Vertex *)malloc(sizeof(SDL_Vertex) * vertexNum);
    index = (int *)malloc(sizeof(int) * (vertexNum - 2) * 3);

    // 计算所有顶点
    for (int i = 0; i < vertexNum; i++)
    {
        vertex[i].color = *color;
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

/**
 * \brief 绘制n个对称小球
 * \param centerX 大圆心x
 * \param centerY 大圆心y
 * \param R 大圆的半径
 * \param r 小圆的半径
 * \param color 颜色指针
 * \param O 小球的旋转角度
 * \param n 小球个数
 */
void drawCircleN(SDL_Renderer *renderer, float centerX, float centerY, float R, float r, SDL_Color *color, float O, int n)
{
    if (!color)
        return;
    for (int o = 0; o < n; o++)
        drawCircle(renderer,
                   centerX + R * cosf(O + 2.0f * (float)o * PI / (float)n),
                   centerY + R * sinf(O + 2.0f * (float)o * PI / (float)n),
                   r, &color[o]);
}

/**
 * \brief 绘制抗锯齿的圆角边框
 * \param renderer 渲染器
 * \param texture 纹理
 * \param dstrect 渲染目标矩形
 * \param radius 半径, 在0-1之间，表示占矩形边长的比例
 */
void drawRoundedBorderTextureF(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_FRect *dstrect, float radius)
{
    int sWidth, sHeight; // 纹理宽高
    float Radius;        // 实际半径
    float radiusX;       // x轴半径
    float radiusY;       // y轴半径

    // 检查参数是否存在
    if (!renderer || !texture || !dstrect)
        return;

    // 获取纹理宽高
    SDL_QueryTexture(texture, NULL, NULL, &sWidth, &sHeight);

    // 计算出radius
    float r = MIN(sWidth, sHeight) * radius;
    radiusX = r / sWidth;
    radiusY = r / sHeight;

    // 计算出Radius
    Radius = MIN(dstrect->w, dstrect->h) * radius;

    // // 渲染四个顶角
    int *index = NULL;
    SDL_Vertex *vertex = NULL;

    // int vertexNum = TO32((int)(PI * (2.0f * Radius + 2.0f * Radius) * (1.0f + 3.0f * fabsf(2.0f * Radius - 2.0f * Radius) / (10.0f * (2.0f * Radius + 2.0f * Radius))))); // 边缘顶点数
    int vertexNum = TO32((int)(2.0f * PI * Radius));

    vertex = (SDL_Vertex *)malloc(sizeof(SDL_Vertex) * (vertexNum + 4 + 4));
    index = (int *)malloc(sizeof(int) * (vertexNum + 6) * 3);

    // 右上角
    vertex[vertexNum + 4 + 0].color = (SDL_Color){0xFF, 0xFF, 0xFF, 0xFF};
    vertex[vertexNum + 4 + 0].position.x = dstrect->x + dstrect->w - Radius;
    vertex[vertexNum + 4 + 0].position.y = dstrect->y + Radius;
    vertex[vertexNum + 4 + 0].tex_coord.x = 1.0f - radiusX;
    vertex[vertexNum + 4 + 0].tex_coord.y = radiusY;

    // 左上角
    vertex[vertexNum + 4 + 1].color = (SDL_Color){0xFF, 0xFF, 0xFF, 0xFF};
    vertex[vertexNum + 4 + 1].position.x = dstrect->x + Radius;
    vertex[vertexNum + 4 + 1].position.y = dstrect->y + Radius;
    vertex[vertexNum + 4 + 1].tex_coord.x = radiusX;
    vertex[vertexNum + 4 + 1].tex_coord.y = radiusY;

    // 左下角
    vertex[vertexNum + 4 + 2].color = (SDL_Color){0xFF, 0xFF, 0xFF, 0xFF};
    vertex[vertexNum + 4 + 2].position.x = dstrect->x + Radius;
    vertex[vertexNum + 4 + 2].position.y = dstrect->y + dstrect->h - Radius;
    vertex[vertexNum + 4 + 2].tex_coord.x = radiusX;
    vertex[vertexNum + 4 + 2].tex_coord.y = 1.0f - radiusY;

    // 右下角
    vertex[vertexNum + 4 + 3].color = (SDL_Color){0xFF, 0xFF, 0xFF, 0xFF};
    vertex[vertexNum + 4 + 3].position.x = dstrect->x + dstrect->w - Radius;
    vertex[vertexNum + 4 + 3].position.y = dstrect->y + dstrect->h - Radius;
    vertex[vertexNum + 4 + 3].tex_coord.x = 1.0f - radiusX;
    vertex[vertexNum + 4 + 3].tex_coord.y = 1.0f - radiusY;

    int *indexP = NULL;
    SDL_Vertex *vertexP = NULL;
    indexP = index;
    vertexP = vertex;

    for (int i = 0; i < 4; i++)
    {
        int j = i * vertexNum / 4;
        for (; j < (i + 1) * vertexNum / 4; j++, vertexP++)
        {
            // 设置点
            vertexP->color = (SDL_Color){0xFF, 0xFF, 0xFF, 0xFF};
            vertexP->position.x = vertex[vertexNum + 4 + i].position.x + Radius * cosf(2.0f * PI * (float)j / (float)vertexNum);
            vertexP->position.y = vertex[vertexNum + 4 + i].position.y - Radius * sinf(2.0f * PI * (float)j / (float)vertexNum);
            vertexP->tex_coord.x = vertex[vertexNum + 4 + i].tex_coord.x + radiusX * cosf(2.0f * PI * (float)j / (float)vertexNum);
            vertexP->tex_coord.y = vertex[vertexNum + 4 + i].tex_coord.y - radiusY * sinf(2.0f * PI * (float)j / (float)vertexNum);

            // 设置索引
            *(indexP++) = vertexNum + 4 + i;
            *(indexP++) = (*(indexP++) = vertexP - vertex) + 1;
        }
        vertexP->color = (SDL_Color){0xFF, 0xFF, 0xFF, 0xFF};
        vertexP->position.x = vertex[vertexNum + 4 + i].position.x + Radius * cosf(2.0f * PI * (float)j / (float)vertexNum);
        vertexP->position.y = vertex[vertexNum + 4 + i].position.y - Radius * sinf(2.0f * PI * (float)j / (float)vertexNum);
        vertexP->tex_coord.x = vertex[vertexNum + 4 + i].tex_coord.x + radiusX * cosf(2.0f * PI * (float)j / (float)vertexNum);
        vertexP->tex_coord.y = vertex[vertexNum + 4 + i].tex_coord.y - radiusY * sinf(2.0f * PI * (float)j / (float)vertexNum);
        vertexP++;
    }

    // 渲染主体部分
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

    // SDL_Vertex vertex[12] = {0};
    // int vertexNum[6 * 3] = {0};

    SDL_RenderGeometry(renderer, texture, vertex, vertexNum + 4 + 4, index, (vertexNum + 6) * 3);
    free(vertex);
    free(index);
}

/**
 * \brief 将用户信息渲染成Texture
 */
bool renderInfo2Surface(SDL_Renderer *renderer, list *face, list **faceTexture, list **faceSurface)
{
    char text[256] = {0};
    list *tmp = NULL;

    // 设置文本颜色
    SDL_Color colorHV = {0, 255, 0, 255}; // 人物存在
    SDL_Color colorNO = {255, 0, 0, 255}; // 人物不存在

    // Surface
    SDL_Surface *surface1 = NULL;
    SDL_Surface *surface2 = NULL;
    SDL_Surface *surface3 = NULL;
    SDL_Surface *surface = NULL;

    // 渲染名片到tmp链表
    list *node = face;
    while (node)
    {
        if (((vector *)node->data)->flag == HV) // 人物存在
        {
            // 渲染人物姓名
            snprintf(text, 256, "姓名:%s", ((vector *)node->data)->info.name);
            surface1 = TTF_RenderUTF8_Blended(Global.font, text, colorHV);

            // 渲染人物年龄
            snprintf(text, 256, "年龄:%d", ((vector *)node->data)->info.age);
            surface2 = TTF_RenderUTF8_Blended(Global.font, text, colorHV);

            // 渲染人物性别
            switch (((vector *)node->data)->info.sex)
            {
            case MAN: // 男性
                strcpy(text, "性别:男性");
                break;
            case WOM: // 女性
                strcpy(text, "性别:女性");
                break;
            case SEX: // 未知
            default:
                strcpy(text, "性别:未知");
                break;
            }
            surface3 = TTF_RenderUTF8_Blended(Global.font, text, colorHV);

            // 创建人物信息框
            surface = SDL_CreateRGBSurfaceWithFormat(0, MAX_OF_THREE(surface1->w, surface2->w, surface3->w), surface1->h + surface2->h + surface3->h + 20, 32, SDL_PIXELFORMAT_RGBA32);

            // 用浅透明的淡蓝色去填充背景
            SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 128, 128, 255, 64));

            // 将surface1~3渲染到surface
            SDL_Rect rect = {0};
            SDL_BlitSurface(surface1, NULL, surface, NULL);
            rect.x = 0;
            rect.y = surface1->h + 10;
            rect.w = surface2->w;
            rect.h = surface2->h;
            SDL_BlitSurface(surface2, NULL, surface, &rect);
            rect.x = 0;
            rect.y = surface1->h + surface2->h + 20;
            rect.w = surface3->w;
            rect.h = surface3->h;
            SDL_BlitSurface(surface3, NULL, surface, &rect);

            // 添加到链表
            addData(&tmp, surface, 0, false);

            // 释放资源
            SDL_FreeSurface(surface1);
            SDL_FreeSurface(surface2);
            SDL_FreeSurface(surface3);
        }
        else // 人物不存在
        {
            // 人物要显示的信息信息
            addData(&tmp, TTF_RenderUTF8_Blended(Global.font, "no info", colorNO), 0, false);
        }

        node = node->next;
    }

    // 将tmp链表的surface转换成texture
    node = tmp;
    while (node)
    {
        addData(faceTexture, SDL_CreateTextureFromSurface(renderer, (SDL_Surface *)node->data), 0, false);
        node = node->next;
    }

    // 将tmp链表的surface放入surface ( 这一步用于保持正方向放入链表 )
    while (tmp)
    {
        void *data = NULL;
        if (getData(&tmp, &data) == false)
            break;
        addData(faceSurface, data, 0, false);
    }

    return true;
}

/**
 * \brief 将图片缩放到指定区间
 * \param total 总区间
 * \param dRect 目标区间
 * \param w 图片宽度
 * \param h 图片高度
 */
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

/**
 * \brief 检测当前位置是否在按钮上
 */
bool checkPointInCircle(SDL_FPoint point, float x, float y, float r)
{
    if ((point.x - x) * (point.x - x) + (point.y - y) * (point.y - y) <= r * r)
        return true;
    return false;
}

/**
 * \brief 生成一个圆角边框的texture的渲染数据
 * \param texture (可选) 纹理
 * \param textureW (可选) 纹理宽度
 * \param textureH (可选) 纹理高度
 * \param dstrect 渲染目标矩形
 * \param radius 半径, 在0-1之间
 * \param index (返回) 索引数组
 * \param vertex (返回) 顶点数组
 * \warning texture和textureW/H至少存在一个
 * \warning radius会根据最小边长自动调整
 */
void getRoundedBorder(SDL_Texture *texture, int textureW, int textureH, const SDL_FRect *dstrect, float radius, data **index, data **vertex)
{
    float Radius;  // 目标区域半径
    float radiusX; // 纹理半径
    float radiusY; // 纹理半径
    int vertexNum; // 圆角上的顶点数量

    // 检查参数
    if (!(texture || (textureW && textureH)) || !dstrect || !index || !vertex)
        return;

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
    if (*vertex)
        Free(*vertex);
    if (*index)
        Free(*index);
    *vertex = Malloc(sizeof(SDL_Vertex) * (vertexNum + 4 + 4));
    *index = Malloc(sizeof(int) * (vertexNum + 6) * 3);

    /* 计算四个圆角的中心点 */
    // 右上角
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 0].color = (SDL_Color){0xFF, 0xFF, 0xFF, 0xFF};
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 0].position.x = dstrect->x + dstrect->w - Radius;
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 0].position.y = dstrect->y + Radius;
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 0].tex_coord.x = 1.0f - radiusX;
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 0].tex_coord.y = radiusY;

    // 左上角
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 1].color = (SDL_Color){0xFF, 0xFF, 0xFF, 0xFF};
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 1].position.x = dstrect->x + Radius;
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 1].position.y = dstrect->y + Radius;
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 1].tex_coord.x = radiusX;
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 1].tex_coord.y = radiusY;

    // 左下角
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 2].color = (SDL_Color){0xFF, 0xFF, 0xFF, 0xFF};
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 2].position.x = dstrect->x + Radius;
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 2].position.y = dstrect->y + dstrect->h - Radius;
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 2].tex_coord.x = radiusX;
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 2].tex_coord.y = 1.0f - radiusY;

    // 右下角
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 3].color = (SDL_Color){0xFF, 0xFF, 0xFF, 0xFF};
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 3].position.x = dstrect->x + dstrect->w - Radius;
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 3].position.y = dstrect->y + dstrect->h - Radius;
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 3].tex_coord.x = 1.0f - radiusX;
    ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + 3].tex_coord.y = 1.0f - radiusY;

    /* 计算所有圆角的顶点和索引 */
    int *indexP = ((int *)((*index)->data));
    SDL_Vertex *vertexP = ((SDL_Vertex *)(*vertex)->data);
    for (int i = 0; i < 4; i++)
    {
        int j = i * vertexNum / 4;
        for (; j < (i + 1) * vertexNum / 4; j++, vertexP++)
        {
            // 设置点
            vertexP->color = (SDL_Color){0xFF, 0xFF, 0xFF, 0xFF};
            vertexP->position.x = ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + i].position.x + Radius * cosf(2.0f * PI * (float)j / (float)vertexNum);
            vertexP->position.y = ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + i].position.y - Radius * sinf(2.0f * PI * (float)j / (float)vertexNum);
            vertexP->tex_coord.x = ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + i].tex_coord.x + radiusX * cosf(2.0f * PI * (float)j / (float)vertexNum);
            vertexP->tex_coord.y = ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + i].tex_coord.y - radiusY * sinf(2.0f * PI * (float)j / (float)vertexNum);

            // 设置索引
            *(indexP++) = vertexNum + 4 + i;
            *(indexP++) = (*(indexP++) = vertexP - (SDL_Vertex *)(*vertex)->data) + 1;
        }
        vertexP->color = (SDL_Color){0xFF, 0xFF, 0xFF, 0xFF};
        vertexP->position.x = ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + i].position.x + Radius * cosf(2.0f * PI * (float)j / (float)vertexNum);
        vertexP->position.y = ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + i].position.y - Radius * sinf(2.0f * PI * (float)j / (float)vertexNum);
        vertexP->tex_coord.x = ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + i].tex_coord.x + radiusX * cosf(2.0f * PI * (float)j / (float)vertexNum);
        vertexP->tex_coord.y = ((SDL_Vertex *)(*vertex)->data)[vertexNum + 4 + i].tex_coord.y - radiusY * sinf(2.0f * PI * (float)j / (float)vertexNum);
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