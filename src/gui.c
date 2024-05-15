#include "gui.h"

/**
 * \brief gui界面
 */
void play()
{
    Uint64 FRAME_DURATION = FPS_MS; // 每一帧的持续时间（毫秒）
    Uint64 frame_start, frame_time;

    SDL_Event event = {0};
    SDL_FPoint mouse = {0};

    // 渲染按钮颜色
    SDL_Color button_msg_color[] = {
        {75, 75, 75, 255},
        {50, 50, 50, 255},
        {10, 10, 10, 255}};
    SDL_Color button_color[] = {
        {200, 250, 200, 255},
        {210, 255, 210, 255},
        {200, 245, 210, 255}};
    SDL_Color watt_color[] = {
        {200, 250, 200, 255 / 4.5f},
        {200, 250, 200, 255 / 4.0f},
        {200, 250, 200, 255 / 3.5f},
        {200, 250, 200, 255 / 3.0f},
        {200, 250, 200, 255 / 2.5f},
        {200, 250, 200, 255 / 2.0f},
        {200, 250, 200, 255 / 1.5f},
        {200, 250, 200, 255 / 1.0f},
    };

    // 动画计时器
    Uint64 timer1 = 0;
    Uint64 timer2 = 0;

    while (1)
    {
        // 设置帧开始时间
        frame_start = SDL_GetTicks64();

        // 事件处理
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT: // 退出事件
                return;
                break;
            case SDL_WINDOWEVENT: // 窗口事件
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_SIZE_CHANGED: // 窗口大小变化事件
                    // 更新窗口大小
                    Global.windowRect.w = event.window.data1;
                    Global.windowRect.h = event.window.data2;

                    // 重新按钮区域
                    resetButton();
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

                // 当前为按下状态
                if (event.motion.state == SDL_BUTTON_LMASK)
                {
                    // 按下图片移动
                    if (CHECK_FLAG(image_enter))
                    {
                        // 移动图片
                        Global.surfaceRect.x += event.motion.xrel;
                        Global.surfaceRect.y += event.motion.yrel;
                    }
                    // 按下按钮移动
                    else if (CHECK_FLAG(button_enter))
                    {
                    }

                    // 设置按钮状态信息
                    if (checkButton(mouse, Global.buttonPoint.x, Global.buttonPoint.y, Global.buttonRadius) == true)
                        SET_FLAG(button_In);
                    else
                        CLEAR_FLAG(button_In);
                }
                // 当前未按下
                else
                {
                    // 在按钮区域内
                    if (checkButton(mouse, Global.buttonPoint.x, Global.buttonPoint.y, Global.buttonRadius) == true)
                    {
                        // 刚刚进入
                        if (!CHECK_FLAG(button_In))
                        {
                            // 设置按钮选中动画
                            if (!CHECK_FLAG(button_Wait) && !CHECK_FLAG(button_Not_Wait)) // 等待动画优先级最高
                                SET_FLAG(button_Select);

                            // 设置按钮状态标志
                            SET_FLAG(button_In);
                        }
                    }
                    // 不在按钮内
                    else
                    {
                        // 刚刚离开
                        if (CHECK_FLAG(button_In))
                        {
                            // 设置按钮离开动画
                            if (!CHECK_FLAG(button_Wait) && !CHECK_FLAG(button_Not_Wait)) // 等待动画优先级最高
                                SET_FLAG(button_Not_Select);

                            // 清除按钮状态标志
                            CLEAR_FLAG(button_In);
                        }
                    }
                }

                break;

            case SDL_MOUSEBUTTONDOWN: // 按下事件
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    // 在按钮上按下，并且未触发等待动画
                    if (CHECK_FLAG(button_In) && !CHECK_FLAG(button_Wait) && !CHECK_FLAG(button_Not_Wait))
                    {
                        SET_FLAG(button_Click);

                        // 设置按钮状态标志
                        SET_FLAG(button_enter);
                    }
                    // 在图片上按下
                    else if (SDL_PointInFRect(&mouse, &Global.surfaceRect) == SDL_TRUE)
                    {
                        SET_FLAG(image_enter);
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP: // 弹起事件
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    // 按钮状态下的弹起
                    if (CHECK_FLAG(button_enter))
                    {
                        // 在按钮上弹起
                        if (CHECK_FLAG(button_In))
                        {
                            // 处理数据
                            // 如果上一个线程结束并且成功读取图片
                            if (getTh() == false && choiceImage() == true)
                            {
                                // 触发等待动画
                                SET_FLAG(button_Wait);
                            }
                            else
                            {
                                // 未选择动画
                                SET_FLAG(button_Not_Click);
                            }
                        }
                        else
                        {
                            // 更新按钮大小
                            Global.buttonRadius = getButtonRect();
                            setCross();
                        }

                        // 设置按钮状态标志
                        CLEAR_FLAG(button_enter);
                    }
                    // 图片状态下的弹起
                    else if (CHECK_FLAG(image_enter))
                    {
                        CLEAR_FLAG(image_enter);
                    }
                }
                break;
            case SDL_MOUSEWHEEL: // 滚轮事件
                // 图片被选中并且鼠标在图片区域内并且鼠标不在按钮区域内
                if (CHECK_FLAG(image_is_choice) && !CHECK_FLAG(button_In))
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
        SDL_SetRenderDrawColor(Global.renderer, 0xff, 0xff, 0xff, 255);
        SDL_RenderClear(Global.renderer);

        // 绘制照片
        if (CHECK_FLAG(image_is_choice))
            SDL_RenderCopyF(Global.renderer, Global.texture, NULL, &Global.surfaceRect);

        // 渲染选择框
        if (CHECK_FLAG(image_is_choice) && getTh() == false && !CHECK_FLAG(button_Wait) && !CHECK_FLAG(button_Not_Wait))
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
        }

        // 检测是否为按钮选中动画
        if (CHECK_FLAG(button_Select))
        {
            //  绘制初始化按钮选中动画
            if (!CHECK_FLAG(button_Select_Start))
            {
                // 更新时间
                timer1 = SDL_GetTicks64();

                // 标志为已初始化
                SET_FLAG(button_Select_Start);
            }

            // 获取间隔时间
            timer2 = SDL_GetTicks64() - timer1;

            // 绘制结束
            if (timer2 > 100)
            {
                CLEAR_FLAG(button_Select);
                CLEAR_FLAG(button_Select_Start);

                // 更新按钮大小
                Global.buttonRadius = getButtonRect() * 1.2f;
                setCross();

                // 绘制结束图案
                drawCircle(Global.renderer, Global.buttonPoint.x, Global.buttonPoint.y, Global.buttonRadius, &button_color[1]);
                SDL_SetRenderDrawColor(Global.renderer, button_msg_color[1].r, button_msg_color[1].g, button_msg_color[1].b, button_msg_color[1].a);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgWRect);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgHRect);
            }
            else
            {
                // 更新按钮大小
                Global.buttonRadius = getButtonRect() * (1.0f + 0.2f * timer2 / 100.0f);
                setCross();

                drawCircle(Global.renderer, Global.buttonPoint.x, Global.buttonPoint.y, Global.buttonRadius, &button_color[1]);
                SDL_SetRenderDrawColor(Global.renderer, button_msg_color[1].r, button_msg_color[1].g, button_msg_color[1].b, button_msg_color[1].a);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgWRect);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgHRect);
            }
        }
        // 检测是否为按钮离开动画
        else if (CHECK_FLAG(button_Not_Select))
        {
            //  绘制初始化按钮离开动画
            if (!CHECK_FLAG(button_Not_Select_Start))
            {
                // 更新时间
                timer1 = SDL_GetTicks64();

                // 标志为已初始化
                SET_FLAG(button_Not_Select_Start);
            }

            // 获取间隔时间
            timer2 = SDL_GetTicks64() - timer1;

            // 绘制结束
            if (timer2 > 100)
            {
                CLEAR_FLAG(button_Not_Select);
                CLEAR_FLAG(button_Not_Select_Start);

                // 更新按钮大小
                Global.buttonRadius = getButtonRect() * 1.0f;
                setCross();

                // 绘制结束图案
                drawCircle(Global.renderer, Global.buttonPoint.x, Global.buttonPoint.y, Global.buttonRadius, &button_color[0]);
                SDL_SetRenderDrawColor(Global.renderer, button_msg_color[0].r, button_msg_color[0].g, button_msg_color[0].b, button_msg_color[0].a);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgWRect);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgHRect);
            }
            else
            {
                // 更新按钮大小
                Global.buttonRadius = getButtonRect() * (1.0f + 0.2f * (100.0f - timer2) / 100.0f);
                setCross();

                drawCircle(Global.renderer, Global.buttonPoint.x, Global.buttonPoint.y, Global.buttonRadius, &button_color[1]);
                SDL_SetRenderDrawColor(Global.renderer, button_msg_color[1].r, button_msg_color[1].g, button_msg_color[1].b, button_msg_color[1].a);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgWRect);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgHRect);
            }
        }
        // 检测是否为按钮点击动画
        else if (CHECK_FLAG(button_Click))
        {
            //  绘制初始化
            if (!CHECK_FLAG(button_Click_Start))
            {
                // 更新时间
                timer1 = SDL_GetTicks64();

                // 标志为已初始化
                SET_FLAG(button_Click_Start);
            }

            // 获取间隔时间
            timer2 = SDL_GetTicks64() - timer1;

            // 绘制结束
            if (timer2 > 100)
            {
                CLEAR_FLAG(button_Click);
                CLEAR_FLAG(button_Click_Start);

                // 绘制结束图案
                drawCircle(Global.renderer, Global.buttonPoint.x, Global.buttonPoint.y, Global.buttonRadius, &button_color[2]);
                SDL_SetRenderDrawColor(Global.renderer, button_msg_color[2].r, button_msg_color[2].g, button_msg_color[2].b, button_msg_color[2].a);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgWRect);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgHRect);
            }
            else
            {
                // 绘制动画
                drawCircle(Global.renderer, Global.buttonPoint.x, Global.buttonPoint.y, Global.buttonRadius, &button_color[1]);
                SDL_SetRenderDrawColor(Global.renderer, button_msg_color[1].r, button_msg_color[1].g, button_msg_color[1].b, button_msg_color[1].a);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgWRect);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgHRect);

                drawCircle(Global.renderer, Global.buttonPoint.x, Global.buttonPoint.y, Global.buttonRadius * timer2 / 100.0f, &button_color[2]);
                SDL_SetRenderDrawColor(Global.renderer, button_msg_color[2].r, button_msg_color[2].g, button_msg_color[2].b, button_msg_color[2].a);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgWRect);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgHRect);
            }
        }
        // 检测是否为按钮松开动画
        else if (CHECK_FLAG(button_Not_Click))
        {
            //  绘制初始化
            if (!CHECK_FLAG(button_Not_Click_Start))
            {
                // 更新时间
                timer1 = SDL_GetTicks64();

                // 标志为已初始化
                SET_FLAG(button_Not_Click_Start);
            }

            // 获取间隔时间
            timer2 = SDL_GetTicks64() - timer1;

            // 绘制结束
            if (timer2 > 100)
            {
                CLEAR_FLAG(button_Not_Click);
                CLEAR_FLAG(button_Not_Click_Start);

                // 绘制结束图案
                drawCircle(Global.renderer, Global.buttonPoint.x, Global.buttonPoint.y, Global.buttonRadius, &button_color[1]);
                SDL_SetRenderDrawColor(Global.renderer, button_msg_color[1].r, button_msg_color[1].g, button_msg_color[1].b, button_msg_color[1].a);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgWRect);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgHRect);
            }
            else
            {
                // 绘制动画
                drawCircle(Global.renderer, Global.buttonPoint.x, Global.buttonPoint.y, Global.buttonRadius, &button_color[2]);
                SDL_SetRenderDrawColor(Global.renderer, button_msg_color[2].r, button_msg_color[2].g, button_msg_color[2].b, button_msg_color[2].a);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgWRect);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgHRect);

                drawCircle(Global.renderer, Global.buttonPoint.x, Global.buttonPoint.y, Global.buttonRadius * timer2 / 100.0f, &button_color[1]);
                SDL_SetRenderDrawColor(Global.renderer, button_msg_color[1].r, button_msg_color[1].g, button_msg_color[1].b, button_msg_color[1].a);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgWRect);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgHRect);
            }
        }
        // 检测是否为按钮等待动画
        else if (CHECK_FLAG(button_Wait))
        {
            //  绘制初始化
            if (!CHECK_FLAG(button_Wait_Start))
            {
                // 更新时间
                timer1 = SDL_GetTicks64();

                // 标志为已初始化
                SET_FLAG(button_Wait_Start);
            }

            timer2 = SDL_GetTicks64() - timer1;

            // 八个小球分开
            if (timer2 < 100)
            {

                drawCircle8(Global.renderer, Global.buttonPoint.x, Global.buttonPoint.y,
                            Global.buttonRadius * 1.4f * (float)timer2 / 100.0f,
                            Global.buttonRadius * (1.0f - timer2 / 160.0f),
                            timer2 / 350.0f,
                            watt_color);
            }
            // 转圈圈
            else
            {
                // 检测到线程结束，则绘制结束动画，并渲染个人信息，因为渲染需要使用Renderer，所以个人信息需要在这里进行渲染
                if (getTh() == false)
                {
                    renderInfo2Surface();

                    CLEAR_FLAG(button_Wait);
                    CLEAR_FLAG(button_Wait_Start);

                    SET_FLAG(button_Not_Wait);
                }

                drawCircle8(Global.renderer, Global.buttonPoint.x, Global.buttonPoint.y,
                            Global.buttonRadius * 1.4f,
                            Global.buttonRadius * (1.0f - 0.625),
                            timer2 / 400.0f,
                            watt_color);
            }
        }
        // 检测是否为按钮等待结束动画
        else if (CHECK_FLAG(button_Not_Wait))
        {
            //  绘制初始化
            if (!CHECK_FLAG(button_Not_Wait_Start))
            {
                // 更新时间
                timer1 = SDL_GetTicks64();

                // 标志为已初始化
                SET_FLAG(button_Not_Wait_Start);
            }

            timer2 = SDL_GetTicks64() - timer1;

            // 绘制结束
            if (timer2 > 100)
            {
                CLEAR_FLAG(button_Not_Wait);
                CLEAR_FLAG(button_Not_Wait_Start);

                drawCircle(Global.renderer, Global.buttonPoint.x, Global.buttonPoint.y, Global.buttonRadius, &button_color[0]);
            }
            else
            {
                drawCircle8(Global.renderer, Global.buttonPoint.x, Global.buttonPoint.y,
                            Global.buttonRadius * 1.4f * (1.0f - (float)timer2 / 100.0f),
                            Global.buttonRadius * (0.375 + timer2 / 160.0f),
                            timer2 / 400.0f,
                            watt_color);
            }
        }
        // 普通情况
        else
        {
            // 按钮的按下状态
            if (CHECK_FLAG(button_enter))
            {
                drawCircle(Global.renderer, Global.buttonPoint.x, Global.buttonPoint.y, Global.buttonRadius, &button_color[2]);
                SDL_SetRenderDrawColor(Global.renderer, button_msg_color[2].r, button_msg_color[2].g, button_msg_color[2].b, button_msg_color[2].a);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgWRect);
                SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgHRect);
            }
            // 未按下状态
            else
            {
                // 是在按钮区域内并且不是图像的按下状态
                if (!CHECK_FLAG(image_enter) && CHECK_FLAG(button_In))
                {
                    drawCircle(Global.renderer, Global.buttonPoint.x, Global.buttonPoint.y, Global.buttonRadius, &button_color[1]);
                    SDL_SetRenderDrawColor(Global.renderer, button_msg_color[1].r, button_msg_color[1].g, button_msg_color[1].b, button_msg_color[1].a);
                    SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgWRect);
                    SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgHRect);
                }
                else
                {
                    drawCircle(Global.renderer, Global.buttonPoint.x, Global.buttonPoint.y, Global.buttonRadius, &button_color[0]);
                    SDL_SetRenderDrawColor(Global.renderer, button_msg_color[0].r, button_msg_color[0].g, button_msg_color[0].b, button_msg_color[0].a);
                    SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgWRect);
                    SDL_RenderFillRectF(Global.renderer, &Global.buttonMsgHRect);
                }
            }
        }

        // 刷新Renderer
        SDL_RenderPresent(Global.renderer);

        // 控制刷新率
        frame_time = SDL_GetTicks64() - frame_start;
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
 * 获取当前按钮标准大小
 */
float getButtonRect()
{
    return MIN(Global.windowRect.w, Global.windowRect.h) * 0.06;
}

/**
 * \brief 根据按钮位置设置十字
 */
void setCross()
{
    Global.buttonMsgWRect = (SDL_FRect){
        .x = Global.buttonPoint.x - Global.buttonRadius * 0.5f,
        .y = Global.buttonPoint.y - Global.buttonRadius * 0.06f,
        .w = Global.buttonRadius * 1.0f,
        .h = Global.buttonRadius * 0.12f};

    Global.buttonMsgHRect = (SDL_FRect){
        .x = Global.buttonPoint.x - Global.buttonRadius * 0.06f,
        .y = Global.buttonPoint.y - Global.buttonRadius * 0.5f,
        .w = Global.buttonRadius * 0.12f,
        .h = Global.buttonRadius * 1.0f};
}

/**
 * \brief 重置按钮位置
 */
void resetButton()
{
    Global.buttonPoint = (SDL_FPoint){
        .x = Global.windowRect.w * 0.8f,
        .y = Global.windowRect.h * 0.8f};

    Global.buttonRadius = getButtonRect();

    setCross();
}

/**
 * \brief 检测当前位置是否在按钮上
 */
bool checkButton(SDL_FPoint point, float x, float y, float r)
{
    if ((point.x - x) * (point.x - x) + (point.y - y) * (point.y - y) <= r * r)
        return true;
    return false;
}