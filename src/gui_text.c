#include "gui_text.h"

void gtInit(guiText *gt, SDL_Rect rect, SDL_Rect textRect, SDL_Color textColor)
{
    if (!gt)
        return;

    memset(gt, 0, sizeof(guiText));

    // 初始化文本框
    gt->textIn = malloc(1);
    gt->textIn[0] = '\0';
    gt->textSub = malloc(1);
    gt->textSub[0] = '\0';

    // 初始化文本颜色
    gt->rect = rect;
    gt->textRect = textRect;
    gt->textColor = textColor;
    gt->isRun = false;
}

void gtEvent(guiText *gt, SDL_Event *e, SDL_Renderer *renderer)
{
    bool renderText = false;

    if (!gt)
        return;

    switch (e->type)
    {
    case SDL_MOUSEBUTTONDOWN: // 鼠标按下

        if (e->button.button == SDL_BUTTON_LEFT) // 左键按下
        {
            SDL_Point mousePoint = {e->motion.x, e->motion.y};

            // 在背景框内按下并且没有在处理文本框
            if (SDL_PointInRect(&mousePoint, &gt->rect) == SDL_TRUE &&
                gt->isRun == false)
            {
                SDL_StartTextInput();
                SDL_SetTextInputRect(&gt->rect);
                gt->isRun = true;
            }
            // 在背景框外按下并且正在处理文本框
            else if (SDL_PointInRect(&mousePoint, &gt->rect) == SDL_FALSE &&
                     gt->isRun == true)
            {
                SDL_StopTextInput();
                gt->isRun = false;
            }
            break;
        }

    case SDL_TEXTINPUT: // 文本输入
        if (gt->isRun == true)
        {
            gt->textIn = realloc(gt->textIn, strlen(gt->textIn) + strlen(e->text.text) + 1);
            strcat(gt->textIn, e->text.text);
            renderText = true;
        }
        break;

    case SDL_TEXTEDITING: // 文本编辑
        if (gt->isRun == true)
        {
            gt->textSub = realloc(gt->textSub, strlen(e->edit.text) + 1);
            strcpy(gt->textSub, e->edit.text);
            renderText = true;
        }
        break;

    case SDL_KEYDOWN: // 按键按下
        if (gt->isRun == true)
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
            case SDLK_RETURN: // 回车键
                break;
            }
        }
        break;
    }

    // 渲染文本
    if (renderText)
    {
        // 释放原有的表面和纹理
        if (gt->surface)
        {
            SDL_FreeSurface(gt->surface);
            gt->surface = NULL;
        }
        if (gt->texture)
        {
            SDL_DestroyTexture(gt->texture);
            gt->texture = NULL;
        }

        // 合并文本
        char *text = malloc(strlen(gt->textIn) + strlen(gt->textSub) + 1);
        strcpy(text, gt->textIn);
        strcat(text, gt->textSub);

        // 创建文本框Surface TTF_RenderUTF8_Blended
        gt->surface = TTF_RenderUTF8_Blended(Global.font, text, gt->textColor);

        // 创建文本框纹理
        gt->texture = SDL_CreateTextureFromSurface(renderer, gt->surface);

        // 渲染文本
        free(text);
    }
}

void gtRender(guiText *gt, SDL_Renderer *renderer)
{
    if (!gt || !renderer || !gt->texture || !gt->surface)
        return;

    // SDL_FRect rect = {
    //     .x = (float)gt->textRect.x,
    //     .y = (float)gt->textRect.y,
    //     .w = (float)gt->surface->w * ((float)gt->textRect.h / (float)gt->surface->h),
    //     .h = (float)gt->textRect.h,
    // };

    float scale = (float)gt->textRect.h / (float)gt->surface->h;
    SDL_FRect rect = {
        .x = (float)gt->textRect.x + (gt->textRect.w - (float)gt->surface->w * scale) / 2.0f,
        .y = (float)gt->textRect.y,
        .w = (float)gt->surface->w * scale,
        .h = (float)gt->textRect.h,
    };

    SDL_RenderCopyF(renderer, gt->texture, NULL, &rect);
}

/**
 * \brief 释放文本框
 */
void gtFree(guiText *gt)
{
    if (!gt)
        return;

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

    if (gt->surface)
    {
        SDL_FreeSurface(gt->surface);
        gt->surface = NULL;
    }

    if (gt->texture)
    {
        SDL_DestroyTexture(gt->texture);
        gt->texture = NULL;
    }

    memset(gt, 0, sizeof(guiText));
}
