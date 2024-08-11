#include "gui_play.h"

static const SDL_Color figureLiveColorFD = {0, 255, 0, 255};    // 人物存在的字体颜色
static const SDL_Color figureLiveColorBG = {128, 128, 255, 64}; // 人物存在的背景颜色
static const SDL_Color figureDeadColorFD = {255, 0, 0, 255};    // 人物不存在的字体颜色
static const SDL_Color figureDeadColorBG = {128, 0, 0, 64};     // 人物不存在的背景颜色

#define GUI_PLAY_BUTTON_RUN_SELECT_TIME 100 // 按钮选中动画时间
#define GUI_PLAY_BUTTON_RUN_LEAVE_TIME 100  // 按钮离开动画时间
#define GUI_PLAY_BUTTON_RUN_PRESS_TIME 150  // 按钮按下动画时间
#define GUI_PLAY_BUTTON_WATT_1_TIME 200     // 等待动画1时间
#define GUI_PLAY_BUTTON_WATT_2_TIME 200     // 等待动画2时间
#define GUI_PLAY_BUTTON_SIZE \
    (MIN(WINDOW_PLAY_DEFAULT_WIDTH, WINDOW_PLAY_DEFAULT_HEIGHT) * 0.06f) // 按钮大小

void getFaceThread(faceThreadParam *param);

/** \brief 选择图片文件 */
static void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer data)
{
    if (response_id == GTK_RESPONSE_ACCEPT)
    {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        g_autofree char *filename = gtk_file_chooser_get_filename(chooser);
        g_strlcpy((gchar *)data, filename, PATH_MAX);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}
bool selectImageFile(char *path, size_t size)
{
    if (path == NULL || size == 0)
    {
        return false;
    }
    *path = '\0';

    GtkWidget *dialog = gtk_file_chooser_dialog_new("选择图片文件",
                                                    NULL,
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "取消",
                                                    GTK_RESPONSE_CANCEL,
                                                    "选择",
                                                    GTK_RESPONSE_ACCEPT,
                                                    NULL);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "图片文件");

    const char *patterns[] = {
        "*.[Bb][Mm][Pp]", "*.[Gg][Ii][Ff]", "*.[Jj][Pp][Gg]", "*.[Jj][Pp][Ee][Gg]",
        "*.[Ll][Bb][Mm]", "*.[Pp][Cc][Xx]", "*.[Pp][Nn][Gg]", "*.[Pp][Nn][Mm]",
        "*.[Ss][Vv][Gg]", "*.[Tt][Gg][Aa]", "*.[Tt][Ii][Ff][Ff]", "*.[Ww][Ee][Bb][Pp]",
        "*.[Xx][Cc][Ff]", "*.[Xx][Pp][Mm]", "*.[Xx][Vv]"};

    for (size_t i = 0; i < G_N_ELEMENTS(patterns); ++i)
    {
        gtk_file_filter_add_pattern(filter, patterns[i]);
    }

    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    g_signal_connect(dialog, "response", G_CALLBACK(on_dialog_response), path);
    g_signal_connect_swapped(dialog, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show(dialog);
    gtk_main();

    return *path != '\0';
}

/** \brief 绘制按钮 */
static void gpRenderButtonDraw(guiPlay *gp, SDL_Texture *texture, float r)
{
    if (texture == NULL)
        texture = *gp->buttonTexture;
    if (r < 0)
        r = gp->buttonRadius;
    // 绘制按钮
    SDL_RenderCopyF(gp->renderer, texture, NULL, &(SDL_FRect){.x = gp->buttonCenter.x - r, .y = gp->buttonCenter.y - r, .w = r * 2, .h = r * 2});

    // 绘制按钮十字
    SDL_SetRenderDrawColor(gp->renderer, 0, 0, 0, 255);
    SDL_RenderFillRectF(gp->renderer, &(SDL_FRect){
                                          .x = gp->buttonCenter.x - r * 0.5f,
                                          .y = gp->buttonCenter.y - r * 0.06f,
                                          .w = r,
                                          .h = r * 0.12f});
    SDL_RenderFillRectF(gp->renderer, &(SDL_FRect){
                                          .x = gp->buttonCenter.x - r * 0.06f,
                                          .y = gp->buttonCenter.y - r * 0.5f,
                                          .w = r * 0.12f,
                                          .h = r});
}

/** \brief 初始化界面Play一些数据 */
static void gpInit(guiPlay *gm, list *figure, guiMsg *msg, SDL_Renderer *renderer)
{
    // 清空界面1数据
    memset(gm, 0, sizeof(guiPlay));

    // 设置按钮大小, 位置及纹理
    gm->buttonRadius = GUI_PLAY_BUTTON_SIZE;
    gm->buttonCenter = (SDL_FPoint){
        .x = WINDOW_PLAY_DEFAULT_WIDTH * 0.8f,
        .y = WINDOW_PLAY_DEFAULT_HEIGHT * 0.8f};
    gm->buttonTextureS = drawCircle(gm->buttonRadius * 2, (SDL_Color){200, 250, 200, 255}, renderer, true);
    gm->buttonTextureP = drawCircle(gm->buttonRadius * 2, (SDL_Color){200, 240, 210, 255}, renderer, true);
    gm->buttonTexture = &gm->buttonTextureS;

    // 设置图片数据
    gm->scale1 = gm->scale2 = 1.0f;

    // 设置动画时间
    gm->time = 0;

    // 设置人物信息
    gm->figure = figure;

    // 设置消息队列
    gm->msg = msg;

    // 设置渲染器
    gm->renderer = renderer;
}

/** \brief 处理按钮事件 */
static int gpEventButton(guiPlay *gp, SDL_Event *event)
{
    // 未按下
    int ret = 0;

    switch (event->type)
    {
    case SDL_MOUSEMOTION:                            // 移动事件
        if (event->motion.state != SDL_BUTTON_LMASK) // 按钮未被按下，处理是否进入按钮
            // 在按钮内移动
            if (checkPointInCircle((SDL_FPoint){(float)event->motion.x, (float)event->motion.y}, gp->buttonCenter.x, gp->buttonCenter.y, gp->buttonRadius))
            {
                // 按钮未被选中 (刚进入按钮)
                if (!CHECKFLAG(gp->flag, guiPlayEnum_button_selected))
                {
                    // 设置按钮被选中
                    SETFLAG(gp->flag, guiPlayEnum_button_selected);

                    // 设置按钮被选中动画
                    SETFLAG(gp->flag, guiPlayEnum_button_selected_init);
                    SETFLAG(gp->flag, guiPlayEnum_button_selected_start);
                }
            }
            // 如果不在按钮区域内移动
            else
            {
                // 如果按钮被选中 (刚离开按钮)
                if (CHECKFLAG(gp->flag, guiPlayEnum_button_selected))
                {
                    // 清除按钮被选中
                    CLEARFLAG(gp->flag, guiPlayEnum_button_selected);

                    // 设置按钮离开动画
                    SETFLAG(gp->flag, guiPlayEnum_button_leave_init);
                    SETFLAG(gp->flag, guiPlayEnum_button_leave_start);
                }
            }

        break;
    case SDL_MOUSEBUTTONDOWN:                        // 按下事件
        if (event->button.button == SDL_BUTTON_LEFT) // 左键按下
        {
            // 如果在按钮区域内按下
            if (CHECKFLAG(gp->flag, guiPlayEnum_button_selected))
            {
                // 设置按钮被按下
                SETFLAG(gp->flag, guiPlayEnum_button_pressed);

                // 设置按钮被按下动画
                SETFLAG(gp->flag, guiPlayEnum_button_pressed_init);
                SETFLAG(gp->flag, guiPlayEnum_button_pressed_start);
            }
        }
        break;

    case SDL_MOUSEBUTTONUP:                          // 弹起事件
        if (event->button.button == SDL_BUTTON_LEFT) // 左键弹起
        {
            // 如果按钮被按下
            if (CHECKFLAG(gp->flag, guiPlayEnum_button_pressed))
            {
                // 清除按钮被按下
                CLEARFLAG(gp->flag, guiPlayEnum_button_pressed);

                // 如果在按钮区域内弹起
                if (checkPointInCircle((SDL_FPoint){(float)event->button.x, (float)event->button.y}, gp->buttonCenter.x, gp->buttonCenter.y, gp->buttonRadius))
                {
                    // 清除颜色
                    gp->buttonTexture = &gp->buttonTextureS;

                    // 设置被按下
                    ret = 1;
                }
                else
                {
                    // 清除按钮被选中
                    CLEARFLAG(gp->flag, guiPlayEnum_button_selected);

                    // 设置按钮离开动画
                    SETFLAG(gp->flag, guiPlayEnum_button_leave_init);
                    SETFLAG(gp->flag, guiPlayEnum_button_leave_start);
                }
            }
        }
        break;
    }

    return ret;
}

/** \brief 按钮渲染 */
static void gpRenderButton(guiPlay *gp)
{
    // 如果被ban则不渲染
    if (CHECKFLAG(gp->flag, guiPlayEnum_ban))
        return;

    // 渲染按钮动画
    if (CHECKFLAG(gp->flag, guiPlayEnum_button_selected_start)) // 选中动画
    {
        // 初始化
        if (CHECKFLAG(gp->flag, guiPlayEnum_button_selected_init))
        {
            CLEARFLAG(gp->flag, guiPlayEnum_button_selected_init);

            gp->buttonRadius = GUI_PLAY_BUTTON_SIZE * 1.2f;
            gp->buttonTexture = &gp->buttonTextureS;
            gp->time = SDL_GetTicks();
        }

        // 计算时间
        int time = SDL_GetTicks() - gp->time;

        // 结束
        if (time > GUI_PLAY_BUTTON_RUN_SELECT_TIME)
        {
            CLEARFLAG(gp->flag, guiPlayEnum_button_selected_start);
            time = GUI_PLAY_BUTTON_RUN_SELECT_TIME;
        }

        // 渲染
        gpRenderButtonDraw(gp, NULL, GUI_PLAY_BUTTON_SIZE * (1.0f + 0.2f * time / GUI_PLAY_BUTTON_RUN_SELECT_TIME));
    }
    else if (CHECKFLAG(gp->flag, guiPlayEnum_button_leave_start)) // 离开动画
    {
        // 初始化
        if (CHECKFLAG(gp->flag, guiPlayEnum_button_leave_init))
        {
            CLEARFLAG(gp->flag, guiPlayEnum_button_leave_init);

            gp->buttonRadius = GUI_PLAY_BUTTON_SIZE;
            gp->buttonTexture = &gp->buttonTextureS;
            gp->time = SDL_GetTicks();
        }

        // 计算时间
        int time = SDL_GetTicks() - gp->time;

        // 结束
        if (time > GUI_PLAY_BUTTON_RUN_LEAVE_TIME)
        {
            CLEARFLAG(gp->flag, guiPlayEnum_button_leave_start);
            time = GUI_PLAY_BUTTON_RUN_LEAVE_TIME;
        }

        // 渲染
        gpRenderButtonDraw(gp, NULL, GUI_PLAY_BUTTON_SIZE * (1.2f - 0.2f * time / GUI_PLAY_BUTTON_RUN_LEAVE_TIME));
    }
    else if (CHECKFLAG(gp->flag, guiPlayEnum_button_pressed_start)) // 按下动画
    {
        // 初始化
        if (CHECKFLAG(gp->flag, guiPlayEnum_button_pressed_init))
        {
            CLEARFLAG(gp->flag, guiPlayEnum_button_pressed_init);

            gp->buttonTexture = &gp->buttonTextureP;
            gp->time = SDL_GetTicks();
        }

        // 计算时间
        int time = SDL_GetTicks() - gp->time;

        // 结束
        if (time > GUI_PLAY_BUTTON_RUN_PRESS_TIME)
        {
            CLEARFLAG(gp->flag, guiPlayEnum_button_pressed_start);
            time = GUI_PLAY_BUTTON_RUN_PRESS_TIME;
        }

        // 渲染
        float r = GUI_PLAY_BUTTON_SIZE * 1.2f * time / GUI_PLAY_BUTTON_RUN_PRESS_TIME;
        SDL_RenderCopyF(gp->renderer, gp->buttonTextureS, NULL, &(SDL_FRect){.x = gp->buttonCenter.x - gp->buttonRadius, .y = gp->buttonCenter.y - gp->buttonRadius, .w = gp->buttonRadius * 2, .h = gp->buttonRadius * 2});
        SDL_RenderCopyF(gp->renderer, gp->buttonTextureP, NULL, &(SDL_FRect){.x = gp->buttonCenter.x - r, .y = gp->buttonCenter.y - r, .w = r * 2, .h = r * 2});

        // 绘制按钮十字
        SDL_SetRenderDrawColor(gp->renderer, 0, 0, 0, 255);
        SDL_RenderFillRectF(gp->renderer, &(SDL_FRect){
                                              .x = gp->buttonCenter.x - gp->buttonRadius * 0.5f,
                                              .y = gp->buttonCenter.y - gp->buttonRadius * 0.06f,
                                              .w = gp->buttonRadius,
                                              .h = gp->buttonRadius * 0.12f});
        SDL_RenderFillRectF(gp->renderer, &(SDL_FRect){
                                              .x = gp->buttonCenter.x - gp->buttonRadius * 0.06f,
                                              .y = gp->buttonCenter.y - gp->buttonRadius * 0.5f,
                                              .w = gp->buttonRadius * 0.12f,
                                              .h = gp->buttonRadius});
    }
    else
    {
        gpRenderButtonDraw(gp, NULL, -1.0f);
    }
}

/** \brief 处理图片事件 */
static void gpEventImage(guiPlay *gp, SDL_Event *event)
{
    switch (event->type)
    {
    case SDL_MOUSEMOTION: // 移动事件

        // 如果图片被选中
        if (CHECKFLAG(gp->flag, guiPlayEnum_choice))
        {
            if (abs(event->motion.x - gp->choicePos.x) > 10 ||
                abs(event->motion.y - gp->choicePos.y) > 10)
                CLEARFLAG(gp->flag, guiPlayEnum_choice); // 清除选中
        }

        // 图片被按下移动
        if (CHECKFLAG(gp->flag, guiPlayEnum_image_enter))
        {
            gp->imageRect.x += (float)event->motion.xrel;
            gp->imageRect.y += (float)event->motion.yrel;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:                        // 按下事件
        if (event->button.button == SDL_BUTTON_LEFT) // 左键按下
        {
            // 在图片内按下并且不在按钮内
            if (SDL_TRUE == SDL_PointInFRect(&(SDL_FPoint){(float)event->button.x, (float)event->button.y}, &gp->imageRect) &&
                !checkPointInCircle((SDL_FPoint){(float)event->button.x, (float)event->button.y}, gp->buttonCenter.x, gp->buttonCenter.y, gp->buttonRadius))
            {
                // 设置图片被按下
                SETFLAG(gp->flag, guiPlayEnum_image_enter);

                // 如果在人物信息内按下
                if (getThread() == false)
                {
                    list *node = gp->figure->fd;
                    while (node != gp->figure)
                    {
                        personal *p = (personal *)node->data;
                        if (SDL_PointInFRect(&(SDL_FPoint){(float)event->button.x, (float)event->button.y},
                                             &(SDL_FRect){.x = gp->imageRect.x + p->rect.x * gp->scale1 * gp->scale2,
                                                          .y = gp->imageRect.y + p->rect.y * gp->scale1 * gp->scale2,
                                                          .w = p->rect.w * gp->scale1 * gp->scale2,
                                                          .h = p->rect.h * gp->scale1 * gp->scale2}))
                        {
                            // 设置人物被选中
                            SETFLAG(gp->flag, guiPlayEnum_choice);
                            gp->choiceNode = p;
                            gp->choicePos = (SDL_Point){event->button.x, event->button.y};
                            break;
                        }

                        node = node->fd;
                    }
                }
            }
        }
        break;

    case SDL_MOUSEBUTTONUP:                          // 弹起事件
        if (event->button.button == SDL_BUTTON_LEFT) // 左键弹起
        {
            // 清除图片被按下
            CLEARFLAG(gp->flag, guiPlayEnum_image_enter);

            // 图片被选中后弹起
            if (CHECKFLAG(gp->flag, guiPlayEnum_choice))
            {
                CLEARFLAG(gp->flag, guiPlayEnum_choice);     // 清除选中
                SETFLAG(gp->flag, guiPlayEnum_choice_enter); // 确认选中
            }
        }
        break;
    case SDL_MOUSEWHEEL:
        float scale = 0.20f * event->wheel.y;                        // 缩放比例
        if (scale + gp->scale2 > 0.1f && scale + gp->scale2 < 15.0f) // 缩放比例限制
        {
            float w = gp->imageRect.w;
            float h = gp->imageRect.h;

            // 设置缩放比例
            gp->scale2 += scale;

            // 重新设置图片区域
            gp->imageRect.w = gp->imageW * gp->scale1 * gp->scale2;
            gp->imageRect.h = gp->imageH * gp->scale1 * gp->scale2;

            // 从鼠标位置向周围缩放
            gp->imageRect.x -= (((float)event->wheel.mouseX - gp->imageRect.x) / w) * (gp->imageRect.w - w);
            gp->imageRect.y -= (((float)event->wheel.mouseY - gp->imageRect.y) / h) * (gp->imageRect.h - h);
        }
        break;
    }
}

/** \brief 处理图片渲染 */
static void gpRenderImage(guiPlay *gp)
{
    SDL_RenderCopyF(gp->renderer, gp->imageTexture, NULL, &gp->imageRect);

    // 渲染人物信息
    if (getThread() == false)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);

        list *node = gp->figure->fd;
        while (node != gp->figure)
        {
            personal *p = (personal *)node->data;
            // 绘制人物框
            if (p->flag == HV)
                SDL_SetRenderDrawColor(gp->renderer, 0, 255, 0, 255);
            else
                SDL_SetRenderDrawColor(gp->renderer, 255, 0, 0, 255);

            // 人物框实际位置
            SDL_FRect rect = {.x = gp->imageRect.x + p->rect.x * gp->scale1 * gp->scale2,
                              .y = gp->imageRect.y + p->rect.y * gp->scale1 * gp->scale2,
                              .w = p->rect.w * gp->scale1 * gp->scale2,
                              .h = p->rect.h * gp->scale1 * gp->scale2};
            SDL_RenderDrawRectF(gp->renderer, &rect);

            // 绘制人物信息
            if (SDL_PointInFRect(&(SDL_FPoint){(float)x, (float)y}, &rect))
            {
                SDL_RenderCopyF(gp->renderer, p->infoTexture, NULL, &(SDL_FRect){.x = gp->imageRect.x + p->infoRect.x * gp->scale1 * gp->scale2, .y = gp->imageRect.y + p->infoRect.y * gp->scale1 * gp->scale2, .w = p->infoRect.w * gp->scale1 * gp->scale2, .h = p->infoRect.h * gp->scale1 * gp->scale2});
            }

            node = node->fd;
        }
    }
}

/** \brief 处理等待渲染 */
static void gpRenderWatt(guiPlay *gp)
{
    // init
    if (CHECKFLAG(gp->flag, guiPlayEnum_watt_init))
    {
        CLEARFLAG(gp->flag, guiPlayEnum_watt_init);
        gp->time2 = gp->time = SDL_GetTicks();
    }

    int time = SDL_GetTicks() - gp->time;

    // 等待动画1
    if (time <= GUI_PLAY_BUTTON_WATT_1_TIME && getThread() == true)
    {
        drawCircle8(&gp->buttonCenter,
                    GUI_PLAY_BUTTON_SIZE * 1.5f * (float)time / (float)GUI_PLAY_BUTTON_WATT_1_TIME,
                    GUI_PLAY_BUTTON_SIZE * (1.2f - 0.8f * (float)time / (float)GUI_PLAY_BUTTON_WATT_1_TIME),
                    time * 0.005f,
                    (int[]){0x3B, 0x57, 0x73, 0x8F, 0xAB, 0xC7, 0xE3, 0xFF},
                    gp->buttonTextureS,
                    gp->renderer);
    }
    // 等待动画2
    else if (getThread() == true)
    {
        drawCircle8(&gp->buttonCenter,
                    GUI_PLAY_BUTTON_SIZE * 1.5f,
                    GUI_PLAY_BUTTON_SIZE * 0.4f,
                    time * 0.005f,
                    (int[]){0x3B, 0x57, 0x73, 0x8F, 0xAB, 0xC7, 0xE3, 0xFF},
                    gp->buttonTextureS,
                    gp->renderer);
    }
    else
    {
        // 处理线程结束数据
        if (!CHECKFLAG(gp->flag, guiPlayEnum_watt_end))
        {
            SETFLAG(gp->flag, guiPlayEnum_watt_end);
            gp->time = SDL_GetTicks();

            // 渲染用户数据等等
            list *node = gp->figure->fd;
            while (node != gp->figure)
            {
                renderUserData((personal *)node->data, gp->renderer);
                node = node->fd;
            }
        }

        int time = SDL_GetTicks() - gp->time;

        // 等待动画3结束
        if (time >= GUI_PLAY_BUTTON_WATT_2_TIME)
        {
            time = GUI_PLAY_BUTTON_WATT_2_TIME;

            CLEARFLAG(gp->flag, guiPlayEnum_watt_end);
            CLEARFLAG(gp->flag, guiPlayEnum_button_ban);
        }

        drawCircle8(&gp->buttonCenter,
                    GUI_PLAY_BUTTON_SIZE * 1.5f * (1.0f - (float)time / (float)GUI_PLAY_BUTTON_WATT_2_TIME),
                    GUI_PLAY_BUTTON_SIZE * 1.2f * (0.4f + 0.6f * (float)time / (float)GUI_PLAY_BUTTON_WATT_2_TIME),
                    (SDL_GetTicks() - gp->time2) * 0.005f,
                    (int[]){0x3B, 0x57, 0x73, 0x8F, 0xAB, 0xC7, 0xE3, 0xFF},
                    gp->buttonTextureS,
                    gp->renderer);
    }
}

/** \brief 事件处理 */
static void gpEvent(guiPlay *gm, SDL_Event *event)
{
    // 界面被ban
    if (CHECKFLAG(gm->flag, guiPlayEnum_ban))
        return;

    // 处理按钮事件
    if (!CHECKFLAG(gm->flag, guiPlayEnum_button_ban))
    {
        // 被按下
        if (gpEventButton(gm, event) == 1)
        {
            // 选择图片
            char path[PATH_MAX] = {0};
            if (selectImageFile(path, PATH_MAX))
            {
                // 读取图片
                SDL_Surface *surface = IMG_Load(path);
                if (surface)
                {
                    gmAdd(gm->msg, "读取图片成功", guiMsgEnum_Success);

                    // 释放原来的图片
                    if (gm->imageTexture)
                        SDL_DestroyTexture(gm->imageTexture);

                    // 释放原来的人物信息
                    freeList(gm->figure, (void (*)(void *))freePersonal);

                    // 创建新的图片的纹理
                    gm->imageTexture = SDL_CreateTextureFromSurface(gm->renderer, surface);

                    // 设置图片位置, 大小及其缩放
                    gm->imageW = surface->w;
                    gm->imageH = surface->h;
                    resizeImage(&(SDL_Rect){0, 0, WINDOW_PLAY_DEFAULT_WIDTH, WINDOW_PLAY_DEFAULT_HEIGHT}, &gm->imageRect, gm->imageW, gm->imageH);
                    gm->scale1 = gm->imageRect.w / gm->imageW;
                    gm->scale2 = 1.0f;

                    // 禁用按钮同时启用等待动画
                    SETFLAG(gm->flag, guiPlayEnum_button_ban);
                    SETFLAG(gm->flag, guiPlayEnum_watt_init);

                    // 启用线程去获取识别人脸并获取人物信息
                    faceThreadParam *param = (faceThreadParam *)malloc(sizeof(faceThreadParam));
                    strcpy(param->PATH, path);
                    param->msg = gm->msg;
                    param->figure = gm->figure;

                    setThread(true);
                    pthread_create(&Global.thread, NULL, (void *(*)(void *))getFaceThread, param);

                    // 释放图片
                    SDL_FreeSurface(surface);
                }
            }
        }
    }

    // 处理图片事件
    if (!CHECKFLAG(gm->flag, guiPlayEnum_image_ban))
        gpEventImage(gm, event);
}

/** \brief 渲染界面 */
static void gpRender(guiPlay *gm)
{
    if (CHECKFLAG(gm->flag, guiPlayEnum_ban))
        return;

    // 渲染图片
    if (!CHECKFLAG(gm->flag, guiPlayEnum_image_ban))
        gpRenderImage(gm);

    // 渲染按钮
    if (!CHECKFLAG(gm->flag, guiPlayEnum_button_ban))
        gpRenderButton(gm);

    // 渲染等待动画
    else
        gpRenderWatt(gm);
}

/** \brief gui_play界面 */
void gui_play()
{
    // 创建窗口
    SDL_Window *window = SDL_CreateWindow("encFace", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_PLAY_DEFAULT_WIDTH, WINDOW_PLAY_DEFAULT_HEIGHT, SDL_WINDOW_SHOWN);
    // 创建渲染器
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // 人物信息
    list figure = {0};
    initList(&figure);

    // 消息队列
    guiMsg msg = {0};
    gmInit(&msg,
           Global.font,
           NULL, NULL, NULL, NULL, NULL, NULL,
           (SDL_Rect){20, WINDOW_PLAY_DEFAULT_HEIGHT - 30 - 20, 250, 30},
           0.7,
           10,
           3000,
           1000,
           -1,
           renderer);

    gmAdd(&msg, "登陆成功", guiMsgEnum_Success);

    // guiPlay
    guiPlay gp = {0};
    gpInit(&gp, &figure, &msg, renderer);

    // 控制帧率
    Uint64 frameStart = 0, frameTime = 0;

    // 退出标志
    int quit = 1;
    while (quit)
    {
        // 设置帧开始时间
        frameStart = SDL_GetTicks64();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                quit = 0;
                break;
            }

            // 事件处理
            gpEvent(&gp, &event);
        }

        // 清空界面
        SDL_SetRenderDrawColor(renderer, 0xDD, 0xDD, 0xDD, 255);
        SDL_RenderClear(renderer);

        // 渲染基础界面
        gpRender(&gp);

        // 检查是否需要进入新的界面
        if (CHECKFLAG(gp.flag, guiPlayEnum_choice_enter))
        {
            CLEARFLAG(gp.flag, guiPlayEnum_choice_enter);

            // 进入编辑界面
            SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_PLAY_DEFAULT_WIDTH, WINDOW_PLAY_DEFAULT_HEIGHT, 32, SDL_PIXELFORMAT_RGBA32);
            SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, surface->pixels, surface->pitch);
            quit = guiEdit(renderer, surface, &msg, gp.choiceNode);
            SDL_FreeSurface(surface);
        }

        // 渲染消息
        gmRender(&msg);

        // 更新界面
        SDL_RenderPresent(renderer);

        // 控制刷新率
        frameTime = SDL_GetTicks64() - frameStart;
        if (frameTime < FPS_MS)
            SDL_Delay(FPS_MS - frameTime);
    }

    // 释放资源
    gmDestroy(&msg);
    freeList(&figure, (void (*)(void *))freePersonal);
    SDL_DestroyTexture(gp.imageTexture);
    SDL_DestroyTexture(gp.buttonTextureS);
    SDL_DestroyTexture(gp.buttonTextureP);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

/** \brief 获取数据线程 */
void getFaceThread(faceThreadParam *param)
{
    faceThreadParam msg = {0};
    memcpy(&msg, param, sizeof(faceThreadParam));
    free(param);

    // 从facenet服务器获取人脸特征向量
    if (getFaceVector(msg.PATH, msg.figure) == false)
    {
        gmAdd(msg.msg, "获取人脸特征向量失败", guiMsgEnum_Error);
        setThread(false);
        return;
    }
    gmAdd(msg.msg, "提取人脸特征向量完成", guiMsgEnum_Success);

    // 从远程服务器获取人物消息
    list *node = msg.figure->fd;
    while (node != msg.figure)
    {
        personal *p = (personal *)node->data;
        if (getFaceInfo(p) == false)
        {
            gmAdd(msg.msg, "获取人物信息失败", guiMsgEnum_Error);
            setThread(false);
            return;
        }
        node = node->fd;
    }

    gmAdd(msg.msg, "获取人物信息成功", guiMsgEnum_Success);
    setThread(false);
    return;
}

/** \brief 渲染用户数据 */
void renderUserData(personal *p, SDL_Renderer *renderer)
{
    // 清空原有渲染
    if (p->infoTexture)
        SDL_DestroyTexture(p->infoTexture);

    // 人物是否存在则渲染消息
    if (p->flag == HV)
    {
        // 生成渲染数据
        char msg[0x200] = {0};
        sprintf(msg, "姓名: %s\n学号: %s\n学院: %s\n专业班级: %s", p->info.name, p->info.id, p->info.college, p->info.major);
        SDL_Surface *surface = TTF_RenderUTF8_Blended_Wrapped(Global.font, msg, figureLiveColorFD, 600);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

        // 创建texture
        p->infoTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, surface->w, surface->h);
        SDL_SetTextureBlendMode(p->infoTexture, SDL_BLENDMODE_BLEND);
        SDL_SetRenderTarget(renderer, p->infoTexture);
        SDL_SetRenderDrawColor(renderer, figureLiveColorBG.r, figureLiveColorBG.g, figureLiveColorBG.b, figureLiveColorBG.a);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_SetRenderTarget(renderer, NULL);

        p->infoRect = (SDL_FRect){p->rect.x + p->rect.w + 20, p->rect.y, (float)surface->w, (float)surface->h};

        // 释放空间
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
    // 人物不存在则渲染不存在
    else
    {
        char msg[] = {"没有信息"};
        SDL_Surface *surface = TTF_RenderUTF8_Blended_Wrapped(Global.font, msg, figureDeadColorFD, 600);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

        // 创建texture
        p->infoTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, surface->w, surface->h);
        SDL_SetTextureBlendMode(p->infoTexture, SDL_BLENDMODE_BLEND);
        SDL_SetRenderTarget(renderer, p->infoTexture);
        SDL_SetRenderDrawColor(renderer, figureDeadColorBG.r, figureDeadColorBG.g, figureDeadColorBG.b, figureDeadColorBG.a);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_SetRenderTarget(renderer, NULL);

        p->infoRect = (SDL_FRect){p->rect.x + p->rect.w + 20, p->rect.y, (float)surface->w, (float)surface->h};

        // 释放空间
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
}
