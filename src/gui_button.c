#include "gui_button.h"

static SDL_Color defaultColor[3][4] = {
    {(SDL_Color){0xFF, 0xFF, 0xFF, 0xFF}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xFF}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xFF}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xFF}},
    {(SDL_Color){0xC0, 0xC0, 0xC0, 0xC0}, (SDL_Color){0xC0, 0xC0, 0xC0, 0xC0}, (SDL_Color){0xC0, 0xC0, 0xC0, 0xC0}, (SDL_Color){0xC0, 0xC0, 0xC0, 0xC0}},
    {(SDL_Color){0xA0, 0xA0, 0xA0, 0xA0}, (SDL_Color){0xA0, 0xA0, 0xA0, 0xA0}, (SDL_Color){0xA0, 0xA0, 0xA0, 0xA0}, (SDL_Color){0xA0, 0xA0, 0xA0, 0xA0}}};
static SDL_Texture *defaultTexture[3] = {
    NULL,
    NULL,
    NULL};

/**
 * \brief 初始化按钮
 * \param gb 按钮指针
 * \param rect 按钮位置
 * \param text 按钮文本
 * \param font 按钮文本字体
 * \param color 按钮文本颜色
 * \param scale 按钮文本高度所占比例
 * \param radius 按钮圆角半径
 * \param colorNSP 分别为一般状态颜色, 选中状态颜色, 按下状态颜色
 * \param textureNSP 分别为按钮一般纹理(可选), 按钮选中纹理(可选), 按钮按下纹理(可选)
 * \param renderer 渲染器
 * \return void
 */
void gbInit(guiButton *gb,
            SDL_Rect *rect, // 按钮位置

            const char *text, // 按钮文本
            TTF_Font *font,   // 按钮文本字体
            SDL_Color *color, // 按钮文本颜色
            float scale,      // 按钮文本高度所占比例

            float radius,                  // 按钮圆角半径
            SDL_Color (*colorNSP)[3][4],   // 一般状态颜色, 选中状态颜色, 按下状态颜色
            SDL_Texture *(*textureNSP)[3], // 按钮一般纹理(可选), 按钮选中纹理(可选), 按钮按下纹理(可选)

            SDL_Renderer *renderer // 渲染器
)
{
    if (colorNSP == NULL)
        colorNSP = &defaultColor;
    if (textureNSP == NULL)
        textureNSP = &defaultTexture;

    // flag
    gb->flag = 0;

    // 初始化按钮位置
    memcpy(&gb->Rect, rect, sizeof(SDL_Rect));

    // 初始化渲染器
    gb->Renderer = renderer;

    // 初始化按钮文本及背景
    SDL_Surface *textSurface = TTF_RenderUTF8_Blended(font, text, *color);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    float Scale = rect->h * scale / textSurface->h; // 按钮文本实际缩放
    SDL_FRect textRect = {
        .w = (float)textSurface->w * Scale,
        .h = (float)textSurface->h * Scale,
        .x = ((float)rect->w - textSurface->w * Scale) / 2.0f,
        .y = ((float)rect->h - textSurface->h * Scale) / 2.0f};

    // 渲染按钮文本
    for (int i = 0; i < 3; i++)
    {
        gb->TextureNSP[i] = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, rect->w, rect->h);
        SDL_SetTextureBlendMode(gb->TextureNSP[i], SDL_BLENDMODE_BLEND); // 设置纹理混合模式

        SDL_Texture *back = drawRoundRect(rect->w, rect->h, (*colorNSP)[i], radius, renderer, (*textureNSP)[i]);

        SDL_SetRenderTarget(renderer, gb->TextureNSP[i]);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, back, NULL, NULL);
        SDL_RenderCopyF(renderer, textTexture, NULL, &textRect);

        SDL_DestroyTexture(back);
    }
    gb->Texture = &gb->TextureNSP[0];

    SDL_SetRenderTarget(renderer, NULL);
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

/**
 * \brief 格式化按钮内容
 * \return void
 */
void gbClear(guiButton *gb)
{
    gb->flag = 0;
}

/**
 * \brief 设置按钮隐藏状态
 * \param gb 按钮指针
 * \param hide 是否隐藏
 * \return void
 */
void gbSetHide(guiButton *gb, bool hide)
{
    if (hide)
    {
        gbClear(gb);
        SETFLAG(gb->flag, guiButtonEnum_hide);
    }
    else
        CLEARFLAG(gb->flag, guiButtonEnum_hide);
}

/**
 * \brief 处理按钮事件
 */
bool gbEvent(guiButton *gb, SDL_Event *e)
{
    bool clicked = false;

    // 按钮隐藏则不处理事件
    if (CHECKFLAG(gb->flag, guiButtonEnum_hide))
        return clicked;

    // 处理事件
    switch (e->type)
    {
    case SDL_MOUSEMOTION: // 鼠标移动
        if (SDL_TRUE == SDL_PointInRect(&(SDL_Point){e->motion.x, e->motion.y}, &gb->Rect))
        {
            SETFLAG(gb->flag, guiButtonEnum_selected);
        }
        else if (CHECKFLAG(gb->flag, guiButtonEnum_selected))
        {
            CLEARFLAG(gb->flag, guiButtonEnum_selected);
        }
        break;

    case SDL_MOUSEBUTTONDOWN: // 鼠标按下
                              // 在按钮区域内左键按下
        if (e->button.button == SDL_BUTTON_LEFT &&
            SDL_TRUE == SDL_PointInRect(&(SDL_Point){e->button.x, e->button.y}, &gb->Rect))
            SETFLAG(gb->flag, guiButtonEnum_pressed);
        break;

    case SDL_MOUSEBUTTONUP:                      // 鼠标弹起
        if (e->button.button == SDL_BUTTON_LEFT) // 左键弹起
        {
            // 按钮被点击后并在按钮内弹起
            if (CHECKFLAG(gb->flag, guiButtonEnum_pressed) && SDL_TRUE == SDL_PointInRect(&(SDL_Point){e->button.x, e->button.y}, &gb->Rect))
            {
                clicked = true;
            }
            CLEARFLAG(gb->flag, guiButtonEnum_pressed);
        }
        break;
    }
    return clicked;
}

/**
 * \brief 渲染按钮
 */
void gbRender(guiButton *gb)
{
    // 按钮隐藏则不渲染
    if (CHECKFLAG(gb->flag, guiButtonEnum_hide))
        return;

    // 渲染按钮
    if (CHECKFLAG(gb->flag, guiButtonEnum_pressed))
        gb->Texture = &gb->TextureNSP[2];
    else if (CHECKFLAG(gb->flag, guiButtonEnum_selected))
        gb->Texture = &gb->TextureNSP[1];
    else
        gb->Texture = &gb->TextureNSP[0];
    SDL_RenderCopy(gb->Renderer, *gb->Texture, NULL, &gb->Rect);
}

/**
 * \brief 销毁按钮
 */
void gbDestroy(guiButton *gb)
{
    for (int i = 0; i < 3; i++)
        SDL_DestroyTexture(gb->TextureNSP[i]);

    memset(gb, 0, sizeof(guiButton));
}
