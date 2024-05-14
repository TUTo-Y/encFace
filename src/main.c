#include "main.h"

#undef main
int main(int argc, char *argv[])
{
    WSADATA wsaData; // winsocket数据
    int ret;         // 返回值

    /************* 初始化设置 *************/
    srand(time(NULL));

    // 设置调试控制台支持字符
    DEB(SetConsoleOutputCP(CP_UTF8));

    // 初始化wsaData
    ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    CHECK(ret == 0, "无法初始化winsocket : %d\n", WSAGetLastError());

    // 初始化与远程服务器的连接
    DEBUG("正在连接到远程服务器\n");
    ret = connectServer();
    CHECK(ret == true, "无法连接到远程服务器\n");

    // 启动facenet服务器并与facenet服务器的连接
    DEBUG("正在连接到facenet服务器\n");
    ret = connectFaceNet();
    CHECK(ret == true, "无法连接到facenet服务器\n");

    // 读取sm9主公钥
    FILE *fp = fopen("master_public_key.pem", "rb");
    CHECK(fp, "无法打开master_public_key.pem\n");
    sm9_enc_master_public_key_from_pem(&Global.SM9master, fp);
    fclose(fp);

    // 初始化SDL
    DEBUG("正在初始化SDL\n");
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

    // 初始化窗口全局数据
    SDL_GetWindowPosition(Global.window, &Global.windowRect.x, &Global.windowRect.y);
    Global.windowRect.w = WINDOW_DEFAULT_WIDTH;
    Global.windowRect.h = WINDOW_DEFAULT_HEIGHT;

    // 设置按钮区域
    Global.buttonRect.x = Global.windowRect.w * 0.2f;
    Global.buttonRect.y = Global.windowRect.h * 0.7f;
    Global.buttonRect.w = Global.windowRect.w * 0.6f;
    Global.buttonRect.h = Global.windowRect.h * 0.15f;
    Global.buttonMsgWRect.w = 50.0f;
    Global.buttonMsgWRect.h = 10.0f;
    Global.buttonMsgWRect.x = Global.buttonRect.x + (Global.buttonRect.w - Global.buttonMsgWRect.w) / 2.0f;
    Global.buttonMsgWRect.y = Global.buttonRect.y + (Global.buttonRect.h - Global.buttonMsgWRect.h) / 2.0f;
    Global.buttonMsgHRect.w = 10.0f;
    Global.buttonMsgHRect.h = 50.0f;
    Global.buttonMsgHRect.x = Global.buttonRect.x + (Global.buttonRect.w - Global.buttonMsgHRect.w) / 2.0f;
    Global.buttonMsgHRect.y = Global.buttonRect.y + (Global.buttonRect.h - Global.buttonMsgHRect.h) / 2.0f;

    // core gui
    play();

error:
    // 释放资源
    if (CHECK_FLAG(image_is_choice))
    {
        listFree(&Global.face, (void (*)(void *))freeVector);
        listFree(&Global.faceSurface, (void (*)(void *))SDL_FreeSurface);
        listFree(&Global.faceTexture, (void (*)(void *))SDL_DestroyTexture);
        SDL_FreeSurface(Global.surface);
        SDL_DestroyTexture(Global.texture);
    }
    SDL_DestroyRenderer(Global.renderer);
    SDL_DestroyWindow(Global.window);
    TTF_CloseFont(Global.font);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

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
bool selectImage(wchar_t *path, size_t size)
{
    OPENFILENAMEW ofn;
    HWND hwnd = NULL;
    HANDLE hf;

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
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
bool loadFace()
{
    wchar_t path[MAX_PATH] = {0};
    int ret = 0;
    char *buffer = NULL;

    // 选择图片
    ret = selectImage(path, MAX_PATH);
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
    FILE *fp = _wfopen(path, L"rb");
    CHECK(fp, "打开图片失败 : %s\n", path);

    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // 读取整个文件到内存
    buffer = malloc(fsize + 1);
    fread(buffer, fsize, 1, fp);
    fclose(fp);

    // 读取图片Surface
    Global.surface = IMG_Load_RW(SDL_RWFromMem(buffer, fsize), 1); // 第二个参数为1表示在读取后关闭SDL_RWops
    free(buffer);
    CHECK(Global.surface, "读取图片失败 : %s\n", IMG_GetError());

    // 读取图片Texture
    Global.texture = SDL_CreateTextureFromSurface(Global.renderer, Global.surface);
    CHECK(Global.texture, "创建图片纹理失败 : %s\n", SDL_GetError());

    // 设置区域大小
    resizeImage(&Global.windowRect, &Global.surfaceRect, Global.surface->w, Global.surface->h);
    Global.scale = (float)Global.surfaceRect.w / (float)Global.surface->w;
    Global.scale2 = 1.0f;

    // 获取人脸特征向量
    ret = getFaceVector(path, &Global.face);
    CHECK(ret == true, "获取人脸特征向量失败\n");

    if (0 < listLen(Global.face))
    {
        // 获取人脸信息
        ret = getFaceInfo(Global.face);
        CHECK(ret == true, "获取人脸信息失败\n");

        // 渲染个人信息
        ret = renderInfo();
        CHECK(ret == true, "渲染个人信息失败\n");
    }

    DEB(else { DEBUG("未检测到人脸\n"); });

    // 设置已选择标志
    SET_FLAG(image_is_choice);
    return true;
error:
    return false;
}
