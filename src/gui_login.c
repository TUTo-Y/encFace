#include "gui_login.h"

// 登陆
void *login(void *arg);
// 注册
void *reg(void *arg);

/**
 * \brief 扰动函数
 * \param window_width 窗口宽度
 * \param window_height 窗口高度
 * \param x 鼠标 x 坐标
 * \param y 鼠标 y 坐标
 * \param stonesize 石头大小
 * \param stoneweight 石头力度
 * \param buf 波幅数组
 */
static void disturb(int window_width, int window_height, int x, int y, int stonesize, int stoneweight, short *buf)
{
    if ((x >= window_width - stonesize) ||
        (x < stonesize) ||
        (y >= window_height - stonesize) ||
        (y < stonesize))
        return;

    int stonesize_squared = stonesize * stonesize;
    for (int posx = x - stonesize; posx < x + stonesize; posx++)
    {
        int dx_squared = (posx - x) * (posx - x);
        for (int posy = y - stonesize; posy < y + stonesize; posy++)
        {
            int dy_squared = (posy - y) * (posy - y);
            if (dx_squared + dy_squared < stonesize_squared)
            {
                buf[window_width * posy + posx] += stoneweight;
            }
        }
    }
}

/**
 * \brief 计算下一帧波幅
 * \param window_width 窗口宽度
 * \param window_height 窗口高度
 * \param buf 当前波幅
 * \param buf2 下一帧波幅
 */
static void nextFrame(int window_width, int window_height, short **buf, short **buf2)
{
    for (int i = window_width; i < window_height * (window_width - 1); i++)
    {
        // 公式：X0'= (X1+X2+X3+X4) / 2 - X0
        (*buf2)[i] = (((*buf)[i - window_width] + (*buf)[i + window_width] + (*buf)[i - 1] + (*buf)[i + 1]) >> 1) - (*buf2)[i];

        // 波能衰减
        (*buf2)[i] -= (*buf2)[i] >> 5;
    }

    // short *temp = *buf;
    // *buf = *buf2;
    // *buf2 = temp;

    for (int i = window_width; i < window_height * (window_width - 1); i++)
    {
        // 公式：X0'= (X1+X2+X3+X4) / 2 - X0
        (*buf)[i] = (((*buf2)[i - window_width] + (*buf2)[i + window_width] + (*buf2)[i - 1] + (*buf2)[i + 1]) >> 1) - (*buf)[i];

        // 波能衰减
        (*buf)[i] -= (*buf)[i] >> 5;
    }
}

/**
 * \brief 渲染波纹
 * \param window_width 窗口宽度
 * \param window_height 窗口高度
 * \param buf 波幅数组
 * \param color_src 原图片内存指针
 * \param color_tar 处理后显示的位图内存指针
 */
static void RenderRipple(int window_width, int window_height, short *buf, SDL_Color *color_src, SDL_Color *color_tar)
{
    int i = 0;
    for (int y = 0; y < window_height; y++)
    {
        int offset_y = y - window_height / 2;
        for (int x = 0; x < window_width; x++)
        {
            short data = 1024 - buf[i];
            int offset_x = x - window_width / 2;

            int a = (offset_x * data / 1024) + window_width / 2;
            int b = (offset_y * data / 1024) + window_height / 2;

            if (a >= window_width)
                a = window_width - 1;
            if (a < 0)
                a = 0;
            if (b >= window_height)
                b = window_height - 1;
            if (b < 0)
                b = 0;

            color_tar[i] = color_src[a + (b * window_width)];
            i++;
        }
    }
}

/**
 * \brief gui_login界面
 */
unsigned int gui_login()
{
    int ret;

    unsigned int loginRet = LOGIN_ERROR; // 登陆结果

    Uint64 frameStart = 0, frameTime = 0, waveTime = 0, waveCount = 0; // 控制帧率

    SDL_FPoint mouse = {0.0f, 0.0f}; // 鼠标位置

    // 创建窗口
    SDL_Window *window = SDL_CreateWindow("ecnFace - login", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, SDL_WINDOW_SHOWN);
    CHECK(window != NULL, "创建窗口失败 : %s\n", SDL_GetError());

    // 创建渲染器
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    CHECK(renderer != NULL, "创建渲染器失败 : %s\n", SDL_GetError());

    // 设置渲染器
    ret = SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    CHECK(ret == 0, "设置渲染器错误 : %s\n", SDL_GetError());

    // 读取背景图片
    SDL_Surface *tmp = IMG_Load(LOGIN_IMAGE);
    if (!tmp)
        SDL_CreateRGBSurfaceWithFormat(0, WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, 32, SDL_PIXELFORMAT_RGBA32);

    // 转换图片格式
    SDL_Surface *surface = SDL_ConvertSurfaceFormat(tmp, SDL_PIXELFORMAT_RGBA32, 0);
    CHECK(surface != NULL, "转换图片格式失败 : %s\n", SDL_GetError());

    // 原图像数据
    SDL_Surface *surface_src = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_BlitScaled(surface, NULL, surface_src, NULL);

    // 新图像数据
    SDL_Surface *surface_tar = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_BlitSurface(surface, NULL, surface_tar, NULL);

    // 波幅数组
    SDL_Color *color_src = surface_src->pixels;                                                                                    // 原图片片内存指针
    SDL_Color *color_tar = surface_tar->pixels;                                                                                    // 处理后显示的位图内存指针
    short *buf = malloc((WINDOW_LOGIN_DEFAULT_HEIGHT * WINDOW_LOGIN_DEFAULT_WIDTH + WINDOW_LOGIN_DEFAULT_WIDTH) * sizeof(short));  // 当前波幅
    short *buf2 = malloc((WINDOW_LOGIN_DEFAULT_HEIGHT * WINDOW_LOGIN_DEFAULT_WIDTH + WINDOW_LOGIN_DEFAULT_WIDTH) * sizeof(short)); // 后一时刻波幅

    SDL_Texture *waveTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT);

    // 背景框
    SDL_FRect backRect = {WINDOW_LOGIN_DEFAULT_WIDTH / 2.0f - 300.0f, WINDOW_LOGIN_DEFAULT_HEIGHT / 2.0f - 150.0f,
                          600.0f, 300.0f};
    SDL_Texture *backTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, backRect.w, backRect.h);
    ebo backEBO = {0};

    // 渲染一个背景框
    {
        SDL_SetRenderTarget(renderer, backTexture);
        SDL_Vertex vertex[4] = {
            {.color = {0x74, 0xEB, 0xD5, 255},
             .position = {0, 0},
             .tex_coord = {0, 0}},
            {.color = {0xAC, 0xB6, 0xE5, 255},
             .position = {backRect.w, 0},
             .tex_coord = {0, 0}},
            {.color = {0xAC, 0xB6, 0xE5, 255},
             .position = {backRect.w, backRect.h},
             .tex_coord = {0, 0}},
            {.color = {0x74, 0xEB, 0xD5, 255},
             .position = {0, backRect.h},
             .tex_coord = {0, 0}},
        };
        int index[6] = {0, 1, 2, 0, 2, 3};
        // 渲染texture
        SDL_RenderGeometry(renderer, NULL, vertex, 4, index, 6);
        // 获取背景框的EBO
        getRoundedBorder(NULL, (int)backRect.w, (int)backRect.h, &backRect, 0.1f, &backEBO, NULL);
        // 恢复renderer
        SDL_SetRenderTarget(renderer, NULL);
        SDL_SetTextureColorMod(backTexture, 0x00, 0x00, 0x00);
    }

    // 设置输入框结构体
    int nameRender = 0; // 是否渲染输入内容
    guiText name = {0};
    gtInit(&name,
           (SDL_Rect){backRect.x + (backRect.w - 500) / 2, backRect.y + 80, 500, 40},
           (SDL_Rect){backRect.x + (backRect.w - 500) / 2, backRect.y + 80 + 2, 500, 40 - 4},
           (SDL_Color){0, 0, 0, 255});

    // 输入框
    SDL_Color textColor[2] = {
        {0x80, 0x80, 0x80, 0xFF},
        {0x00, 0x00, 0x00, 0xFF},
    };
    SDL_Rect textRect = {
        .x = name.rect.x,
        .y = name.rect.y + name.rect.h + 5,
        .w = name.rect.w,
        .h = 2,
    };

    // 请输入用户名文本渲染
    SDL_Surface *nameSurface = TTF_RenderUTF8_Blended(Global.font, "请输入用户名", (SDL_Color){0xA0, 0xA0, 0xA0, 0xFF});
    SDL_Texture *nameTexture = SDL_CreateTextureFromSurface(renderer, nameSurface);
    float scale = (float)name.textRect.h / (float)nameSurface->h;
    SDL_FRect nameRect = {
        .x = (float)name.textRect.x + (name.textRect.w - (float)nameSurface->w * scale) / 2.0f,
        .y = (float)name.textRect.y,
        .w = (float)nameSurface->w * scale,
        .h = (float)name.textRect.h,
    };
    SDL_FreeSurface(nameSurface);

    // 登陆按钮
    SDL_Surface *loginTextSurface = TTF_RenderUTF8_Blended(Global.font, "登陆", (SDL_Color){0x00, 0x00, 0x00, 0xFF});
    SDL_Texture *loginTextTexture = SDL_CreateTextureFromSurface(renderer, loginTextSurface);
    SDL_FRect loginButtonRect = {
        .x = backRect.x + (backRect.w - 400.0f) / 2.0f,
        .y = backRect.y + 200.0f,
        .w = 190.0f,
        .h = 50.0f,
    };
    float scale1 = 0.8f;                                                    // 高度所占比例
    float scale2 = loginButtonRect.h * scale1 / (float)loginTextSurface->h; // 实际缩放比例
    SDL_FRect loginTextRect = {
        .x = loginButtonRect.x + (loginButtonRect.w - ((float)loginTextSurface->w * scale2)) / 2.0f,
        .y = loginButtonRect.y + (loginButtonRect.h - ((float)loginTextSurface->h * scale2)) / 2.0f,
        .w = (float)loginTextSurface->w * scale2,
        .h = (float)loginTextSurface->h * scale2,
    };
    ebo loginEBOA = {0}; // 正常状态
    ebo loginEBOB = {0}; // 按下状态
    ebo *loginEBO = &loginEBOA;
    getRoundedBorder(NULL, loginButtonRect.w, loginButtonRect.h, &loginButtonRect,
                     0.2f, &loginEBOA,
                     &(SDL_Color){0x00, 0xBF, 0xFF, 0xFF});
    getRoundedBorder(NULL, loginButtonRect.w, loginButtonRect.h, &loginButtonRect,
                     0.2f, &loginEBOB,
                     &(SDL_Color){0x1E, 0x90, 0xFF, 0xFF});

    // 注册按钮
    SDL_Surface *regTextSurface = TTF_RenderUTF8_Blended(Global.font, "注册", (SDL_Color){0x00, 0x00, 0x00, 0xFF});
    SDL_Texture *regTextTexture = SDL_CreateTextureFromSurface(renderer, regTextSurface);
    SDL_FRect regButtonRect = {
        .x = loginButtonRect.x + loginButtonRect.w + 20.0f,
        .y = loginButtonRect.y,
        .w = loginButtonRect.w,
        .h = loginButtonRect.h,
    };
    scale1 = 0.8f;                                                // 高度所占比例
    scale2 = regButtonRect.h * scale1 / (float)regTextSurface->h; // 实际缩放比例
    SDL_FRect regTextRect = {
        .x = regButtonRect.x + (regButtonRect.w - ((float)regTextSurface->w * scale2)) / 2.0f,
        .y = regButtonRect.y + (regButtonRect.h - ((float)regTextSurface->h * scale2)) / 2.0f,
        .w = (float)regTextSurface->w * scale2,
        .h = (float)regTextSurface->h * scale2,
    };
    ebo regEBOA = {0}; // 正常状态
    ebo regEBOB = {0}; // 按下状态
    ebo *regEBO = &regEBOA;
    getRoundedBorder(NULL, regButtonRect.w, regButtonRect.h, &regButtonRect,
                     0.2f, &regEBOA,
                     &(SDL_Color){0xFF, 0xFF, 0xFF, 0xFF});
    getRoundedBorder(NULL, regButtonRect.w, regButtonRect.h, &regButtonRect,
                     0.2f, &regEBOB,
                     &(SDL_Color){0xC0, 0xC0, 0xC0, 0xFF});

    // 设置水波纹
    waveTime = SDL_GetTicks64();

    // 动画时间
    int time1 = 0, time2 = 0;

    // 设置线程
    setThread(false);

    // 渲染
    Global.quit = 1;
    // while (getThread() == false && Global.quit)
    while (Global.quit)
    {
        // 事件处理
        SDL_Event event = {0};

        // 设置帧开始时间
        frameStart = SDL_GetTicks64();
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                Global.quit = 0;
                break;

            case SDL_MOUSEMOTION: // 移动事件

                // 更新鼠标位置
                mouse.x = event.motion.x;
                mouse.y = event.motion.y;

                // 按下状态移动
                if (event.motion.state == SDL_BUTTON_LMASK)
                {
                    // 在背景框内按下移动
                    if (SDL_PointInFRect(&mouse, &backRect) == SDL_TRUE)
                    {
                    }
                    // 在背景框外按下移动
                    else
                    {
                        disturb(WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, event.motion.x, event.motion.y, 4, 128, buf);
                    }
                }
                break;

            case SDL_MOUSEBUTTONDOWN:                       // 按下事件
                if (event.button.button == SDL_BUTTON_LEFT) // 左键按下
                {
                    // 在背景框内按下
                    if (SDL_PointInFRect(&mouse, &backRect) == SDL_TRUE)
                    {
                        SDL_FRect tmpRect = {
                            .x = (float)name.rect.x,
                            .y = (float)name.rect.y,
                            .w = (float)name.rect.w,
                            .h = (float)name.rect.h};

                        // 按下登陆按钮
                        if (SDL_PointInFRect(&mouse, &loginButtonRect) == SDL_TRUE &&
                            getThread() == false)
                        {
                            // 设置按下状态
                            SET_FLAG(login_button);

                            // 切换EBO
                            loginEBO = &loginEBOB;
                        }
                        // 按下注册按钮
                        else if (SDL_PointInFRect(&mouse, &regButtonRect) == SDL_TRUE &&
                                 getThread() == false)
                        {
                            // 设置按下状态
                            SET_FLAG(reg_button);

                            // 切换EBO
                            regEBO = &regEBOB;
                        }

                        // 在背景框内按下并且没有在处理文本框
                        if (SDL_PointInFRect(&mouse, &tmpRect) == SDL_TRUE &&
                            name.isRun == false)
                        {
                            SET_FLAG(text_Click);
                        }
                        // 在背景框外按下并且正在处理文本框
                        else if (SDL_PointInFRect(&mouse, &tmpRect) == SDL_FALSE &&
                                 name.isRun == true)
                        {
                            nameRender = strlen(name.textIn);
                            SET_FLAG(text_Click_Not);
                        }
                    }
                    // 在背景框外按下
                    else
                    {
                        // 创建水波纹
                        disturb(WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, event.button.x, event.button.y, 5, 2560, buf);
                    }
                }
                break;
            // 弹起事件
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) // 左键弹起
                {
                    // 如果按下登陆按钮
                    if (CHECK_FLAG(login_button))
                    {
                        // 清除按下状态
                        CLEAR_FLAG(login_button);

                        // 切换EBO
                        loginEBO = &loginEBOA;

                        // 检测是否在按钮内
                        if (SDL_PointInFRect(&mouse, &loginButtonRect) == SDL_TRUE)
                        {
                            DEBUG("登陆\n");

                            // 登陆
                            setThread(true);

                            // 创建线程
                            strcpy(Global.name, name.textIn);
                            pthread_create(&Global.thread, NULL, login, &loginRet);
                        }
                    }

                    // 如果按下注册按钮
                    else if (CHECK_FLAG(reg_button))
                    {
                        // 清除按下状态
                        CLEAR_FLAG(reg_button);

                        // 切换EBO
                        regEBO = &regEBOA;

                        // 检测是否在按钮内
                        if (SDL_PointInFRect(&mouse, &regButtonRect) == SDL_TRUE)
                        {
                            DEBUG("注册\n");

                            // 注册
                            setThread(true);

                            // 创建线程
                            strcpy(Global.name, name.textIn);
                            pthread_create(&Global.thread, NULL, reg, &loginRet);
                            // reg(&loginRet);
                        }
                    }
                }
                break;
            }
            gtEvent(&name, &event, renderer);
        }

        // 清空屏幕
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // 创建波纹
        if (SDL_GetTicks64() - waveTime > waveCount * 600)
        {
            disturb(WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT,
                    rand() % WINDOW_LOGIN_DEFAULT_WIDTH, rand() % WINDOW_LOGIN_DEFAULT_HEIGHT,
                    5, 128 + rand() % 12800,
                    buf);
            waveCount++;
        }
        /* 渲染动态水波纹背景 */
        // 计算下一帧波幅
        nextFrame(WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, &buf, &buf2);
        // 渲染波纹
        RenderRipple(WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, buf, color_src, color_tar);
        // 更新背景
        SDL_LockTexture(waveTexture, NULL, (void **)&color_tar, &surface->pitch);
        SDL_UpdateTexture(waveTexture, NULL, color_tar, surface->pitch);
        SDL_UnlockTexture(waveTexture);
        // 渲染背景
        SDL_RenderCopy(renderer, waveTexture, NULL, NULL);

        // 渲染背景框
        drawFromEBO(renderer, backTexture, backEBO);

        // 渲染文本框
        if (CHECK_FLAG(text_Click))
        {
            //  绘制初始化选中按钮框
            if (!CHECK_FLAG(text_Click_Start))
            {
                // 标志为已初始化
                SET_FLAG(text_Click_Start);

                // 更新时间
                time1 = SDL_GetTicks64();
            }

            // 获取间隔时间
            time2 = SDL_GetTicks64() - time1;

            // 绘制结束
            if (time2 > 200)
            {
                CLEAR_FLAG(text_Click);
                CLEAR_FLAG(text_Click_Start);

                SDL_SetRenderDrawColor(renderer, textColor[1].r, textColor[1].g, textColor[1].b, textColor[1].a);
                SDL_RenderDrawRect(renderer, &textRect);
            }
            // 绘制动画
            else
            {
                float scale = (float)time2 / 200.0f;
                SDL_Rect tmpRect = {0};
                tmpRect.w = textRect.w * scale;
                tmpRect.h = textRect.h;
                tmpRect.x = textRect.x + (int)((float)textRect.w - (float)tmpRect.w) / 2.0f;
                tmpRect.y = textRect.y;
                SDL_SetRenderDrawColor(renderer, textColor[0].r, textColor[0].g, textColor[0].b, textColor[0].a);
                SDL_RenderDrawRect(renderer, &textRect);
                SDL_SetRenderDrawColor(renderer, textColor[1].r, textColor[1].g, textColor[1].b, textColor[1].a);
                SDL_RenderDrawRect(renderer, &tmpRect);
            }
        }
        else if (CHECK_FLAG(text_Click_Not))
        {
            //  绘制初始化选中按钮框
            if (!CHECK_FLAG(text_Click_Not_Start))
            {
                // 标志为已初始化
                SET_FLAG(text_Click_Not_Start);

                // 更新时间
                time1 = SDL_GetTicks64();
            }

            // 获取间隔时间
            time2 = SDL_GetTicks64() - time1;

            // 绘制结束
            if (time2 > 200)
            {
                CLEAR_FLAG(text_Click_Not);
                CLEAR_FLAG(text_Click_Not_Start);

                SDL_SetRenderDrawColor(renderer, textColor[0].r, textColor[0].g, textColor[0].b, textColor[0].a);
                SDL_RenderDrawRect(renderer, &textRect);
            }
            // 绘制动画
            else
            {
                float scale = (float)time2 / 200.0f;

                SDL_SetRenderDrawColor(renderer,
                                       textColor[1].r + (int)(((float)textColor[0].r - (float)textColor[1].r) * scale),
                                       textColor[1].g + (int)(((float)textColor[0].g - (float)textColor[1].g) * scale),
                                       textColor[1].b + (int)(((float)textColor[0].b - (float)textColor[1].b) * scale),
                                       textColor[0].a);
                SDL_RenderDrawRect(renderer, &textRect);
            }
        }
        else
        {
            if (name.isRun == true)
                SDL_SetRenderDrawColor(renderer, textColor[1].r, textColor[1].g, textColor[1].b, textColor[1].a);
            else
                SDL_SetRenderDrawColor(renderer, textColor[0].r, textColor[0].g, textColor[0].b, textColor[0].a);
            SDL_RenderDrawRect(renderer, &textRect);
        }

        // 渲染文本
        if (nameRender || name.isRun == true)
            gtRender(&name, renderer);
        // 渲染提示文本
        else
            SDL_RenderCopyF(renderer, nameTexture, NULL, &nameRect);

        // 绘制按钮
        drawFromEBO(renderer, NULL, *loginEBO);
        SDL_RenderCopyF(renderer, loginTextTexture, NULL, &loginTextRect);
        drawFromEBO(renderer, NULL, *regEBO);
        SDL_RenderCopyF(renderer, regTextTexture, NULL, &regTextRect);

        // 更新Renderer
        SDL_RenderPresent(renderer);
        // 控制刷新率
        frameTime = SDL_GetTicks64() - frameStart;
        if (frameTime < FPS_MS)
            SDL_Delay(FPS_MS - frameTime);
    }

error:

    // 释放资源
    SDL_FreeSurface(loginTextSurface);
    SDL_DestroyTexture(loginTextTexture);
    freeEBO(&loginEBOA);
    freeEBO(&loginEBOB);

    SDL_FreeSurface(regTextSurface);
    SDL_DestroyTexture(regTextTexture);
    freeEBO(&regEBOA);
    freeEBO(&regEBOB);

    SDL_DestroyTexture(nameTexture);

    gtFree(&name);

    freeEBO(&backEBO);
    SDL_DestroyTexture(backTexture);

    free(buf);
    free(buf2);
    SDL_DestroyTexture(waveTexture);
    SDL_FreeSurface(surface_tar);
    SDL_FreeSurface(surface_src);
    SDL_FreeSurface(surface);
    SDL_FreeSurface(tmp);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return loginRet;
}

void *login(void *arg)
{
    int ret;
    uint8_t key[ZUC_KEY_SIZE] = {0};
    uint8_t iv[ZUC_KEY_SIZE] = {0};

    // 获取用户信息
    // ret = userGet(USER_CONFIG, Global.name, &Global.SM2user, key, iv);
    // CHECK(ret == true, "未在本地检测到该用户");

    // // 计算出ZUC密钥
    // zuc_init(&Global.ZUCstate, key, iv);

    // 与在服务端登陆


    // 成功登陆
    Global.quit = 0;
    *(unsigned int *)arg = LOGIN_SUCCESS;
    
error:
    setThread(false);
    return NULL;
}

void *reg(void *arg)
{
    int ret;
    uint8_t key[ZUC_KEY_SIZE] = {0};
    uint8_t iv[ZUC_KEY_SIZE] = {0};
    
    // 向服务器请求注册
    memset(&Global.SM2user, 0, sizeof(SM2_KEY));
    ret = registerUser(Global.name, &Global.SM2server, &Global.SM2user);
    CHECK(ret == true);

    // 随机生成ZUC密钥和初始化向量
    zucKeyVi(key, iv);

    // 写入配置文件
    ret = userAdd(USER_CONFIG, Global.name, &Global.SM2user, key, iv);
    CHECK(ret == true, "写入配置文件失败\n");

    DEBUG("注册成功\n");

error:
    setThread(false);
}