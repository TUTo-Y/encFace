#include "main.h"

#undef main
int main(int argc, char *argv[])
{
    WSADATA wsaData; // winsocket数据
    int ret;

    /************* 初始化设置 *************/
    srand(time(NULL));

    // 设置调试控制台支持字符
    DEB(SetConsoleOutputCP(CP_UTF8));

    // 初始化线程锁
    pthread_mutex_init(&Global.lock, NULL);

    // 初始化wsaData
    ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    CHECK(ret == 0, "无法初始化winsocket : %d\n", WSAGetLastError());

    // 初始化与远程服务器的连接
    DEBUG("正在连接到远程服务器...\n");
    ret = connectServer();
    CHECK(ret == true, "无法连接到远程服务器\n");

    // 启动facenet服务器并与facenet服务器的连接
    DEBUG("正在连接到facenet服务器...\n");
    ret = connectFaceNet();
    CHECK(ret == true, "无法连接到facenet服务器\n");

    // 读取sm9主公钥
    DEBUG("加载sm9主公钥...\n");
    FILE *fp = fopen("master_public_key.pem", "rb");
    CHECK(fp, "无法打开master_public_key.pem\n");
    sm9_enc_master_public_key_from_pem(&Global.SM9master, fp);
    fclose(fp);

    // 初始化SDL
    DEBUG("初始化SDL...\n");
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_AVIF | IMG_INIT_JXL);
    TTF_Init();

    // 加载字体
    Global.font = TTF_OpenFont(TTF_PATH, 24);
    CHECK(Global.font != NULL, "TTF_OpenFont: %s\n", TTF_GetError());

    // 创建窗口
    Global.window = SDL_CreateWindow("encFace", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    CHECK(Global.window != NULL, "创建窗口失败 : %s\n", SDL_GetError());

    // 设置窗口的最小大小
    SDL_SetWindowMinimumSize(Global.window, 400, 400);

    // 创建渲染器
    Global.renderer = SDL_CreateRenderer(Global.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    CHECK(Global.renderer != NULL, "创建渲染器失败 : %s\n", SDL_GetError());

    // 设置渲染器
    ret = SDL_SetRenderDrawBlendMode(Global.renderer, SDL_BLENDMODE_BLEND);
    CHECK(ret == 0, "设置渲染器错误 : %s\n", SDL_GetError());

    // 初始化窗口全局数据
    SDL_GetWindowPosition(Global.window, &Global.windowRect.x, &Global.windowRect.y);
    Global.windowRect.w = WINDOW_DEFAULT_WIDTH;
    Global.windowRect.h = WINDOW_DEFAULT_HEIGHT;

    // 设置按钮区域
    resetButton();

    // core gui
    play();

error:
    SDL_DestroyRenderer(Global.renderer);
    SDL_DestroyWindow(Global.window);
    TTF_CloseFont(Global.font);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    // 杀死线程
    if (pthread_kill(Global.thread, 0) != ESRCH)
    {
        pthread_kill(Global.thread, SIGTERM);
    }

    // 销毁线程锁
    pthread_mutex_destroy(&Global.lock);

    // 释放资源
    if (CHECK_FLAG(image_is_choice))
    {
        listFree(&Global.face, (void (*)(void *))freeVector);
        listFree(&Global.faceSurface, (void (*)(void *))SDL_FreeSurface);
        listFree(&Global.faceTexture, (void (*)(void *))SDL_DestroyTexture);
        SDL_FreeSurface(Global.surface);
        SDL_DestroyTexture(Global.texture);
    }

    // 关闭与facenet服务器的连接
    closeFaceNet();

    // 关闭与远程服务器的连接
    closeServer();

    // 释放wsaData
    WSACleanup();

    // 退出
    DEB(system("pause"));
    return 0;
}

/**
 * \brief 调用WindowsAPI选择图片
 */
bool selectImageFile(wchar_t *path, size_t size)
{
    OPENFILENAMEW ofn;
    HANDLE hf;

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = path;
    ofn.lpstrFile[0] = L'\0';
    ofn.nMaxFile = size;
    ofn.lpstrFilter = L"Image\0*.BMP;*.GIF;*.JPG;*.JPEG;*.LBM;*.PCX;*.PNG;*.PNM;*.SVG;*.TGA;*.TIFF;*.WEBP;*.XCF;*.XPM;*.XV\0\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn) == TRUE)
    {
        // 检查文件是否存在
        hf = CreateFileW(ofn.lpstrFile,
                         GENERIC_READ,
                         0,
                         (LPSECURITY_ATTRIBUTES)NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         (HANDLE)NULL);
        // 文件存在
        if (hf != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hf);
            return true;
        }
    }
    return false;
}

/**
 * \brief 加载人脸
 */
bool choiceImage()
{
    int ret = 0;

    // 初始化
    memset(Global.path, 0, sizeof(Global.path));

    // 选择图片
    ret = selectImageFile(Global.path, MAX_PATH);
    CHECK(ret == true, "未选择图片\n");

    // 清除原有数据
    if (CHECK_FLAG(image_is_choice))
    {
        // 释放资源
        listFree(&Global.face, (void (*)(void *))freeVector);
        listFree(&Global.faceSurface, (void (*)(void *))SDL_FreeSurface);
        listFree(&Global.faceTexture, (void (*)(void *))SDL_DestroyTexture);
        SDL_FreeSurface(Global.surface);
        SDL_DestroyTexture(Global.texture);

        // 清除标志
        CLEAR_FLAG(image_is_choice);
    }

    // 使用_wfopen函数打开文件
    FILE *fp = _wfopen(Global.path, L"rb");
    CHECK(fp, "打开图片失败 : %s\n", Global.path);

    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // 读取整个文件到内存
    char *buffer = malloc(fsize + 1);
    fread(buffer, fsize, 1, fp);
    fclose(fp);

    // 读取图片Surface
    Global.surface = IMG_Load_RW(SDL_RWFromMem(buffer, fsize), 1);
    free(buffer);
    CHECK(Global.surface, "读取图片失败 : %s\n", IMG_GetError());

    // 转化图片格式到四通道
    SDL_Surface *tmpSurface = SDL_ConvertSurfaceFormat(Global.surface, SDL_PIXELFORMAT_RGBA32, 0);
    CHECK(tmpSurface, "转换图片格式失败 : %s\n", SDL_GetError());
    SDL_FreeSurface(Global.surface);
    Global.surface = tmpSurface;

    // 设置Surface为圆角
    setSurfaceRoundedBorder(Global.surface, 20, (SDL_Color){0xff, 0xff, 0xff, 255});

    // 读取图片Texture
    Global.texture = SDL_CreateTextureFromSurface(Global.renderer, Global.surface);
    CHECK(Global.texture, "创建图片纹理失败 : %s\n", SDL_GetError());

    // 调整图片大小
    resizeImage(&Global.windowRect, &Global.surfaceRect, Global.surface->w, Global.surface->h);
    Global.scale = (float)Global.surfaceRect.w / (float)Global.surface->w;
    Global.scale2 = 1.0f;

    // 创建线程
    setTh(true);
    ret = pthread_create(&Global.thread, NULL, th, Global.path);
    CHECK(ret == 0, "创建线程失败\n");

    // 设置已选择标志
    SET_FLAG(image_is_choice);
    return true;
error:
    return false;
}

/**
 * \brief 使用多线程加载数据
 */
void *th(void *arg)
{
    int ret = 0;

    // 获取人脸特征向量
    ret = getFaceVector((wchar_t *)arg, &Global.face);
    CHECK(ret == true, "获取人脸特征向量失败\n");

    // 如果检测到人脸
    if (0 < listLen(Global.face))
    {
        // 获取人脸信息
        ret = getFaceInfo(Global.face);
        CHECK(ret == true, "获取人脸信息失败\n");
    }

    DEB(else { DEBUG("未检测到人脸\n"); });

error:

    // 设置线程结束标志
    setTh(false);
    return NULL;
}

/**
 * \brief 访问线程使用标志
 */
bool getTh()
{
    bool ret;

    pthread_mutex_lock(&Global.lock);
    ret = Global.th;
    pthread_mutex_unlock(&Global.lock);
    return ret;
}

/**
 * \brief 设置线程使用标志
 */
void setTh(bool flag)
{
    pthread_mutex_lock(&Global.lock);
    Global.th = flag;
    pthread_mutex_unlock(&Global.lock);
}
