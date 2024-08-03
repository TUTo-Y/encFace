#include "gui_login.h"

// 登陆
static void *login(void *arg);
// 注册
static void *reg(void *arg);

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

    Uint64 frameStart = 0, frameTime = 0; // 控制帧率

    SDL_FPoint mouse = {0.0f, 0.0f}; // 鼠标位置

    /* 创建窗口和渲染器 */
    SDL_Window *window = SDL_CreateWindow("ecnFace - login", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    /* 加载背景图片和初始化动态水波纹 */
    Uint64 waveTime = 0, waveCount = 0;
    SDL_Surface *tmp = IMG_Load(LOGIN_IMAGE);
    if (!tmp) // 若图片不存在
        tmp = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_Surface *surface = SDL_ConvertSurfaceFormat(tmp, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(tmp);
    CHECK(surface != NULL, "转换图片格式失败 : %s\n", SDL_GetError());
    // 创建两个相同的表面
    SDL_Surface *surface_src = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_Surface *surface_tar = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_BlitScaled(surface, NULL, surface_src, NULL);
    SDL_BlitSurface(surface, NULL, surface_tar, NULL);
    // 波幅数组
    SDL_Color *color_src = surface_src->pixels;                                                                                    // 原图片片内存指针
    SDL_Color *color_tar = surface_tar->pixels;                                                                                    // 处理后显示的位图内存指针
    short *buf = malloc((WINDOW_LOGIN_DEFAULT_HEIGHT * WINDOW_LOGIN_DEFAULT_WIDTH + WINDOW_LOGIN_DEFAULT_WIDTH) * sizeof(short));  // 当前波幅
    short *buf2 = malloc((WINDOW_LOGIN_DEFAULT_HEIGHT * WINDOW_LOGIN_DEFAULT_WIDTH + WINDOW_LOGIN_DEFAULT_WIDTH) * sizeof(short)); // 后一时刻波幅
    // 波纹纹理
    SDL_Texture *waveTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT);

    /* 背景框 */
    SDL_FRect backRect = {WINDOW_LOGIN_DEFAULT_WIDTH / 2.0f - 300.0f, WINDOW_LOGIN_DEFAULT_HEIGHT / 2.0f - 150.0f,
                          600.0f, 300.0f};
    SDL_Texture *backTexture = drawRoundRect(backRect.w, backRect.h,
                                             (SDL_Color[]){(SDL_Color){0x74, 0xEB, 0xD5, 0xFF},
                                                           (SDL_Color){0xAC, 0xB6, 0xE5, 0xFF},
                                                           (SDL_Color){0xAC, 0xB6, 0xE5, 0xFF},
                                                           (SDL_Color){0x74, 0xEB, 0xD5, 0xFF}},
                                             0.1f, renderer, NULL);

    /* 消息框 */
    guiMsg msg = {0};
    gmInit(&msg,
           Global.font,
           NULL,
           NULL,
           NULL,
           NULL,
           NULL,
           NULL,
           (SDL_Rect){20, WINDOW_LOGIN_DEFAULT_HEIGHT - 30 - 20, 250, 30},
           0.7,
           10,
           3000,
           1000,
           -1,
           renderer);

    /* 输入框 */
    guiText name = {0};
    gtInit(&name,
           (SDL_Rect){backRect.x + (backRect.w - 500) / 2, backRect.y + 60, 500, 60},
           0.6,
           2,
           Global.font,
           "请输入用户名",
           (SDL_Color){0xA0, 0xA0, 0xA0, 0xFF},
           (SDL_Color){0x00, 0x00, 0x00, 0xff},
           (SDL_Color){0x80, 0x80, 0x85, 0xFF},
           (SDL_Color){0x10, 0x10, 0x10, 0xFF},
           200, 200,
           renderer);
    gtSetCenter(&name, true);

    /* 登陆按钮 */
    guiButton loginButton = {0};
    gbInit(&loginButton,
           &(SDL_Rect){.x = backRect.x + (backRect.w - 400.0f) / 2.0f,
                       .y = backRect.y + 200.0f,
                       .w = 190.0f,
                       .h = 50.0f},
           "登录",
           Global.font,
           &(SDL_Color){0x00, 0x00, 0x00, 0xFF},
           0.8f,
           0.2f,
           &(SDL_Color[3][4])
           // 一般状态
           {{{0xB2, 0xFE, 0xFA, 0xFF}, {0x0E, 0xD2, 0xF7, 0xFF}, {0x0E, 0xD2, 0xF7, 0xFF}, {0xB2, 0xFE, 0xFA, 0xFF}},
            // 选中状态
            {{0x56, 0xCC, 0xF2, 0xFF}, {0x2F, 0x80, 0xED, 0xFF}, {0x2F, 0x80, 0xED, 0xFF}, {0x56, 0xCC, 0xF2, 0xFF}},
            // 按下状态
            {{0x40, 0xC0, 0xFF, 0xFF}, {0x2C, 0x70, 0xF0, 0xFF}, {0x2C, 0x70, 0xF0, 0xFF}, {0x40, 0xC0, 0xFF, 0xFF}}},
           NULL,
           renderer);


    /* 注册按钮 */
    guiButton regButton = {0};
    gbInit(&regButton,
           &(SDL_Rect){
               .x = loginButton.Rect.x + loginButton.Rect.w + 20,
               .y = loginButton.Rect.y,
               .w = loginButton.Rect.w,
               .h = loginButton.Rect.h},
           "注册",
           Global.font,
           &(SDL_Color){0x00, 0x00, 0x00, 0xFF},
           0.8f,
           0.2f,
           &(SDL_Color[3][4])
           // 一般状态
           {{{0xEC, 0xE9, 0xE6, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF}, {0xEC, 0xE9, 0xE6, 0xFF}},
            // 选中状态
            {{0xE0, 0xE0, 0xE0, 0xFF}, {0xE0, 0xE0, 0xE0, 0xFF}, {0xE0, 0xE0, 0xE0, 0xFF}, {0xE0, 0xE0, 0xE0, 0xFF}},
            // 按下状态
            {{0xC0, 0xC0, 0xC0, 0xFF}, {0xC0, 0xC0, 0xC0, 0xFF}, {0xC0, 0xC0, 0xC0, 0xFF}, {0xC0, 0xC0, 0xC0, 0xFF}}},
           NULL,
           renderer);

    // 设置水波纹
    waveTime = SDL_GetTicks64();

    // 设置线程
    setThread(false);

    /**
     * 全局退出标识由线程设置
     * 线程退出标识由消息循环设置
     */
    int quit_th = 1;  // 线程退出标识
    int quit_gui = 1; // GUI退出标识
    while (getThread() == true ? quit_gui : quit_th && quit_gui)
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
                quit_gui = 0;
                break;

            case SDL_MOUSEMOTION: // 移动事件

                // 更新鼠标位置
                mouse.x = event.motion.x;
                mouse.y = event.motion.y;

                // 按下状态移动
                if (event.motion.state == SDL_BUTTON_LMASK)
                {
                    // 在背景框外按下移动
                    if (SDL_FALSE == SDL_PointInFRect(&mouse, &backRect))
                    {
                        disturb(WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, event.motion.x, event.motion.y, 4, 128, buf);
                    }
                }
                break;

            case SDL_MOUSEBUTTONDOWN:                       // 按下事件
                if (event.button.button == SDL_BUTTON_LEFT) // 左键按下
                {
                    // 在背景框外按下
                    if (SDL_FALSE == SDL_PointInFRect(&mouse, &backRect))
                    {
                        // 创建水波纹
                        disturb(WINDOW_LOGIN_DEFAULT_WIDTH, WINDOW_LOGIN_DEFAULT_HEIGHT, event.button.x, event.button.y, 5, 2560, buf);
                    }
                }
                break;
            }

            // 输入框事件
            gtEvent(&name, &event);

            // 确认按钮
            if (gbEvent(&loginButton, &event) &&
                strlen(name.textIn) > 0 && strlen(name.textIn) < USER_LEN &&
                getThread() == false)
            {
                // 登陆
                setThread(true);

                // 设置参数
                loginThreadParam *arg = malloc(sizeof(loginThreadParam));
                strncpy(arg->name, name.textIn, sizeof(arg->name));
                arg->quit = &quit_th;
                arg->ret = &loginRet;
                arg->msg = &msg;

                // 创建线程
                pthread_create(&Global.thread, NULL, login, arg);
            }

            // 注册按钮
            if (gbEvent(&regButton, &event) &&
                strlen(name.textIn) > 0 && strlen(name.textIn) < USER_LEN &&
                getThread() == false)
            {
                // 注册
                setThread(true);

                // 设置参数
                regThreadParam *arg = malloc(sizeof(regThreadParam));
                strncpy(arg->name, name.textIn, sizeof(arg->name));
                arg->msg = &msg;

                // 创建线程
                pthread_create(&Global.thread, NULL, reg, arg);
            }
        }

        // 清空屏幕
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        /* 创建新的波纹点 */
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
        // 渲染水波纹
        SDL_RenderCopy(renderer, waveTexture, NULL, NULL);

        /* 渲染背景框 */
        SDL_RenderCopyF(renderer, backTexture, NULL, &backRect);

        /* 渲染文本框 */
        gtRender(&name);

        /* 渲染按钮 */
        gbRender(&loginButton);
        gbRender(&regButton);

        /* 渲染消息框 */
        gmRender(&msg);

        // 更新Renderer
        SDL_RenderPresent(renderer);

        // 控制刷新率
        frameTime = SDL_GetTicks64() - frameStart;
        if (frameTime < FPS_MS)
            SDL_Delay(FPS_MS - frameTime);
    }

    // 释放资源
    gmDestroy(&msg);
    gbDestroy(&loginButton);
    gbDestroy(&regButton);
    gtDestroy(&name);

    SDL_DestroyTexture(backTexture);

    free(buf);
    free(buf2);
    SDL_DestroyTexture(waveTexture);
    SDL_FreeSurface(surface_tar);
    SDL_FreeSurface(surface_src);
    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return loginRet;

error:
    return LOGIN_ERROR;
}

static void *login(void *arg)
{
    // 设置传递的参数
    int *quit = ((loginThreadParam *)arg)->quit;
    unsigned int *loginRet = ((loginThreadParam *)arg)->ret;
    guiMsg *msg = ((loginThreadParam *)arg)->msg;
    char name[USER_LEN] = {0};
    strncpy(name, ((loginThreadParam *)arg)->name, sizeof(name));
    free(arg);

    int ret;
    uint8_t key[ZUC_KEY_SIZE] = {0};
    uint8_t iv[ZUC_KEY_SIZE] = {0};

    // 获取用户信息
    ret = userGet(USER_CONFIG, name, &Global.SM2user, key, iv);
    if (ret == false)
        gmAdd(msg, "未在检测到该用户", guiMsgEnum_Error);
    CHECK(ret == true, "未在检测到该用户");

    // 计算出ZUC密钥
    zuc_init(&Global.ZUCstate, key, iv);

    // 与在服务端登陆
    ret = loginUser(name, &Global.SM2server, &Global.SM2user);
    CHECK(ret == 0, "登陆失败\n");
    
    {
        // test
        personal p = {0};
        p.vector = Malloc(500);
        strcpy(p.info.name, "TUTo");
        uploadFaceInfo(&p);
    }

    // 成功登陆
    *quit = 0;                 // 设置界面退出
    *loginRet = LOGIN_SUCCESS; // 设置登陆结果

    gmAdd(msg, "登陆成功", guiMsgEnum_Success);

    setThread(false);
    return NULL;

error:
    if (ret == 2)
        gmAdd(msg, "用户不存在", guiMsgEnum_Error);

    gmAdd(msg, "登陆失败", guiMsgEnum_Error);

    setThread(false);
    return NULL;
}

static void *reg(void *arg)
{
    // 设置传递的参数
    guiMsg *msg = ((regThreadParam *)arg)->msg;
    char name[USER_LEN] = {0};
    strncpy(name, ((regThreadParam *)arg)->name, sizeof(name));
    free(arg);

    int ret;
    SM2_KEY SM2user = {0};
    uint8_t key[ZUC_KEY_SIZE] = {0};
    uint8_t iv[ZUC_KEY_SIZE] = {0};

    // 向服务器请求注册
    ret = registerUser(name, &Global.SM2server, &SM2user);
    CHECK(ret == 0);

    // 随机生成ZUC密钥和初始化向量
    zucKeyVi(key, iv);

    // 写入配置文件
    ret = userAdd(USER_CONFIG, name, &SM2user, key, iv);
    CHECK(ret == true, "写入配置文件失败\n");

    gmAdd(msg, "注册成功", guiMsgEnum_Success);

    setThread(false);
    return NULL;

error:

    if (ret == 2)
        gmAdd(msg, "用户已存在", guiMsgEnum_Error);
    gmAdd(msg, "注册失败", guiMsgEnum_Error);

    setThread(false);
    return NULL;
}