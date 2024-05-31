/**
 * 你怎么知道这里全是💩
 */
#include "gui_play.h"

// 根据窗口大小获取按钮的位置
static SDL_FPoint getButtonPost(const SDL_Rect *windowRect)
{
    return (SDL_FPoint){
        .x = windowRect->w * 0.8f,
        .y = windowRect->h * 0.8f};
}

// 根据窗口大小获取按钮的半径大小
static float getButtonRadius(const SDL_Rect *windowRect)
{
    return MIN(windowRect->w, windowRect->h) * 0.06f;
}

// 根据按钮位置设置十字
static void setCross(SDL_FRect *buttonMsgWRect, SDL_FRect *buttonMsgHRect, const SDL_FPoint *buttonPoint, float buttonRadius)
{
    *buttonMsgWRect = (SDL_FRect){
        .x = buttonPoint->x - buttonRadius * 0.5f,
        .y = buttonPoint->y - buttonRadius * 0.06f,
        .w = buttonRadius * 1.0f,
        .h = buttonRadius * 0.12f};

    *buttonMsgHRect = (SDL_FRect){
        .x = buttonPoint->x - buttonRadius * 0.06f,
        .y = buttonPoint->y - buttonRadius * 0.5f,
        .w = buttonRadius * 0.12f,
        .h = buttonRadius * 1.0f};
}

// 重置按钮位置
static void resetButton(SDL_FPoint *buttonPoint, float *buttonRadius, SDL_FRect *buttonMsgWRect, SDL_FRect *buttonMsgHRect, const SDL_Rect *windowRect)
{
    // 设置按钮位置
    *buttonPoint = getButtonPost(windowRect);

    // 设置按钮半径
    *buttonRadius = getButtonRadius(windowRect);

    // 设置十字
    setCross(buttonMsgWRect, buttonMsgHRect, buttonPoint, *buttonRadius);
}

/**
 * \brief gui_play界面
 */
void gui_play()
{
    int ret = 0;

    Uint64 frameStart = 0, frameTime = 0; // 控制帧率

    SDL_FPoint mouse = {0};    // 鼠标位置
    SDL_Rect windowRect = {0}; // 窗口位置

    float buttonRadius = 0.0f;      // 按钮半径
    SDL_FPoint buttonPoint = {0};   // 按钮中心点
    SDL_FRect buttonMsgWRect = {0}; // 按钮十字区域W
    SDL_FRect buttonMsgHRect = {0}; // 按钮十字区域H

    SDL_Surface *surface = NULL; // 照片Surface
    SDL_Texture *texture = NULL; // 照片Texture
    SDL_FRect surfaceRect = {0}; // 图片在窗口的实际区域
    data *vertexImage = NULL;    // 图片渲染的顶点数据
    data *indexImage = NULL;     // 图片渲染的顶点索引
    float scale = 0.0f;          // 图片缩放比例(修正宽度比原图像宽度的比例，用于调整图像大小以适应窗口)
    float scale2 = 0.0f;         // 图片二次缩放比例(由滚轮控制)

    list *face = NULL;        // 人脸列表
    list *faceSurface = NULL; // 人脸Surface列表
    list *faceTexture = NULL; // 人脸Texture列表

    // 背景颜色莫奈的灰，温柔的灰
    SDL_Color backGroundColor = {165, 166, 177, 255};
    // 按钮内容颜色
    SDL_Color buttonMsgColor[] = {
        {75, 75, 75, 255},  // 未选中
        {50, 50, 50, 255},  // 选中时
        {10, 10, 10, 255}}; // 点击时
    // 按钮颜色
    SDL_Color buttonColor[] = {
        {200, 250, 200, 255},  // 未选中
        {210, 255, 210, 255},  // 选中时
        {200, 245, 210, 255}}; // 点击时
    // 等待时八个圈圈的颜色
    SDL_Color wattColor[] = {
        {200, 250, 200, 255 / 4.5f},
        {200, 250, 200, 255 / 4.0f},
        {200, 250, 200, 255 / 3.5f},
        {200, 250, 200, 255 / 3.0f},
        {200, 250, 200, 255 / 2.5f},
        {200, 250, 200, 255 / 2.0f},
        {200, 250, 200, 255 / 1.5f},
        {200, 250, 200, 255 / 1.0f},
    };

    // 创建窗口
    SDL_Window *window = SDL_CreateWindow("encFace", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    CHECK(window != NULL, "创建窗口失败 : %s\n", SDL_GetError());

    // 设置窗口的最小大小
    SDL_SetWindowMinimumSize(window, 400, 400);

    // 创建渲染器
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    CHECK(renderer != NULL, "创建渲染器失败 : %s\n", SDL_GetError());

    // 设置渲染器
    ret = SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    CHECK(ret == 0, "设置渲染器错误 : %s\n", SDL_GetError());

    // 初始化窗口全局数据
    SDL_GetWindowPosition(window, &windowRect.x, &windowRect.y);
    SDL_GetWindowSize(window, &windowRect.w, &windowRect.h);

    // 加载初始化图片
    surface = IMG_Load(TEAM_IMAGE);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    resizeImage(&windowRect, &surfaceRect, surface->w, surface->h);
    scale = surfaceRect.w / surface->w;
    scale2 = 1.0f;
    getRoundedBorder(NULL, surface->w, surface->h, &surfaceRect, 0.02f, &indexImage, &vertexImage);
    SET_FLAG(image_is_choice);

    // 动画计时器
    Uint64 timer1 = 0;
    Uint64 timer2 = 0;

    // 初始化按钮区域
    resetButton(&buttonPoint, &buttonRadius, &buttonMsgWRect, &buttonMsgHRect, &windowRect);

    SDL_Event event = {0}; // 事件
    int quit = 1;
    while (quit)
    {
        // 设置帧开始时间
        frameStart = SDL_GetTicks64();

        // 事件处理
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                quit = 0;
                break;

            case SDL_WINDOWEVENT: // 窗口事件
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_SIZE_CHANGED: // 窗口大小变化事件
                    // 更新窗口大小
                    windowRect.w = event.window.data1;
                    windowRect.h = event.window.data2;

                    // 重新按钮区域
                    resetButton(&buttonPoint, &buttonRadius, &buttonMsgWRect, &buttonMsgHRect, &windowRect);
                    break;
                case SDL_WINDOWEVENT_MOVED: // 窗口位置改变事件
                    // 更新窗口位置
                    windowRect.x = event.window.data1;
                    windowRect.y = event.window.data2;
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
                        surfaceRect.x += event.motion.xrel;
                        surfaceRect.y += event.motion.yrel;

                        getRoundedBorder(NULL, surface->w, surface->h, &surfaceRect, 0.02f, &indexImage, &vertexImage);
                    }
                    // 按下按钮移动
                    else if (CHECK_FLAG(button_enter))
                    {
                    }

                    // 设置按钮状态信息
                    if (checkPointInCircle(mouse, buttonPoint.x, buttonPoint.y, buttonRadius) == true)
                        SET_FLAG(button_In);
                    else
                        CLEAR_FLAG(button_In);
                }
                // 当前未按下
                else
                {
                    // 在按钮区域内
                    if (checkPointInCircle(mouse, buttonPoint.x, buttonPoint.y, buttonRadius) == true)
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
                    else if (SDL_PointInFRect(&mouse, &surfaceRect) == SDL_TRUE)
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
                            DEBUG("aaaa\n");
                            // 处理数据
                            // 如果上一个线程结束并且成功读取图片
                            if (getThread() == false && choiceImage(&windowRect, renderer, &surface, &texture, &surfaceRect, &scale, &scale2, &face, &faceSurface, &faceTexture) == true)
                            {
                                DEBUG("bbbb\n");
                                // 更新渲染数据
                                getRoundedBorder(NULL, surface->w, surface->h, &surfaceRect, 0.02f, &indexImage, &vertexImage);
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
                            buttonRadius = getButtonRadius(&windowRect);
                            resetButton(&buttonPoint, &buttonRadius, &buttonMsgWRect, &buttonMsgHRect, &windowRect);
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
                    float scale_tmp = 0.20f * event.wheel.y;                     // 缩放比例
                    if (scale_tmp + scale2 > 0.1f && scale_tmp + scale2 < 15.0f) // 缩放比例限制
                    {
                        float w = surfaceRect.w;
                        float h = surfaceRect.h;

                        // 设置缩放比例
                        scale2 += scale_tmp;

                        // 重新设置图片区域
                        surfaceRect.w = surface->w * scale * scale2;
                        surfaceRect.h = surface->h * scale * scale2;

                        // 从鼠标位置向周围缩放
                        surfaceRect.x -= (((float)mouse.x - surfaceRect.x) / w) * (surfaceRect.w - w);
                        surfaceRect.y -= (((float)mouse.y - surfaceRect.y) / h) * (surfaceRect.h - h);
                    }
                    // 更新渲染数据
                    getRoundedBorder(NULL, surface->w, surface->h, &surfaceRect, 0.02f, &indexImage, &vertexImage);
                }
                break;
            }
        }

        // 清空Renderer
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 255);
        SDL_RenderClear(renderer);

        // 绘制照片
        if (CHECK_FLAG(image_is_choice))
        {
            // SDL_RenderCopyF(renderer, texture, NULL, &surfaceRect);
            // drawRoundedBorderTextureF(renderer, texture, &surfaceRect, 0.02f);

            SDL_RenderGeometry(renderer, texture, (SDL_Vertex *)vertexImage->data, vertexImage->size / sizeof(SDL_Vertex), (int *)indexImage->data, indexImage->size / sizeof(int));
        }

        // 渲染选择框
        if (CHECK_FLAG(image_is_choice) && getThread() == false && !CHECK_FLAG(button_Wait) && !CHECK_FLAG(button_Not_Wait))
        {
            list *node = face;
            list *nodeT = faceTexture;
            list *nodeS = faceSurface;
            while (node)
            {
                // 选择框的实际位置
                SDL_FRect rect = {
                    .x = surfaceRect.x + ((vector *)node->data)->rect.x * scale * scale2,
                    .y = surfaceRect.y + ((vector *)node->data)->rect.y * scale * scale2,
                    .w = ((vector *)node->data)->rect.w * scale * scale2,
                    .h = ((vector *)node->data)->rect.h * scale * scale2};

                // 根据人物是否存在设置对应的颜色
                if (((vector *)node->data)->flag == HV)
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                else
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

                // 渲染选择框
                SDL_RenderDrawRectF(renderer, &rect);

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
                    SDL_RenderCopyF(renderer, (SDL_Texture *)nodeT->data, NULL, &rect);
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
                buttonRadius = getButtonRadius(&windowRect) * 1.2f;
                setCross(&buttonMsgWRect, &buttonMsgHRect, &buttonPoint, buttonRadius);

                // 绘制结束图案
                drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius, &buttonColor[1]);
                SDL_SetRenderDrawColor(renderer, buttonMsgColor[1].r, buttonMsgColor[1].g, buttonMsgColor[1].b, buttonMsgColor[1].a);
                SDL_RenderFillRectF(renderer, &buttonMsgWRect);
                SDL_RenderFillRectF(renderer, &buttonMsgHRect);
            }
            else
            {
                // 更新按钮大小
                buttonRadius = getButtonRadius(&windowRect) * (1.0f + 0.2f * timer2 / 100.0f);
                setCross(&buttonMsgWRect, &buttonMsgHRect, &buttonPoint, buttonRadius);

                drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius, &buttonColor[1]);
                SDL_SetRenderDrawColor(renderer, buttonMsgColor[1].r, buttonMsgColor[1].g, buttonMsgColor[1].b, buttonMsgColor[1].a);
                SDL_RenderFillRectF(renderer, &buttonMsgWRect);
                SDL_RenderFillRectF(renderer, &buttonMsgHRect);
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
                buttonRadius = getButtonRadius(&windowRect) * 1.0f;
                setCross(&buttonMsgWRect, &buttonMsgHRect, &buttonPoint, buttonRadius);

                // 绘制结束图案
                drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius, &buttonColor[0]);
                SDL_SetRenderDrawColor(renderer, buttonMsgColor[0].r, buttonMsgColor[0].g, buttonMsgColor[0].b, buttonMsgColor[0].a);
                SDL_RenderFillRectF(renderer, &buttonMsgWRect);
                SDL_RenderFillRectF(renderer, &buttonMsgHRect);
            }
            else
            {
                // 更新按钮大小
                buttonRadius = getButtonRadius(&windowRect) * (1.0f + 0.2f * (100.0f - timer2) / 100.0f);
                setCross(&buttonMsgWRect, &buttonMsgHRect, &buttonPoint, buttonRadius);

                drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius, &buttonColor[1]);
                SDL_SetRenderDrawColor(renderer, buttonMsgColor[1].r, buttonMsgColor[1].g, buttonMsgColor[1].b, buttonMsgColor[1].a);
                SDL_RenderFillRectF(renderer, &buttonMsgWRect);
                SDL_RenderFillRectF(renderer, &buttonMsgHRect);
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
                drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius, &buttonColor[2]);
                SDL_SetRenderDrawColor(renderer, buttonMsgColor[2].r, buttonMsgColor[2].g, buttonMsgColor[2].b, buttonMsgColor[2].a);
                SDL_RenderFillRectF(renderer, &buttonMsgWRect);
                SDL_RenderFillRectF(renderer, &buttonMsgHRect);
            }
            else
            {
                // 绘制动画
                drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius, &buttonColor[1]);
                SDL_SetRenderDrawColor(renderer, buttonMsgColor[1].r, buttonMsgColor[1].g, buttonMsgColor[1].b, buttonMsgColor[1].a);
                SDL_RenderFillRectF(renderer, &buttonMsgWRect);
                SDL_RenderFillRectF(renderer, &buttonMsgHRect);

                drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius * timer2 / 100.0f, &buttonColor[2]);
                SDL_SetRenderDrawColor(renderer, buttonMsgColor[2].r, buttonMsgColor[2].g, buttonMsgColor[2].b, buttonMsgColor[2].a);
                SDL_RenderFillRectF(renderer, &buttonMsgWRect);
                SDL_RenderFillRectF(renderer, &buttonMsgHRect);
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
                drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius, &buttonColor[1]);
                SDL_SetRenderDrawColor(renderer, buttonMsgColor[1].r, buttonMsgColor[1].g, buttonMsgColor[1].b, buttonMsgColor[1].a);
                SDL_RenderFillRectF(renderer, &buttonMsgWRect);
                SDL_RenderFillRectF(renderer, &buttonMsgHRect);
            }
            else
            {
                // 绘制动画
                drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius, &buttonColor[2]);
                SDL_SetRenderDrawColor(renderer, buttonMsgColor[2].r, buttonMsgColor[2].g, buttonMsgColor[2].b, buttonMsgColor[2].a);
                SDL_RenderFillRectF(renderer, &buttonMsgWRect);
                SDL_RenderFillRectF(renderer, &buttonMsgHRect);

                drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius * timer2 / 100.0f, &buttonColor[1]);
                SDL_SetRenderDrawColor(renderer, buttonMsgColor[1].r, buttonMsgColor[1].g, buttonMsgColor[1].b, buttonMsgColor[1].a);
                SDL_RenderFillRectF(renderer, &buttonMsgWRect);
                SDL_RenderFillRectF(renderer, &buttonMsgHRect);
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

                drawCircleN(renderer, buttonPoint.x, buttonPoint.y,
                            buttonRadius * 1.4f * (float)timer2 / 100.0f,
                            buttonRadius * (1.0f - timer2 / 160.0f),
                            wattColor,
                            timer2 / 350.0f,
                            8);
            }
            // 转圈圈
            else
            {
                // 检测到线程结束，则绘制结束动画，并渲染个人信息，因为渲染需要使用Renderer，所以个人信息需要在这里进行渲染
                if (getThread() == false)
                {
                    renderInfo2Surface(renderer, face, &faceSurface, &faceTexture);

                    CLEAR_FLAG(button_Wait);
                    CLEAR_FLAG(button_Wait_Start);

                    SET_FLAG(button_Not_Wait);
                }

                drawCircleN(renderer, buttonPoint.x, buttonPoint.y,
                            buttonRadius * 1.4f,
                            buttonRadius * (1.0f - 0.625),
                            wattColor,
                            timer2 / 400.0f,
                            8);
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

                // 更新按钮大小
                buttonRadius = getButtonRadius(&windowRect);
                if (CHECK_FLAG(button_In))
                    buttonRadius *= 1.2f;
                drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius, &buttonColor[0]);
            }
            else
            {
                drawCircleN(renderer, buttonPoint.x, buttonPoint.y,
                            buttonRadius * 1.4f * (1.0f - (float)timer2 / 100.0f),
                            buttonRadius * (0.375 + timer2 / 160.0f),
                            wattColor,
                            timer2 / 400.0f,
                            8);
            }
        }
        // 普通情况
        else
        {
            // 按钮的按下状态
            if (CHECK_FLAG(button_enter))
            {
                drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius, &buttonColor[2]);
                SDL_SetRenderDrawColor(renderer, buttonMsgColor[2].r, buttonMsgColor[2].g, buttonMsgColor[2].b, buttonMsgColor[2].a);
                SDL_RenderFillRectF(renderer, &buttonMsgWRect);
                SDL_RenderFillRectF(renderer, &buttonMsgHRect);
            }
            // 未按下状态
            else
            {
                // 是在按钮区域内并且不是图像的按下状态
                if (!CHECK_FLAG(image_enter) && CHECK_FLAG(button_In))
                {
                    drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius, &buttonColor[1]);
                    SDL_SetRenderDrawColor(renderer, buttonMsgColor[1].r, buttonMsgColor[1].g, buttonMsgColor[1].b, buttonMsgColor[1].a);
                    SDL_RenderFillRectF(renderer, &buttonMsgWRect);
                    SDL_RenderFillRectF(renderer, &buttonMsgHRect);
                }
                else
                {
                    drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius, &buttonColor[0]);
                    SDL_SetRenderDrawColor(renderer, buttonMsgColor[0].r, buttonMsgColor[0].g, buttonMsgColor[0].b, buttonMsgColor[0].a);
                    SDL_RenderFillRectF(renderer, &buttonMsgWRect);
                    SDL_RenderFillRectF(renderer, &buttonMsgHRect);
                }
            }
        }

        // 刷新Renderer
        SDL_RenderPresent(renderer);

        // 控制刷新率
        frameTime = SDL_GetTicks64() - frameStart;
        if (frameTime < FPS_MS)
            SDL_Delay(FPS_MS - frameTime);
    }
error:

    Free(vertexImage);
    Free(indexImage);

    // 杀死线程
    if (getThread() == true)
        pthread_kill(Global.thread, SIGTERM);

    // 释放资源
    if (CHECK_FLAG(image_is_choice))
    {
        listFree(&face, (void (*)(void *))freeVector);
        listFree(&faceSurface, (void (*)(void *))SDL_FreeSurface);
        listFree(&faceTexture, (void (*)(void *))SDL_DestroyTexture);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}
