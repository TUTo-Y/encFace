#include "draw.h"

/**
 * \brief 绘制圆
 */
void drawCircle(SDL_Renderer *renderer, float centerX, float centerY, float radius, SDL_Color *color)
{
    // 检查Renderer是否存在
    if (!renderer)
        return;

    if (color)
        SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);

    for (float y = -radius; y <= radius; y += 0.5f)
    {
        for (float x = -radius; x <= radius; x += 0.5f)
        {
            if (x * x + y * y <= radius * radius)
            {
                SDL_RenderDrawPointF(renderer, centerX + x, centerY + y);
            }
        }
    }
}

/**
 * \brief 绘制八个对称小球
 * \param centerX 圆心x
 * \param centerY 圆心y
 * \param radius 大圆的半径
 * \param r 小圆的半径
 * \param O 八个小球的旋转角度
 * \param color 颜色指针
 */
void drawCircle8(SDL_Renderer *renderer, float centerX, float centerY, float radius, float r, float O, SDL_Color *color)
{
    if (!color)
        return;
    for (int o = 0; o < 8; o++)
        drawCircle(Global.renderer, centerX + radius * cosf(O + 0.25f * (float)o * PI), centerY + radius * sinf(O + 0.25f * (float)o * PI), r, &color[o]);
}

/**
 * \brief 设置圆角边框
 */
void setSurfaceRoundedBorder(SDL_Surface *surface, int radius, SDL_Color color)
{
    // 检查Surface是否存在, 并且宽高是否大于2*radius
    if ((!surface) || (surface->w < 2 * radius) || (surface->h < 2 * radius))
        return;

    // 对四个角进行处理
    for (int corner = 0; corner < 4; corner++)
    {
        for (int i = 0; i < radius; i++)
        {
            for (int j = 0; j < radius; j++)
            {
                if ((radius - i) * (radius - i) + (radius - j) * (radius - j) <= radius * radius)
                    continue;

                *((Uint32 *)surface->pixels + (corner < 2 ? j : surface->h - j - 1) * surface->w + (corner % 2 == 0 ? i : surface->w - i - 1)) = SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a);
            }
        }
    }
}

/**
 * \brief 将用户信息渲染成Texture
 */
bool renderInfo2Surface()
{
    char text[256] = {0};
    list *tmp = NULL;

    // 设置文本颜色
    SDL_Color colorHV = {0, 255, 0, 255};
    SDL_Color colorNO = {255, 0, 0, 255};
    SDL_Color colorBK = {0, 0, 0, 0};

    // Surface
    SDL_Surface *surface1 = NULL;
    SDL_Surface *surface2 = NULL;
    SDL_Surface *surface3 = NULL;
    SDL_Surface *surface = NULL;

    // 渲染名片到tmp链表
    list *node = Global.face;
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
            strcpy(text, "no info");
            addData(&tmp, TTF_RenderUTF8_Blended(Global.font, text, colorNO), 0, false);
        }

        node = node->next;
    }

    // 将tmp链表的surface转换成texture
    node = tmp;
    while (node)
    {
        addData(&Global.faceTexture, SDL_CreateTextureFromSurface(Global.renderer, (SDL_Surface *)node->data), 0, false);
        node = node->next;
    }

    // 将tmp链表的surface放入surface ( 这一步用于保持正方向放入链表 )
    while (tmp)
    {
        void *data = NULL;
        if (getData(&tmp, &data) == false)
            break;
        addData(&Global.faceSurface, data, 0, false);
    }

    return true;
}