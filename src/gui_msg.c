#include "gui_msg.h"

// 正确消息字体颜色 (前景色: 绿色, 背景色: 浅绿色)
static SDL_Color colorDefaultSuccessFD = {0, 0, 0, 255};       // 正确消息字体颜色
static SDL_Color colorDefaultSuccessBK = {204, 255, 204, 255}; // 正确消息背景颜色

// 警告消息字体颜色 (前景色: 橙色, 背景色: 浅橙色)
static SDL_Color colorDefaultWarningFD = {0, 0, 0, 255};       // 警告消息字体颜色
static SDL_Color colorDefaultWarningBK = {255, 229, 204, 255}; // 警告消息背景颜色

// 错误消息字体颜色 (前景色: 红色, 背景色: 浅红色)
static SDL_Color colorDefaultErrorFD = {0, 0, 0, 255};       // 错误消息字体颜色
static SDL_Color colorDefaultErrorBK = {255, 204, 204, 255}; // 错误消息背景颜色

#define GUI_MSG_ADD_START 100
#define GUI_MSG_ADD_MIDDLE 100

/**
 * \brief 初始化消息
 * \param gm 消息指针
 * \param Font 消息字体
 * \param colorSuccessFD 正确消息字体颜色(可选)
 * \param colorSuccessBK 正确消息背景颜色(可选)
 * \param colorWarningFD 警告消息字体颜色(可选)
 * \param colorWarningBK 警告消息背景颜色(可选)
 * \param colorErrorFD 错误消息字体颜色(可选)
 * \param colorErrorBK 错误消息背景颜色(可选)
 * \param Rect 第一条消息的位置
 * \param scale 消息文本高度所占缩放比例
 * \param interval 间隔长度
 * \param timeLive 消息显示时间(ms)
 * \param timeFade 消息消失时间(ms)
 * \param direction 消息移动方向(向下1, 向上-1)
 * \param Renderer 渲染器
 * \return void
 */
void gmInit(guiMsg *gm,
            TTF_Font *Font,            // 消息字体
            SDL_Color *colorSuccessFD, // 正确消息字体颜色(可选)
            SDL_Color *colorSuccessBK, // 正确消息背景颜色(可选)
            SDL_Color *colorWarningFD, // 警告消息字体颜色(可选)
            SDL_Color *colorWarningBK, // 警告消息背景颜色(可选)
            SDL_Color *colorErrorFD,   // 错误消息字体颜色(可选)
            SDL_Color *colorErrorBK,   // 错误消息背景颜色(可选)

            SDL_Rect Rect, // 第一条消息的位置
            float scale,   // 消息文本高度所占缩放比例
            int interval,  // 间隔长度
            int timeLive,  // 消息显示时间(ms)
            int timeFade,  // 消息消失时间(ms)
            int direction, // 消息移动方向(向下1, 向上-1)

            SDL_Renderer *Renderer // 渲染器
)
{
    // 初始化flag
    gm->flag = 0;

    // 初始化消息队列
    initList(&gm->proc);
    initList(&gm->watt);
    initList(&gm->NoRender);

    // 初始化消息字体
    gm->Font = Font;

    // 设置默认颜色
    if (colorSuccessFD == NULL)
        colorSuccessFD = &colorDefaultSuccessFD;
    if (colorWarningFD == NULL)
        colorWarningFD = &colorDefaultWarningFD;
    if (colorErrorFD == NULL)
        colorErrorFD = &colorDefaultErrorFD;
    if (colorSuccessBK == NULL)
        colorSuccessBK = &colorDefaultSuccessBK;
    if (colorWarningBK == NULL)
        colorWarningBK = &colorDefaultWarningBK;
    if (colorErrorBK == NULL)
        colorErrorBK = &colorDefaultErrorBK;
    gm->colorSuccessFD = *colorSuccessFD;
    gm->colorWarningFD = *colorWarningFD;
    gm->colorErrorFD = *colorErrorFD;
    gm->colorSuccessBK = *colorSuccessBK;
    gm->colorWarningBK = *colorWarningBK;
    gm->colorErrorBK = *colorErrorBK;

    gm->Rect = Rect;
    gm->Rect.w = TO32(gm->Rect.w); // 对齐宽度
    gm->scale = scale;
    gm->interval = interval;
    gm->timeLive = timeLive;
    gm->timeFade = timeFade;
    gm->direction = direction;

    gm->Renderer = Renderer;

    pthread_mutex_init(&gm->lock, NULL);

    gm->time = SDL_GetTicks();
}

// 释放消息
static void gmFreeMsg(guiM *m)
{
    SDL_DestroyTexture(m->Texture);
    m->time = 0;
    free(m);
}

// 渲染已经添加的消息
static void gmRenderProcMsg(guiMsg *gm, int move, int timeNow)
{
    list *node = gm->proc.bk;
    float y = gm->Rect.y + move * gm->direction;

    while (node != &gm->proc)
    {
        guiM *m = (guiM *)node->data;

        // 计算消息经过的时间
        int time = timeNow - m->time;
        // 计算消息的透明度
        if (time >= gm->timeLive)
        {
            if (time >= gm->timeLive + gm->timeFade)
                time = gm->timeLive + gm->timeFade;
            // 消息消失中
            SDL_SetTextureAlphaMod(m->Texture, (float)255.0f - (time - gm->timeLive) * 255.0f / gm->timeFade);
        }
        else
            // 消息未消失
            SDL_SetTextureAlphaMod(m->Texture, 255);

        // 计算消息的位置
        SDL_FRect rect = {
            .x = gm->Rect.x,
            .y = y,
            .w = gm->Rect.w,
            .h = gm->Rect.h};
        y += (gm->Rect.h + gm->interval) * gm->direction;

        // 渲染消息
        SDL_RenderCopyF(gm->Renderer, m->Texture, NULL, &rect);

        // 渲染
        node = node->bk;
    }
}

// 渲染未渲染的消息
void gmRenderNoRender(guiMsg *gm)
{
    pthread_mutex_lock(&gm->lock);

    if (getListCount(&gm->NoRender) == 0)
    {
        pthread_mutex_unlock(&gm->lock);
        return;
    }

    list *node = NULL;
    while (node = getNodeFromListStart(&gm->NoRender))
    {
        // 消息数据
        guiM m;

        // 选择渲染颜色
        SDL_Color colorFD;
        SDL_Color colorBK;
        switch (((guiNoRender *)node->data)->flag)
        {
        case guiMsgEnum_Success:
            colorFD = gm->colorSuccessFD;
            colorBK = gm->colorSuccessBK;
            break;
        case guiMsgEnum_Warning:
            colorFD = gm->colorWarningFD;
            colorBK = gm->colorWarningBK;
            break;
        case guiMsgEnum_Error:
            colorFD = gm->colorErrorFD;
            colorBK = gm->colorErrorBK;
            break;
        }

        // 渲染出文本
        SDL_Surface *textSurface = TTF_RenderUTF8_Blended(gm->Font, ((guiNoRender *)node->data)->msg, colorFD);
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(gm->Renderer, textSurface);
        float scale = gm->Rect.h * gm->scale / textSurface->h;

        // 渲染出背景
        SDL_Texture *backTextureFd = // 前半部分消息背景
            drawRect(
                gm->Rect.w * 2.0f / 3.0f, gm->Rect.h,
                (SDL_Color[]){colorBK, colorBK, colorBK, colorBK},
                gm->Renderer, NULL);
        SDL_Texture *backTextureBk = // 后半部分消息背景
            drawRect(
                gm->Rect.w / 3.0f, gm->Rect.h,
                (SDL_Color[]){
                    (SDL_Color)colorBK,
                    (SDL_Color){colorBK.r, colorBK.g, colorBK.b, 0},
                    (SDL_Color){colorBK.r, colorBK.g, colorBK.b, 0},
                    (SDL_Color)colorBK},
                gm->Renderer, NULL);

        // 合并出完整消息纹理
        m.Texture = SDL_CreateTexture(gm->Renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, gm->Rect.w, gm->Rect.h);
        SDL_SetTextureBlendMode(m.Texture, SDL_BLENDMODE_BLEND);

        SDL_SetRenderTarget(gm->Renderer, m.Texture);
        SDL_SetRenderDrawColor(gm->Renderer, 0xFF, 0xFF, 0xFF, 0);
        SDL_RenderClear(gm->Renderer);
        SDL_RenderCopyF(gm->Renderer, backTextureFd, NULL, &(SDL_FRect){0.0f, 0.0f, gm->Rect.w * 2.0f / 3.0f, (float)gm->Rect.h});
        SDL_RenderCopyF(gm->Renderer, backTextureBk, NULL, &(SDL_FRect){gm->Rect.w * 2.0f / 3.0f, 0.0f, gm->Rect.w / 3.0f, (float)gm->Rect.h});
        SDL_RenderCopyF(gm->Renderer, textTexture, NULL,
                        &(SDL_FRect){
                            .w = (float)textSurface->w * scale,
                            .h = (float)textSurface->h * scale,
                            .x = gm->Rect.h * (1.0f - gm->scale) / 2.0f,
                            .y = gm->Rect.h * (1.0f - gm->scale) / 2.0f});
        SDL_SetRenderTarget(gm->Renderer, NULL);

        // 释放资源
        SDL_DestroyTexture(backTextureBk);
        SDL_DestroyTexture(backTextureFd);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        // 设置消息时间
        m.time = SDL_GetTicks();

        // 添加进消息队列
        addNodeToListEnd(&gm->watt, addDataToNode(createNode(), &m, sizeof(guiM), true));
        // 删除未渲染消息
        deleteNode(NULL, node, free);
    }

    pthread_mutex_unlock(&gm->lock);
}

/**
 * \brief 渲染消息
 * \param gm 消息指针
 * \return void
 */
void gmRender(guiMsg *gm)
{
    int move = 0;
    int time = SDL_GetTicks() - gm->time;

    // 进入添加消息
    if (CHECKFLAG(gm->flag, guiMsgEnum_Add_Start))
    {
        if (time >= GUI_MSG_ADD_START)
        {
            // 设置标识
            CLEARFLAG(gm->flag, guiMsgEnum_Add_Start);
            SETFLAG(gm->flag, guiMsgEnum_Add_Middle);

            // 更新时间
            gm->time = SDL_GetTicks();
            time = GUI_MSG_ADD_START;
        }
        move = (int)((float)time * (float)(gm->Rect.h + gm->interval) / (float)GUI_MSG_ADD_START);
    }
    else if (CHECKFLAG(gm->flag, guiMsgEnum_Add_Middle))
    {
        if (time >= GUI_MSG_ADD_MIDDLE)
        {
            // 设置标识
            CLEARFLAG(gm->flag, guiMsgEnum_Add_Middle);
            CLEARFLAG(gm->flag, guiMsgEnum_Add);

            // 添加消息
            addNodeToListEnd(&gm->proc, getNodeFromListStart(&gm->watt));
        }
        else
        {
            move = gm->Rect.h + gm->interval;

            // 渲染watt头的第一个消息
            guiM *m = (guiM *)gm->watt.fd->data;
            SDL_Rect srect = {
                .x = 0,
                .y = 0,
                .w = (int)((float)gm->Rect.w * (float)time / (float)GUI_MSG_ADD_MIDDLE),
                .h = gm->Rect.h};
            SDL_Rect drect = {
                .x = gm->Rect.x,
                .y = gm->Rect.y,
                .w = (int)((float)gm->Rect.w * (float)time / (float)GUI_MSG_ADD_MIDDLE),
                .h = gm->Rect.h};

            SDL_RenderCopy(gm->Renderer, m->Texture, &srect, &drect);
        }
    }

    // 渲染已经添加的消息
    gmRenderProcMsg(gm, move, SDL_GetTicks());

    // 将未渲染的消息渲染进入watt
    gmRenderNoRender(gm);
    
    // 添加新的消息
    if (!CHECKFLAG(gm->flag, guiMsgEnum_Add))
    {
        if (getListCount(&gm->watt) > 0)
        {
            // 设置消息标志
            SETFLAG(gm->flag, guiMsgEnum_Add);
            SETFLAG(gm->flag, guiMsgEnum_Add_Start);
            gm->time = SDL_GetTicks();
        }
    }

    // 清除旧消息
    if (getListCount(&gm->proc) > 0)
    {
        // 如果最后一个消息已经消失
        if (((guiM *)gm->proc.fd->data)->time + gm->timeLive + gm->timeFade < SDL_GetTicks())
        {
            deleteNode(&gm->proc, gm->proc.fd, (void (*)(void *))gmFreeMsg);
        }
    }

}

/**
 * \brief 添加消息
 * \param gm 消息指针
 * \param msg 消息内容
 * \param flag 消息标志
 */
void gmAdd(guiMsg *gm, char *msg, int flag)
{
    pthread_mutex_lock(&gm->lock);

    // 将消息和字符串拷贝进入消息队列
    guiNoRender *msgNoRender = malloc(sizeof(guiNoRender));
    strcpy(msgNoRender->msg, msg);
    msgNoRender->flag = flag;
    addNodeToListEnd(&gm->NoRender, addDataToNode(createNode(), msgNoRender, sizeof(guiNoRender), false));

    pthread_mutex_unlock(&gm->lock);
}

/**
 * \brief 删除所有消息
 */
void gmDestroy(guiMsg *gm)
{
    pthread_mutex_destroy(&gm->lock);
    
    // 删除proc消息
    freeList(&gm->proc, (void (*)(void *))gmFreeMsg);
    // 删除watt消息
    freeList(&gm->watt, (void (*)(void *))gmFreeMsg);
    // 删除NoRender消息
    freeList(&gm->NoRender, free);

    gm->flag = 0;
}