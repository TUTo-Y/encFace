#include "gui_text.h"

/**
 * \brief 初始化文本框
 * \param gt 文本框指针
 * \param rect 文本框位置
 * \param scale 文本框文本高度所占比例
 * \param lineWidth 文本框横线宽度
 * \param font 文本框字体
 * \param text 初始文本内容
 * \param textInitColor 初始化文本颜色
 * \param textColor 已输入和候选文本颜色
 * \param lineNormalColor 一般横线颜色
 * \param lineSelectedColor 选中横线颜色
 * \param start_time 开始时动画时间(ms)
 * \param end_time 结束时动画时间(ms)
 * \param renderer 渲染器
 * \return void
 */
void gtInit(guiText *gt,
            SDL_Rect rect,
            float scale,             // 文本框文本高度所占比例
            int lineWidth,           // 文本框横线宽度
            TTF_Font *font,          // 文本框字体
            char *text,              // 初始文本内容
            SDL_Color textInitColor, // 初始化文本颜色
            SDL_Color textColor,     // 已输入和候选文本颜色

            SDL_Color lineNormalColor,   // 一般横线颜色
            SDL_Color lineSelectedColor, // 选中横线颜色
            int start_time,              // 开始时动画时间(ms)
            int end_time,                // 结束时动画时间(ms)
            SDL_Renderer *renderer)
{
    memset(gt, 0, sizeof(guiText));

    // 初始化flag
    gt->flag = 0;

    // 初始化文本框位置
    memcpy(&gt->rect, &rect, sizeof(SDL_Rect));

    // 初始化文本框文本高度所占比例
    gt->scale = scale;

    // 初始化文本框横线宽度
    gt->lineWidth = lineWidth;

    // 初始化文本框字体
    gt->font = font;

    // 初始化文本内容
    if (text == NULL || *text == '\0')
    {
        gt->textInit = malloc(2);
        strcpy(gt->textInit, " ");
    }
    else
    {
        gt->textInit = malloc(strlen(text) + 1);
        strcpy(gt->textInit, text);
    }
    gt->textIn = malloc(1);
    gt->textIn[0] = '\0';
    gt->textSub = malloc(1);
    gt->textSub[0] = '\0';

    // 初始化颜色
    gt->textInitColor = textInitColor;
    gt->textColor = textColor;
    gt->lineNormalColor = lineNormalColor;
    gt->lineSelectedColor = lineSelectedColor;

    // 初始化动画时间
    gt->start_time = start_time;
    gt->end_time = end_time;

    // 保存渲染器
    gt->renderer = renderer;

    // 创建初始化文本
    SDL_Surface *Surface = TTF_RenderUTF8_Blended(gt->font, gt->textInit, gt->textInitColor);
    gt->texture = SDL_CreateTextureFromSurface(gt->renderer, Surface);
    SDL_FreeSurface(Surface);
}

void gtEvent(guiText *gt, SDL_Event *e)
{
    // 检查是否隐藏了文本框
    if (CHECKFLAG(gt->flag, guiTextEnum_hide))
        return;

    // 是否需要重新渲染文本
    bool renderText = false;

    switch (e->type)
    {
    case SDL_MOUSEBUTTONDOWN:                    // 鼠标按下
        if (e->button.button == SDL_BUTTON_LEFT) // 左键按下
        {
            // 在背景框内按下并且在此之前没有选中
            if (SDL_TRUE == SDL_PointInRect(&(SDL_Point){e->motion.x, e->motion.y}, &gt->rect) &&
                !CHECKFLAG(gt->flag, guiTextEnum_selected))
            {
                // 开始输入
                SDL_StartTextInput();
                SDL_SetTextInputRect(&gt->rect);
                // 设置标识
                SETFLAG(gt->flag, guiTextEnum_selected);
                SETFLAG(gt->flag, guiTextEnum_selected_start);
                gt->time = SDL_GetTicks();
                // 重新渲染文本
                renderText = true;
            }
            // 在背景框外按下并且在此之前选中
            else if (SDL_FALSE == SDL_PointInRect(&(SDL_Point){e->motion.x, e->motion.y}, &gt->rect) &&
                     CHECKFLAG(gt->flag, guiTextEnum_selected))
            {
                // 结束输入
                SDL_StopTextInput();
                // 设置标识
                CLEARFLAG(gt->flag, guiTextEnum_selected);
                SETFLAG(gt->flag, guiTextEnum_selected_end);
                gt->time = SDL_GetTicks();
                // 重新渲染文本
                renderText = true;
            }
            break;
        }

    case SDL_TEXTINPUT:                                // 文本输入
        if (CHECKFLAG(gt->flag, guiTextEnum_selected)) // 选中状态
        {
            gt->textIn = realloc(gt->textIn, strlen(gt->textIn) + strlen(e->text.text) + 1);
            strcat(gt->textIn, e->text.text);
            // 重新渲染文本
            renderText = true;
        }
        break;

    case SDL_TEXTEDITING:                              // 文本编辑
        if (CHECKFLAG(gt->flag, guiTextEnum_selected)) // 选中状态
        {
            gt->textSub = realloc(gt->textSub, strlen(e->edit.text) + 1);
            strcpy(gt->textSub, e->edit.text);
            // 重新渲染文本
            renderText = true;
        }
        break;

    case SDL_KEYDOWN:                                  // 按键按下
        if (CHECKFLAG(gt->flag, guiTextEnum_selected)) // 选中状态
        {
            switch (e->key.keysym.sym)
            {
            case SDLK_BACKSPACE: // 退格键
                if (strlen(gt->textIn) > 0)
                {
                    int len = strlen(gt->textIn);
                    while (len > 0)
                    {
                        if ((gt->textIn[len - 1] & 0xC0) != 0x80) // 找到一个UTF-8字符的开始
                            break;
                        len--;
                    }
                    if (len > 0)
                    {
                        gt->textIn[len - 1] = '\0'; // 删除这个UTF-8字符
                    }

                    renderText = true;
                }
                break;
            }
        }
        break;
    }

    // 渲染文本
    if (renderText == true)
        gtReRender(gt);
}


/**
 * \brief 设置文本
 * \param gt 文本框指针
 * \return void
 */
void gtSetText(guiText *gt, char *text)
{
    if (text == NULL || *text == '\0')
    {
        gt->textIn = realloc(gt->textIn, 1);
        gt->textIn[0] = '\0';
    }
    else
    {
        gt->textIn = realloc(gt->textIn, strlen(text) + 1);
        strcpy(gt->textIn, text);
    }

    gtReRender(gt);
}

/**
 * \brief 重新渲染文本
 */
void gtReRender(guiText *gt)
{
    // 释放原有的纹理
    if (gt->texture)
        SDL_DestroyTexture(gt->texture);

    // 合并文本
    char *text = malloc(strlen(gt->textIn) + strlen(gt->textSub) + 1);
    strcpy(text, gt->textIn);
    strcat(text, gt->textSub);

    // 创建文本框Surface
    SDL_Surface *surface = NULL;

    if (CHECKFLAG(gt->flag, guiTextEnum_selected) && *text == '\0')
    {
        surface = TTF_RenderUTF8_Blended(gt->font, " ", gt->textInitColor);
    }
    else if (*text == '\0')
        surface = TTF_RenderUTF8_Blended(gt->font, gt->textInit, gt->textInitColor);
    else
        surface = TTF_RenderUTF8_Blended(gt->font, text, gt->textColor);

    // 创建文本框纹理
    gt->texture = SDL_CreateTextureFromSurface(gt->renderer, surface);
    SDL_SetTextureBlendMode(gt->texture, SDL_BLENDMODE_BLEND);

    // 清除数据
    SDL_FreeSurface(surface);
    free(text);
}

void gtRender(guiText *gt)
{
    // 检查是否隐藏了文本框
    if (CHECKFLAG(gt->flag, guiTextEnum_hide))
        return;

    // 渲染文本
    int textW, textH; // 文本宽高
    SDL_QueryTexture(gt->texture, NULL, NULL, &textW, &textH);
    float scale = gt->rect.h * gt->scale / textH;
    SDL_FRect rect = {
        .w = (float)textW * scale,
        .h = (float)textH * scale,
        .x = CHECKFLAG(gt->flag, guiTextEnum_center) ? gt->rect.x + (gt->rect.w - (float)textW * scale) / 2.0f : gt->rect.x + gt->rect.h * (1.0f - gt->scale) / 2.0f,
        .y = CHECKFLAG(gt->flag, guiTextEnum_center) ? gt->rect.y + (gt->rect.h - (float)textH * scale) / 2.0f : gt->rect.y + gt->rect.h * (1.0f - gt->scale) / 2.0f};
    SDL_RenderCopyF(gt->renderer, gt->texture, NULL, &rect);

    // 渲染横线
        int time = SDL_GetTicks() - gt->time;
    if (CHECKFLAG(gt->flag, guiTextEnum_selected_end))
    {
        // 结束选中状态
        if (time > gt->end_time)
        {
            time = gt->end_time;
            CLEARFLAG(gt->flag, guiTextEnum_selected_end);
        }
        SDL_SetRenderDrawColor(gt->renderer,
                               (gt->lineNormalColor.r - gt->lineSelectedColor.r) * (float)time / (float)gt->end_time + gt->lineSelectedColor.r,
                               (gt->lineNormalColor.g - gt->lineSelectedColor.g) * (float)time / (float)gt->end_time + gt->lineSelectedColor.g,
                               (gt->lineNormalColor.b - gt->lineSelectedColor.b) * (float)time / (float)gt->end_time + gt->lineSelectedColor.b,
                               (gt->lineNormalColor.a - gt->lineSelectedColor.a) * (float)time / (float)gt->end_time + gt->lineSelectedColor.a);
        SDL_RenderFillRectF(gt->renderer, &(SDL_FRect){
                                              (float)gt->rect.x,
                                              (float)gt->rect.y + (float)gt->rect.h - gt->lineWidth / 2.0f,
                                              (float)gt->rect.w,
                                              (float)gt->lineWidth});
    }
    else if (CHECKFLAG(gt->flag, guiTextEnum_selected_start))
    {
        // 结束选中状态
        if (time > gt->start_time)
        {
            time = gt->start_time;
            CLEARFLAG(gt->flag, guiTextEnum_selected_start);
        }

        SDL_SetRenderDrawColor(gt->renderer, gt->lineNormalColor.r, gt->lineNormalColor.g, gt->lineNormalColor.b, gt->lineNormalColor.a);
        SDL_RenderFillRectF(gt->renderer, &(SDL_FRect){
                                              (float)gt->rect.x,
                                              (float)gt->rect.y + (float)gt->rect.h - gt->lineWidth / 2.0f,
                                              (float)gt->rect.w,
                                              (float)gt->lineWidth});
        SDL_SetRenderDrawColor(gt->renderer, gt->lineSelectedColor.r, gt->lineSelectedColor.g, gt->lineSelectedColor.b, gt->lineSelectedColor.a);
        SDL_RenderFillRectF(gt->renderer, &(SDL_FRect){
                                              (float)gt->rect.x + (float)((float)gt->rect.w - (float)gt->rect.w * (float)time / (float)gt->start_time) / 2.0f,
                                              (float)gt->rect.y + (float)gt->rect.h - gt->lineWidth / 2.0f,
                                              (float)gt->rect.w * (float)time / (float)gt->start_time,
                                              (float)gt->lineWidth});
    }
    else
    {
        if (CHECKFLAG(gt->flag, guiTextEnum_selected))
            SDL_SetRenderDrawColor(gt->renderer, gt->lineSelectedColor.r, gt->lineSelectedColor.g, gt->lineSelectedColor.b, gt->lineSelectedColor.a);

        else
            SDL_SetRenderDrawColor(gt->renderer, gt->lineNormalColor.r, gt->lineNormalColor.g, gt->lineNormalColor.b, gt->lineNormalColor.a);
        SDL_RenderFillRectF(gt->renderer, &(SDL_FRect){
                                              (float)gt->rect.x,
                                              (float)gt->rect.y + (float)gt->rect.h - gt->lineWidth / 2.0f,
                                              (float)gt->rect.w,
                                              (float)gt->lineWidth});
    }
}

/**
 * \brief 设置文本框中心对齐
 * \param gt 文本框指针
 * \param center 是否居中
 * \return void
 */
void gtSetCenter(guiText *gt, bool center)
{
    int flag = gt->flag;
    if (center)
    {
        SETFLAG(gt->flag, guiTextEnum_center);
    }
    else
    {
        CLEARFLAG(gt->flag, guiTextEnum_center);
    }

    if (flag != gt->flag)
        gtReRender(gt);
}

/**
 * \brief 格式化文本框内容
 * \param gt 文本框指针
 * \return void
 */
void gtClear(guiText *gt)
{
    gt->textIn = realloc(gt->textIn, 1);
    gt->textIn[0] = '\0';
    gt->textSub = realloc(gt->textSub, 1);
    gt->textSub[0] = '\0';

    gt->flag = 0;
    gtReRender(gt);

    CLEARFLAG(gt->flag, guiTextEnum_selected);
    CLEARFLAG(gt->flag, guiTextEnum_selected_start);
    CLEARFLAG(gt->flag, guiTextEnum_selected_end);
}

/**
 * \brief 设置文本框隐藏状态
 * \param gt 文本框指针
 * \param hide 隐藏状态
 * \return void
 */
void gtSetHide(guiText *gt, bool hide)
{
    int flag = 0;
    if (hide)
    {
        gtClear(gt);
        SETFLAG(gt->flag, guiTextEnum_hide);
    }
    else
    {
        CLEARFLAG(gt->flag, guiTextEnum_hide);
    }
}

/**
 * \brief 释放文本框
 */
void gtDestroy(guiText *gt)
{
    if (!gt)
        return;

    if (gt->textInit)
    {
        free(gt->textInit);
        gt->textInit = NULL;
    }

    if (gt->textIn)
    {
        free(gt->textIn);
        gt->textIn = NULL;
    }

    if (gt->textSub)
    {
        free(gt->textSub);
        gt->textSub = NULL;
    }

    if (gt->texture)
    {
        SDL_DestroyTexture(gt->texture);
        gt->texture = NULL;
    }

    memset(gt, 0, sizeof(guiText));
}
