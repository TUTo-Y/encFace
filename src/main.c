#include "main.h"

// 全局数据
struct _Window Global = {0};

#undef main
int main(int argc, char *argv[])
{
    char path[PATH_MAX] = {0};

    srand(time(NULL));

    // 设置调试控制台支持字符
#ifdef _DEBUG
    SetConsoleOutputCP(CP_UTF8);
#endif

    // 选择目录
    if (select_image(path, PATH_MAX) == false)
    {
        DEBUG("未选择文件\n");
        return -1;
    }

    // 设置工作目录
    chdir(dirname(argv[0]));

    // 加载图片Surface
    Global.surface = IMG_Load(path);
    if (Global.surface == NULL)
    {
        ERR("无法加载图片到Surface: %s\n", IMG_GetError());
        return -1;
    }

    // 获取图像特征向量
    DEBUG("正在获取人脸特征向量...\n");
    if (get_face_vector(path, &Global.face) == false)
    {
        ERR("无法提取人脸信息\n");
        return -1;
    }

    // 加密后发送给服务器并获取人物数据
    DEBUG("正在从服务端获取信息...\n");
    if (get_face_info(Global.face) == false)
    {
        ERR("从服务器获取人脸数据失败\n");
        return -1;
    }

    // 初始化SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_AVIF | IMG_INIT_JXL);
    TTF_Init();

    // 创建窗口
    Global.window = SDL_CreateWindow("encFace", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 700, SDL_WINDOW_SHOWN);
    if (Global.window == NULL)
    {
        ERR("Create window failed: %s\n", SDL_GetError());
        return -1;
    }
    Global.renderer = SDL_CreateRenderer(Global.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (Global.renderer == NULL)
    {
        ERR("Create renderer failed: %s\n", SDL_GetError());
        return -1;
    }

    // 初始化全局数据
    SDL_GetWindowPosition(Global.window, &Global.windowRect.x, &Global.windowRect.y);
    SDL_GetWindowSize(Global.window, &Global.windowRect.w, &Global.windowRect.h);
    resize(&Global.windowRect, &Global.surfaceRect, Global.surface->w, Global.surface->h);
    Global.texture = SDL_CreateTextureFromSurface(Global.renderer, Global.surface);
    Global.scale = (float)Global.surfaceRect.w / (float)Global.surface->w;
    Global.scale2 = 1.0f;

    // 渲染人物信息栏
    if (render_info() == false)
    {
        ERR("Render info failed\n");
        return -1;
    }

    // core gui
    play();

    // 释放资源
    freeList(&Global.face, (void (*)(void *))freeVector);
    freeList(&Global.faceSurface, (void (*)(void *))SDL_FreeSurface);
    freeList(&Global.faceTexture, (void (*)(void *))SDL_DestroyTexture);
    SDL_FreeSurface(Global.surface);
    SDL_DestroyTexture(Global.texture);
    SDL_DestroyRenderer(Global.renderer);
    SDL_DestroyWindow(Global.window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}

/**
 * \brief 调用WindowsAPI选择图片
 */
bool select_image(char *path, size_t size)
{
    OPENFILENAME ofn;
    HWND hwnd = NULL;
    HANDLE hf;

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = path;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = size;
    ofn.lpstrFilter = "Image\0*.BMP;*.GIF;*.JPG;*.JPEG;*.LBM;*.PCX;*.PNG;*.PNM;*.SVG;*.TGA;*.TIFF;*.WEBP;*.XCF;*.XPM;*.XV\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE)
    {
        // 检查文件是否存在
        hf = CreateFile(ofn.lpstrFile,
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
