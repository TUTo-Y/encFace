/**
 * 你怎么知道这里全是💩
 */
#include "gui_play.h"

// 渲染按钮
#define drawButton(colorCount)                                                                                                                                    \
    {                                                                                                                                                             \
        drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius, buttonColor[colorCount]);                                                                \
        SDL_SetRenderDrawColor(renderer, buttonMsgColor[colorCount].r, buttonMsgColor[colorCount].g, buttonMsgColor[colorCount].b, buttonMsgColor[colorCount].a); \
        SDL_RenderFillRectF(renderer, &buttonMsgWRect);                                                                                                           \
        SDL_RenderFillRectF(renderer, &buttonMsgHRect);                                                                                                           \
    }

/**
 * \brief 渲染用户信息
 * \param renderer 渲染器
 * \param face 用户信息
 * \param faceSurface 头像表面
 * \param faceTexture 头像纹理
 * \return 是否渲染成功
 */
bool renderInfo2Surface(SDL_Renderer *renderer, list *face, list **faceSurface, list **faceTexture);

// 加载人脸数据
bool choiceImage(
    const SDL_Rect *windowRect,
    SDL_Renderer *renderer,
    SDL_Surface **surface, SDL_Texture **texture,
    SDL_FRect *surfaceRect, float *scale, float *scale2,
    list **face, list **faceSurface, list **faceTexture);

/**
 * \brief 使用多线程加载数据
 */
void *getFace(void *arg);

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
    ebo imageEBO = {0};          // 图片渲染的顶点数据
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
    SDL_Window *window = SDL_CreateWindow("encFace", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_PLAY_DEFAULT_WIDTH, WINDOW_PLAY_DEFAULT_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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
    if (!surface)
    {
        surface = SDL_CreateRGBSurface(0, windowRect.w, windowRect.h, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    resizeImage(&windowRect, &surfaceRect, surface->w, surface->h);
    scale = surfaceRect.w / surface->w;
    scale2 = 1.0f;
    getRoundedBorder(NULL, surface->w, surface->h, &surfaceRect, 0.02f, &imageEBO);

    // 动画计时器
    Uint64 timer1 = 0;
    Uint64 timer2 = 0;

    // 初始化按钮区域
    resetButton(&buttonPoint, &buttonRadius, &buttonMsgWRect, &buttonMsgHRect, &windowRect);

    int quit = 1;
    while (quit)
    {
        SDL_Event event = {0}; // 事件
        
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

                // 是否在按钮区域内
                bool inButton = checkPointInCircle(mouse, buttonPoint.x, buttonPoint.y, buttonRadius);

                // 当前为按下状态移动
                if (event.motion.state == SDL_BUTTON_LMASK)
                {
                    // 按下图片移动
                    if (CHECK_FLAG(image_enter))
                    {
                        // 移动图片
                        surfaceRect.x += event.motion.xrel;
                        surfaceRect.y += event.motion.yrel;

                        // 重新计算图片渲染数据
                        getRoundedBorder(NULL, surface->w, surface->h, &surfaceRect, 0.02f, &imageEBO);
                    }

                    // 按下按钮移动
                    else if (CHECK_FLAG(button_enter))
                    {
                    }

                    // 按下空白处移动
                    else
                    {
                    }
                }
                // 当前未按下状态移动
                else
                {
                    // 如果不是等待状态需要处理按钮动画
                    if (!CHECK_FLAG(button_Wait))
                    {
                        // 进入按钮区域
                        if (inButton == true &&
                            !CHECK_FLAG(button_In))
                            // 设置选择动画
                            SET_FLAG(button_Select);

                        // 离开按钮区域
                        else if (inButton == false &&
                                 CHECK_FLAG(button_In))
                            // 设置未选择动画
                            SET_FLAG(button_Not_Select);
                    }
                }

                // 设置按钮标识位
                inButton == true ? SET_FLAG(button_In) : CLEAR_FLAG(button_In);
                break;

            case SDL_MOUSEBUTTONDOWN:                       // 按下事件
                if (event.button.button == SDL_BUTTON_LEFT) // 左键按下
                {
                    // 在按钮上按下
                    if (CHECK_FLAG(button_In) && !CHECK_FLAG(button_Wait))
                    {
                        // 设置按钮按下标志
                        SET_FLAG(button_enter);

                        // 设置按钮按下动画
                        SET_FLAG(button_Click);
                    }

                    // 在图片上按下
                    else if (SDL_PointInFRect(&mouse, &surfaceRect) == SDL_TRUE)
                    {
                        // 设置图片按下标识
                        SET_FLAG(image_enter);
                    }

                    // 空白处按下
                    else
                    {
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:                         // 弹起事件
                if (event.button.button == SDL_BUTTON_LEFT) // 左键弹起
                {
                    // 按钮状态下的弹起
                    if (CHECK_FLAG(button_enter))
                    {
                        // 在按钮上弹起
                        if (CHECK_FLAG(button_In))
                        {
                            // 处理数据
                            // 如果上一个线程结束并且成功读取图片
                            if (!CHECK_FLAG(button_Wait) && getThread() == false && choiceImage(&windowRect, renderer, &surface, &texture, &surfaceRect, &scale, &scale2, &face, &faceSurface, &faceTexture) == true)
                            {
                                // 触发等待动画
                                SET_FLAG(button_Wait);

                                // 更新渲染数据
                                getRoundedBorder(NULL, surface->w, surface->h, &surfaceRect, 0.02f, &imageEBO);
                            }
                            // 选择失败
                            else if (!CHECK_FLAG(button_Wait))
                            {
                                // 未选择动画
                                if (checkPointInCircle(mouse, buttonPoint.x, buttonPoint.y, buttonRadius) == false)
                                    SET_FLAG(button_Not_Click);
                            }
                        }

                        // 不在按钮上弹起
                        else
                        {
                            if (!CHECK_FLAG(button_Wait))
                                // 设置松开动画
                                SET_FLAG(button_Not_Click);
                        }

                        // 设置按钮状态标志
                        CLEAR_FLAG(button_enter);
                    }

                    // 图片状态下的弹起
                    else if (CHECK_FLAG(image_enter))
                    {
                        CLEAR_FLAG(image_enter);

                        // 如果在按钮区域，则触发按钮选择动画
                        if (!CHECK_FLAG(button_Wait) && checkPointInCircle(mouse, buttonPoint.x, buttonPoint.y, buttonRadius) == true)
                            SET_FLAG(button_Select);
                    }
                }
                break;
            case SDL_MOUSEWHEEL:                                             // 滚轮事件
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
                getRoundedBorder(NULL, surface->w, surface->h, &surfaceRect, 0.02f, &imageEBO);

                break;
            }
        }

        // 清空Renderer
        SDL_SetRenderDrawColor(renderer, backGroundColor.r, backGroundColor.g, backGroundColor.b, backGroundColor.a);
        SDL_RenderClear(renderer);

        // 绘制照片
        drawFromEBO(renderer, texture, imageEBO);

        // 渲染选择框
        if (getThread() == false && !CHECK_FLAG(button_Wait))
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

        // 按钮选中动画
        if (CHECK_FLAG(button_Select))
        {
            //  绘制初始化按钮选中动画
            if (!CHECK_FLAG(button_Select_Start))
            {
                // 标志为已初始化
                SET_FLAG(button_Select_Start);

                // 更新时间
                timer1 = SDL_GetTicks64();
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
            }
            else
            {
                // 更新按钮大小
                buttonRadius = getButtonRadius(&windowRect) * (1.0f + 0.2f * timer2 / 100.0f);
                setCross(&buttonMsgWRect, &buttonMsgHRect, &buttonPoint, buttonRadius);
            }

            // 渲染按钮
            drawButton(1);
        }
        // 按钮离开动画
        else if (CHECK_FLAG(button_Not_Select))
        {
            //  绘制初始化按钮离开动画
            if (!CHECK_FLAG(button_Not_Select_Start))
            {
                // 标志为已初始化
                SET_FLAG(button_Not_Select_Start);

                // 更新时间
                timer1 = SDL_GetTicks64();
            }

            // 获取间隔时间
            timer2 = SDL_GetTicks64() - timer1;

            // 绘制结束
            if (timer2 > 100)
            {
                CLEAR_FLAG(button_Not_Select);
                CLEAR_FLAG(button_Not_Select_Start);

                // 更新按钮大小
                buttonRadius = getButtonRadius(&windowRect);
                setCross(&buttonMsgWRect, &buttonMsgHRect, &buttonPoint, buttonRadius);

                // 渲染按钮
                drawButton(0);
            }
            else
            {
                // 更新按钮大小
                buttonRadius = getButtonRadius(&windowRect) * (1.0f + 0.2f * (100.0f - timer2) / 100.0f);
                setCross(&buttonMsgWRect, &buttonMsgHRect, &buttonPoint, buttonRadius);

                // 渲染按钮
                drawButton(1);
            }
        }
        // 按钮点击动画
        else if (CHECK_FLAG(button_Click))
        {
            //  绘制初始化
            if (!CHECK_FLAG(button_Click_Start))
            {
                // 标志为已初始化
                SET_FLAG(button_Click_Start);

                // 更新时间
                timer1 = SDL_GetTicks64();
            }

            // 获取间隔时间
            timer2 = SDL_GetTicks64() - timer1;

            // 绘制结束
            if (timer2 > 100)
            {
                CLEAR_FLAG(button_Click);
                CLEAR_FLAG(button_Click_Start);

                // 绘制结束图案
                drawButton(2);
            }
            else
            {
                // 绘制动画
                drawButton(1);

                drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius * timer2 / 100.0f, buttonColor[2]);
                SDL_SetRenderDrawColor(renderer, buttonMsgColor[2].r, buttonMsgColor[2].g, buttonMsgColor[2].b, buttonMsgColor[2].a);
                SDL_RenderFillRectF(renderer, &buttonMsgWRect);
                SDL_RenderFillRectF(renderer, &buttonMsgHRect);
            }
        }
        // 按钮松开动画
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

                // 更新按钮大小
                buttonRadius = getButtonRadius(&windowRect);
                setCross(&buttonMsgWRect, &buttonMsgHRect, &buttonPoint, buttonRadius);

                // 渲染按钮
                drawButton(0);
            }
            else
            {
                // 更新按钮大小
                buttonRadius = getButtonRadius(&windowRect) * (1.0f + 0.2f * (100.0f - timer2) / 100.0f);
                setCross(&buttonMsgWRect, &buttonMsgHRect, &buttonPoint, buttonRadius);

                // 渲染按钮
                drawButton(2);
            }
        }
        // 按钮等待动画
        else if (CHECK_FLAG(button_Wait))
        {
            // 等待状态
            if (getThread() == true)
            {
                //  绘制初始化
                if (!CHECK_FLAG(button_Wait_Start))
                {
                    // 标志为已初始化
                    SET_FLAG(button_Wait_Start);

                    // 更新时间
                    timer1 = SDL_GetTicks64();
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
                    drawCircleN(renderer, buttonPoint.x, buttonPoint.y,
                                buttonRadius * 1.4f,
                                buttonRadius * (1.0f - 0.625),
                                wattColor,
                                timer2 / 400.0f,
                                8);
                }
            }
            // 等待结束
            else
            {
                if (!CHECK_FLAG(button_Not_Wait_Start))
                {
                    // 渲染用户信息
                    renderInfo2Surface(renderer, face, &faceSurface, &faceTexture);

                    // 标志为已初始化
                    SET_FLAG(button_Not_Wait_Start);

                    // 更新时间
                    timer1 = SDL_GetTicks64();
                }

                timer2 = SDL_GetTicks64() - timer1;

                if (timer2 > 100)
                {
                    CLEAR_FLAG(button_Wait);
                    CLEAR_FLAG(button_Wait_Start);
                    CLEAR_FLAG(button_Not_Wait_Start);

                    // 重置按钮
                    resetButton(&buttonPoint, &buttonRadius, &buttonMsgWRect, &buttonMsgHRect, &windowRect);
                    if (CHECK_FLAG(button_In))
                    {
                        buttonRadius *= 1.2f;
                        drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius, buttonColor[1]);
                    }
                    else
                        drawCircle(renderer, buttonPoint.x, buttonPoint.y, buttonRadius, buttonColor[0]);
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
        }
        // 普通情况
        else
        {
            // 按钮的按下状态
            if (CHECK_FLAG(button_enter))
            {
                drawButton(2);
            }
            // 未按下状态
            else
            {
                // 是在按钮区域内并且不是图像的按下状态
                if (!CHECK_FLAG(image_enter) && CHECK_FLAG(button_In))
                {
                    drawButton(1);
                }
                else
                {
                    drawButton(0);
                }
            }
        }

        // 更新Renderer
        SDL_RenderPresent(renderer);

        // 控制刷新率
        frameTime = SDL_GetTicks64() - frameStart;
        if (frameTime < FPS_MS)
            SDL_Delay(FPS_MS - frameTime);
    }
error:

    // 释放渲染数据
    freeEBO(&imageEBO);

    // 杀死线程
    if (getThread() == true)
        pthread_kill(Global.thread, SIGTERM);

    // 释放资源
    listFree(&face, (void (*)(void *))freeVector);
    listFree(&faceSurface, (void (*)(void *))SDL_FreeSurface);
    listFree(&faceTexture, (void (*)(void *))SDL_DestroyTexture);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

bool renderInfo2Surface(SDL_Renderer *renderer, list *face, list **faceSurface, list **faceTexture)
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

    // 检查参数
    if (!renderer || !face || !faceSurface || !faceTexture)
        return false;

    // 初始化
    listFree(faceSurface, (void (*)(void *))SDL_FreeSurface);
    listFree(faceTexture, (void (*)(void *))SDL_DestroyTexture);

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

bool choiceImage(
    const SDL_Rect *windowRect,
    SDL_Renderer *renderer,
    SDL_Surface **surface, SDL_Texture **texture,
    SDL_FRect *surfaceRect, float *scale, float *scale2,
    list **face, list **faceSurface, list **faceTexture)
{
    int ret = 0;
    SDL_Surface *tmpSurface = NULL;

    // 初始化
    memset(Global.path, 0, sizeof(Global.path));

    // 选择图片
    ret = selectImageFile(Global.path, sizeof(Global.path));
    CHECK(ret == true, "未选择图片\n");

    // 清除原有数据
    SDL_FreeSurface(*surface);
    SDL_DestroyTexture(*texture);

    // 读取图片Surface
    tmpSurface = IMG_Load_RW(SDL_RWFromFile(Global.path, "rb"), 1);
    CHECK(tmpSurface, "无法读取图片文件 : %s\n", IMG_GetError());

    // 转化图片格式到四通道
    *surface = SDL_ConvertSurfaceFormat(tmpSurface, SDL_PIXELFORMAT_RGBA32, 0);
    CHECK(*surface, "转换图片格式失败 : %s\n", SDL_GetError());
    SDL_FreeSurface(tmpSurface);

    // 读取图片Texture
    *texture = SDL_CreateTextureFromSurface(renderer, *surface);
    CHECK(*texture, "创建图片纹理失败 : %s\n", SDL_GetError());

    // 调整图片大小和缩放
    resizeImage(windowRect, surfaceRect, (*surface)->w, (*surface)->h);
    *scale = surfaceRect->w / (*surface)->w;
    *scale2 = 1.0f;

    // 创建线程
    setThread(true);
    ret = pthread_create(&Global.thread, NULL, getFace, face);
    CHECK(ret == 0, "创建线程失败\n");

    return true;

error:
    if (tmpSurface)
        SDL_FreeSurface(tmpSurface);
    return false;
}

void *getFace(void *arg)
{
    bool ret = false;

    // 获取人脸特征向量
    ret = getFaceVector(Global.path, (list **)arg);
    CHECK(ret == true, "获取人脸特征向量失败\n");

    // 如果检测到人脸
    if (0 < listLen(*(list **)arg))
    {
        // 获取人脸信息
        ret = getFaceInfo(*(list **)arg);
        CHECK(ret == true, "获取人脸信息失败\n");
    }

    DEB(else { DEBUG("未检测到人脸\n"); });

error:
    setThread(false);
    return NULL;
}