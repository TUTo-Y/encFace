#include "gui.h"

/**
 * \brief gui界面
 */
void play()
{
    Uint32 FRAME_DURATION = FPS_MS; // 每一帧的持续时间（毫秒）
    Uint32 frame_start, frame_time;

    SDL_Event event = {0};
    SDL_FPoint mouse = {0};

    // 渲染按钮颜色
    SDL_Color button_msg_color[] = {
        {75, 75, 75, 255},
        {50, 50, 50, 255},
        {10, 10, 10, 255}};
    SDL_Color button_color[] = {
        {200, 250, 200, 255},
        {175, 210, 175, 255},
        {165, 190, 165, 255}};
    SDL_Color *button_msg_color_p = button_msg_color;
    SDL_Color *button_color_p = button_color;

    while (1)
    {
        // 设置帧开始时间
        frame_start = SDL_GetTicks();

        // 事件处理
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT: // 退出事件
                return;
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_SIZE_CHANGED: // 窗口大小变化事件
                    // 更新窗口大小
                    Global.windowRect.w = event.window.data1;
                    Global.windowRect.h = event.window.data2;

                    // 重新按钮区域
                    Global.buttonRect.x = Global.windowRect.w * 0.2f;
                    Global.buttonRect.y = Global.windowRect.h * 0.7f;
                    Global.buttonRect.w = Global.windowRect.w * 0.6f;
                    Global.buttonRect.h = Global.windowRect.h * 0.15f;
                    Global.buttonMsgWRect.w = 50.0f;
                    Global.buttonMsgWRect.h = 10.0f;
                    Global.buttonMsgWRect.x = Global.buttonRect.x + (Global.buttonRect.w - Global.buttonMsgWRect.w) / 2.0f;
                    Global.buttonMsgWRect.y = Global.buttonRect.y + (Global.buttonRect.h - Global.buttonMsgWRect.h) / 2.0f;
                    Global.buttonMsgHRect.w = 10.0f;
                    Global.buttonMsgHRect.h = 50.0f;
                    Global.buttonMsgHRect.x = Global.buttonRect.x + (Global.buttonRect.w - Global.buttonMsgHRect.w) / 2.0f;
                    Global.buttonMsgHRect.y = Global.buttonRect.y + (Global.buttonRect.h - Global.buttonMsgHRect.h) / 2.0f;
                    break;
                case SDL_WINDOWEVENT_MOVED: // 窗口位置改变事件
                    // 更新窗口位置
                    Global.windowRect.x = event.window.data1;
                    Global.windowRect.y = event.window.data2;
                    break;
                }
                break;

            case SDL_MOUSEMOTION: // 移动事件
                // 更新鼠标位置信息
                mouse.x = (float)event.motion.x;
                mouse.y = (float)event.motion.y;

                // 如果按钮保持按下状态
                if (CHECK_FLAG(button_enter))
                {
                    // 设置按钮颜色
                    button_color_p = button_color + 2;
                    button_msg_color_p = button_msg_color + 2;
                }

                // 如果图片被选择并且被按下
                else if (CHECK_FLAG(image_is_choice) && CHECK_FLAG(image_enter))
                {
                    // 移动图片
                    Global.surfaceRect.x += event.motion.xrel;
                    Global.surfaceRect.y += event.motion.yrel;
                }

                // 如果是在按钮区域内移动
                else if (SDL_PointInFRect(&mouse, &Global.buttonRect) == SDL_TRUE && !CHECK_FLAG(image_enter))
                {
                    // 设置按钮颜色
                    button_color_p = button_color + 1;
                    button_msg_color_p = button_msg_color + 1;
                }

                // 如果是在按钮区域外移动
                else
                {
                    // 清空按钮颜色
                    button_color_p = button_color;
                    button_msg_color_p = button_msg_color;
                }
                break;

            case SDL_MOUSEBUTTONDOWN: // 按下事件

                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    // 如果是在按钮区域内按下
                    if (SDL_PointInFRect(&mouse, &Global.buttonRect) == SDL_TRUE)
                    {
                        // 设置按钮按下标志
                        SET_FLAG(button_enter);

                        // 设置按钮颜色
                        button_color_p = button_color + 2;
                        button_msg_color_p = button_msg_color + 2;
                    }
                    else // 在图片上按下
                        SET_FLAG(image_enter);
                }
                break;

            case SDL_MOUSEBUTTONUP: // 弹起事件
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    // 是否按下按钮
                    if (CHECK_FLAG(button_enter))
                    {
                        // 如果是在按钮区域内弹起
                        if (SDL_PointInFRect(&mouse, &Global.buttonRect) == SDL_TRUE)
                        {
                            // 加载人脸信息
                            loadFace();

                            // 设置按钮颜色
                            button_color_p = button_color + 1;
                            button_msg_color_p = button_msg_color + 1;
                        }
                        // 如果是在按钮区域外弹起
                        else
                        {
                            // 清空按钮颜色
                            button_color_p = button_color;
                            button_msg_color_p = button_msg_color;
                        }

                        // 清空按钮按下标志
                        CLEAR_FLAG(button_enter);
                    }
                    else // 在图片上弹起
                        CLEAR_FLAG(image_enter);
                }
                break;
            case SDL_MOUSEWHEEL: // 滚轮事件
                // 图片被选中并且鼠标在图片区域内并且鼠标不在按钮区域内
                if (CHECK_FLAG(image_is_choice) && SDL_PointInFRect(&mouse, &Global.surfaceRect) == SDL_TRUE && SDL_PointInFRect(&mouse, &Global.buttonRect) != SDL_TRUE)
                {
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
                }
                break;
            }
        }

        // 清空Renderer
        SDL_SetRenderDrawColor(Global.renderer, 255, 255, 255, 255);
        SDL_RenderClear(Global.renderer);

        // 绘制照片
        if (CHECK_FLAG(image_is_choice))
            SDL_RenderCopyF(Global.renderer, Global.texture, NULL, &Global.surfaceRect);

        // 渲染选择框
        if (CHECK_FLAG(image_is_choice))
        {
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
                else
                    SDL_SetRenderDrawColor(Global.renderer, 255, 0, 0, 255);

                // 渲染选择框
                SDL_RenderDrawRectF(Global.renderer, &rect);

                // 如果鼠标位置在选择框内，则渲染文本框
                if (SDL_PointInFRect(&mouse, &rect) && !SDL_PointInFRect(&mouse, &Global.buttonRect))
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
        }

        // 绘图按钮
        SDL_SetRenderDrawColor(Global.renderer, button_color_p->r, button_color_p->g, button_color_p->b, button_color_p->a);
        SDL_RenderFillRectF(Global.renderer, &Global.buttonRect);
        SDL_SetRenderDrawColor(Global.renderer, button_msg_color_p->r, button_msg_color_p->g, button_msg_color_p->b, button_msg_color_p->a);
        SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgWRect);
        SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgHRect);

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
 * \brief 将用户信息渲染成名片
 */
bool renderInfo()
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