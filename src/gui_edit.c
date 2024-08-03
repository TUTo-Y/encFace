#include "gui_edit.h"

int guiEdit(SDL_Renderer *renderer, SDL_Surface *backSurface, guiMsg *msg, personal *p)
{
    int ret = 1;
    SDL_Texture *backTexture = SDL_CreateTextureFromSurface(renderer, backSurface);

    // 创建背景框
    SDL_Rect backRect = {(WINDOW_EDIT_DEFAULT_WIDTH - 320.0f) / 2,
                         (WINDOW_EDIT_DEFAULT_HEIGHT - 400.0f) / 2,
                         320.0,
                         500.0};
    SDL_Texture *backRectTexture =
        drawRoundRect(backRect.w,
                      backRect.h,
                      (SDL_Color[4]){(SDL_Color){255, 255, 255, 160},
                                     (SDL_Color){255, 255, 255, 160},
                                     (SDL_Color){255, 255, 255, 160},
                                     (SDL_Color){255, 255, 255, 160}},
                      0.1f,
                      renderer, NULL);

    // 创建输入框
    SDL_Rect textRect[4] = {{backRect.x + 25,
                             backRect.y + 50 + 85 * 0,
                             backRect.w - 50,
                             60},
                            {backRect.x + 25,
                             backRect.y + 50 + 85 * 1,
                             backRect.w - 50,
                             60},
                            {backRect.x + 25,
                             backRect.y + 50 + 85 * 2,
                             backRect.w - 50,
                             60},
                            {backRect.x + 25,
                             backRect.y + 50 + 85 * 3,
                             backRect.w - 50,
                             60}};
    guiText text[4] = {0};
    gtInit(&text[0],
           textRect[0],
           0.7,
           2,
           Global.font,
           "姓名",
           (SDL_Color){0x80, 0x80, 0x80, 0xFF},
           (SDL_Color){0x00, 0x00, 0x00, 0xff},
           (SDL_Color){0x80, 0x80, 0x80, 0xFF},
           (SDL_Color){67, 244, 210, 0xFF},
           200, 200,
           renderer);
    gtInit(&text[1],
           textRect[1],
           0.7,
           2,
           Global.font,
           "学号",
           (SDL_Color){0x80, 0x80, 0x80, 0xFF},
           (SDL_Color){0x00, 0x00, 0x00, 0xff},
           (SDL_Color){0x80, 0x80, 0x80, 0xFF},
           (SDL_Color){67, 244, 210, 0xFF},
           200, 200,
           renderer);
    gtInit(&text[2],
           textRect[2],
           0.7,
           2,
           Global.font,
           "学院",
           (SDL_Color){0x80, 0x80, 0x80, 0xFF},
           (SDL_Color){0x00, 0x00, 0x00, 0xff},
           (SDL_Color){0x80, 0x80, 0x80, 0xFF},
           (SDL_Color){67, 244, 210, 0xFF},
           200, 200,
           renderer);
    gtInit(&text[3],
           textRect[3],
           0.7,
           2,
           Global.font,
           "专业班级",
           (SDL_Color){0x80, 0x80, 0x80, 0xFF},
           (SDL_Color){0x00, 0x00, 0x00, 0xff},
           (SDL_Color){0x80, 0x80, 0x80, 0xFF},
           (SDL_Color){67, 244, 210, 0xFF},
           200, 200,
           renderer);

    // 设置文本
    gtSetText(&text[0], p->info.name);
    gtReRender(&text[0]);
    gtSetText(&text[1], p->info.id);
    gtReRender(&text[1]);
    gtSetText(&text[2], p->info.college);
    gtReRender(&text[2]);
    gtSetText(&text[3], p->info.major);
    gtReRender(&text[3]);

    // 创建按钮
    SDL_Rect buttonRect[2] = {{backRect.x + 25,
                               backRect.y + 50 + 85 * 4,
                               130,
                               60},
                              {backRect.x + 165,
                               backRect.y + 50 + 85 * 4,
                               130,
                               60}};
    guiButton button[2] = {0};

    gbInit(&button[0],
           &buttonRect[0],
           "更新",
           Global.font,
           &(SDL_Color){0x00, 0x00, 0x00, 0xFF},
           0.8f,
           0.2f,
           &(SDL_Color[3][4])
           // 一般状态
           {{{0x4A, 0xC2, 0x9A, 0xFF}, {0xBD, 0xFF, 0xF3, 0xFF}, {0xBD, 0xFF, 0xF3, 0xFF}, {0x4A, 0xC2, 0x9A, 0xFF}},
            // 选中状态
            {{0x43, 0xC6, 0xAC, 0xFF}, {0xF8, 0xFF, 0xAE, 0xFF}, {0xF8, 0xFF, 0xAE, 0xFF}, {0x43, 0xC6, 0xAC, 0xFF}},
            // 按下状态
            {{0x40, 0xC0, 0xA0, 0xFF}, {0xF0, 0xF0, 0xA0, 0xFF}, {0xF0, 0xF0, 0xA0, 0xFF}, {0x40, 0xC0, 0xA0, 0xFF}}},
           NULL,
           renderer);

    gbInit(&button[1],
           &buttonRect[1],
           "取消",
           Global.font,
           &(SDL_Color){0x00, 0x00, 0x00, 0xFF},
           0.8f,
           0.2f,
           &(SDL_Color[3][4])
           // 一般状态
           {{{0xE0, 0xEA, 0xFC, 0xFF}, {0xCF, 0xDE, 0xF3, 0xFF}, {0xCF, 0xDE, 0xF3, 0xFF}, {0xE0, 0xEA, 0xFC, 0xFF}},
            // 选中状态
            {{0xD0, 0xDA, 0xEC, 0xFF}, {0xBF, 0xCE, 0xD3, 0xFF}, {0xBF, 0xCE, 0xD3, 0xFF}, {0xD0, 0xDA, 0xEC, 0xFF}},
            // 按下状态
            {{0xC0, 0xCA, 0xDC, 0xFF}, {0xAF, 0xBE, 0xC3, 0xFF}, {0xAF, 0xBE, 0xC3, 0xFF}, {0xC0, 0xCA, 0xDC, 0xFF}}},
           NULL,
           renderer);

    // 动画效果
    {
        SDL_Texture *Texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, WINDOW_EDIT_DEFAULT_WIDTH, WINDOW_EDIT_DEFAULT_HEIGHT);
        SDL_SetTextureBlendMode(Texture, SDL_BLENDMODE_BLEND);
        SDL_SetRenderTarget(renderer, Texture);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, backRectTexture, NULL, &backRect);
        gtRender(&text[0]);
        gtRender(&text[1]);
        gtRender(&text[2]);
        gtRender(&text[3]);
        gbRender(&button[0]);
        gbRender(&button[1]);
        SDL_SetRenderTarget(renderer, NULL);

        int tiem = SDL_GetTicks();
        for (int i = 0; i < 255; i++)
        {

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderClear(renderer);

            SDL_SetTextureAlphaMod(Texture, i);
            SDL_RenderCopy(renderer, backTexture, NULL, NULL);
            SDL_RenderCopy(renderer, Texture, NULL, NULL);
            gmRender(msg);
            SDL_RenderPresent(renderer);

            if(i % 2)
                SDL_Delay(1);
        }

        SDL_DestroyTexture(Texture);
    }

    // 控制帧率
    Uint64 frameStart = 0, frameTime = 0;

    int quit = 1;
    while (quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                ret = 0;
                quit = 0;
            }
            gtEvent(&text[0], &event);
            gtEvent(&text[1], &event);
            gtEvent(&text[2], &event);
            gtEvent(&text[3], &event);
            if (gbEvent(&button[0], &event) == true) // 点击更新
            {
                // 复制原有信息
                strcpy(p->info.name, text[0].textIn);
                strcpy(p->info.id, text[1].textIn);
                strcpy(p->info.college, text[2].textIn);
                strcpy(p->info.major, text[3].textIn);

                if (uploadFaceInfo(p) == true)
                {
                    p->flag = HV;

                    // 重新渲染该信息
                    renderUserData(p, renderer);
                    // 添加消息
                    gmAdd(msg, "更新成功", guiMsgEnum_Success);

                    // 退出该界面
                    quit = 0;
                }
                else
                    gmAdd(msg, "更新失败", guiMsgEnum_Error);
            }
            if (gbEvent(&button[1], &event)) // 点击取消
            {
                quit = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, backTexture, NULL, NULL);
        SDL_RenderCopy(renderer, backRectTexture, NULL, &backRect);

        // 渲染文本框
        gtRender(&text[0]);
        gtRender(&text[1]);
        gtRender(&text[2]);
        gtRender(&text[3]);

        // 渲染按钮
        gbRender(&button[0]);
        gbRender(&button[1]);

        // 渲染消息
        gmRender(msg);

        SDL_RenderPresent(renderer);

        // 控制刷新率
        frameTime = SDL_GetTicks64() - frameStart;
        if (frameTime < FPS_MS)
            SDL_Delay(FPS_MS - frameTime);
    }

    // 退出动画效果
    {
        SDL_Texture *Texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, WINDOW_EDIT_DEFAULT_WIDTH, WINDOW_EDIT_DEFAULT_HEIGHT);
        SDL_SetTextureBlendMode(Texture, SDL_BLENDMODE_BLEND);
        SDL_SetRenderTarget(renderer, Texture);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, backRectTexture, NULL, &backRect);
        gtRender(&text[0]);
        gtRender(&text[1]);
        gtRender(&text[2]);
        gtRender(&text[3]);
        gbRender(&button[0]);
        gbRender(&button[1]);
        SDL_SetRenderTarget(renderer, NULL);

        int tiem = SDL_GetTicks();
        for (int i = 0; i < 255; i++)
        {

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderClear(renderer);

            SDL_SetTextureAlphaMod(Texture, 255 - i);
            SDL_RenderCopy(renderer, backTexture, NULL, NULL);
            SDL_RenderCopy(renderer, Texture, NULL, NULL);
            gmRender(msg);
            SDL_RenderPresent(renderer);
            if(i % 2)
            SDL_Delay(1);
        }

        SDL_DestroyTexture(Texture);
    }

    SDL_DestroyTexture(backRectTexture);
    SDL_DestroyTexture(backTexture);
    return ret;
}