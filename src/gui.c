#include "gui.h"

/**
 * \brief gui界面
 */
void play()
{
    Uint32 FRAME_DURATION = FPS_MS; // 每一帧的持续时间（毫秒）
    SDL_Event event = {0};
    int mflag = 0; // 鼠标是否按下(1按下)
    int mouseX = 0, mouseY = 0;
    SDL_FPoint mouse = {0};
    Uint32 frame_start, frame_time;

    while (1)
    {
        // 设置帧开始时间
        frame_start = SDL_GetTicks();

        // 事件处理
        while (SDL_PollEvent(&event))
        {
            FRAME_DURATION = 0; // 设置间隔时间0，及时刷新

            switch (event.type)
            {
            case SDL_QUIT: // 退出事件
                return;
                break;

            case SDL_MOUSEMOTION: // 移动事件
                // 更新鼠标位置信息
                mouseX = event.motion.x;
                mouseY = event.motion.y;
                mouse.x = (float)mouseX;
                mouse.y = (float)mouseY;

                // 鼠标按下状态下需要移动图片
                if (mflag)
                {
                    // 移动图片
                    Global.surfaceRect.x += event.motion.xrel;
                    Global.surfaceRect.y += event.motion.yrel;
                }
                break;

            case SDL_MOUSEBUTTONDOWN: // 按下事件
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mflag = 1;
                }
                break;

            case SDL_MOUSEBUTTONUP: // 弹起事件
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mflag = 0;
                }
                break;

            case SDL_MOUSEWHEEL:                                                   // 滚轮事件
                float scale = 0.15f * event.wheel.y;                               // 缩放比例
                if (scale + Global.scale2 > 0.1f && scale + Global.scale2 < 10.0f) // 缩放比例限制
                {
                    float w = Global.surfaceRect.w;
                    float h = Global.surfaceRect.h;

                    // 设置缩放比例
                    Global.scale2 += scale;

                    // 重新设置图片区域
                    Global.surfaceRect.w = Global.surface->w * Global.scale * Global.scale2;
                    Global.surfaceRect.h = Global.surface->h * Global.scale * Global.scale2;

                    // 从鼠标位置向周围缩放
                    Global.surfaceRect.x -= (((float)mouse.x - Global.surfaceRect.x) / w) * (Global.surfaceRect.w - w);
                    Global.surfaceRect.y -= (((float)mouse.y - Global.surfaceRect.y) / h) * (Global.surfaceRect.h - h);
                }
                break;

            default:
                FRAME_DURATION = FPS_MS;
                break;
            }
        }

        // 清空Renderer
        SDL_SetRenderDrawColor(Global.renderer, 255, 255, 255, 255);
        SDL_RenderClear(Global.renderer);

        // 渲染图片
        SDL_RenderCopyF(Global.renderer, Global.texture, NULL, &Global.surfaceRect);

        // 渲染选择框
        list *node = Global.face;
        list *nodeT = Global.faceTexture;
        list *nodeS = Global.faceSurface;
        while (node)
        {
            // 选择框的实际位置
            SDL_FRect rect = {
                .x = Global.surfaceRect.x + ((vector *)node->data)->rect.x * Global.scale * Global.scale2,
                .y = Global.surfaceRect.y + ((vector *)node->data)->rect.y * Global.scale * Global.scale2,
                .w = ((vector *)node->data)->rect.w * Global.scale * Global.scale2,
                .h = ((vector *)node->data)->rect.h * Global.scale * Global.scale2};

            // 根据人物是否存在设置对应的颜色
            if (((vector *)node->data)->flag == HV)
                SDL_SetRenderDrawColor(Global.renderer, 0, 255, 0, 255);
            else if (((vector *)node->data)->flag == NO || ((vector *)node->data)->flag == NU)
                SDL_SetRenderDrawColor(Global.renderer, 255, 0, 0, 255);

            // 渲染选择框
            SDL_RenderDrawRectF(Global.renderer, &rect);

            // 如果鼠标位置在选择框内，则渲染文本框
            if (SDL_PointInFRect(&mouse, &rect))
            {
                // 设置文本框的位置
                rect.x += rect.w + 5;
                rect.w *= 1.5f;
                if (((vector *)node->data)->flag == HV)
                    rect.w *= 1.5f;
                else
                    rect.y += rect.h / 3.0f;
                rect.h = (rect.w / (float)(((SDL_Surface *)nodeS->data)->w)) * (float)((SDL_Surface *)nodeS->data)->h;

                // 渲染
                SDL_RenderCopyF(Global.renderer, (SDL_Texture *)nodeT->data, NULL, &rect);
            }

            // 下一个
            node = node->next;
            nodeT = nodeT->next;
            nodeS = nodeS->next;
        }

        // 刷新Renderer
        SDL_RenderPresent(Global.renderer);

        // 控制刷新率
        frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < FRAME_DURATION)
            SDL_Delay(FRAME_DURATION - frame_time);
    }
}
/**
 * \brief 将图片缩放到指定区间
 * \param total 区间
 * \param dRect 目标区间
 * \param w 图片宽度
 * \param h 图片高度
 */
void resize(const SDL_Rect *total, SDL_FRect *dRect, int w, int h)
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
 * \brief 将用户信息渲染成名片
 */
bool render_info()
{
    char text[256] = {0};
    list *tmp = NULL;

    // 加载字体
    TTF_Font *font = TTF_OpenFont(TTF_PATH, 24);
    if (font == NULL)
    {
        ERR("TTF_OpenFont: %s\n", TTF_GetError());
        return false;
    }

    // 设置文本颜色
    SDL_Color colorHV = {0, 255, 0, 255};
    SDL_Color colorNO = {255, 0, 0, 255};
    SDL_Color colorBK = {0, 0, 0, 0};

    // 渲染名片到tmp链表
    list *node = Global.face;
    while (node)
    {
        if (((vector *)node->data)->flag == HV) // 人物存在
        {
            // 渲染人物姓名
            snprintf(text, 256, "姓名:%s", ((vector *)node->data)->info.name);
            SDL_Surface *surface1 = TTF_RenderUTF8_Blended(font, text, colorHV);

            // 渲染人物年龄
            snprintf(text, 256, "年龄:%d", ((vector *)node->data)->info.age);
            SDL_Surface *surface2 = TTF_RenderUTF8_Blended(font, text, colorHV);

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
            SDL_Surface *surface3 = TTF_RenderUTF8_Blended(font, text, colorHV);

            // 创建人物信息框
            SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, MAX_OF_THREE(surface1->w, surface2->w, surface3->w), surface1->h + surface2->h + surface3->h + 20, 32, SDL_PIXELFORMAT_RGBA32);

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
            addData(&tmp, TTF_RenderUTF8_Blended(font, text, colorNO), 0, false);
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
        if (getData(&tmp, &data, 0) == false)
            break;
        addData(&Global.faceSurface, data, 0, false);
    }

    return true;
}