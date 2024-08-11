#include "main.h"
#undef main

DEB(int test(int argc, char **argv));

#if 1
int main(int argc, char *argv[])
{
    int ret;

    /************* 初始化设置 *************/
    srand(time(NULL));

    // 初始化gtk
    gtk_init(&argc, &argv);

    // 设置调试控制台支持中文字符
    setlocale(LC_ALL, "zh_CN.UTF-8");

    // 与远程服务器的连接
    DEBUG("正在连接到远程服务器...\n");
    ret = connectServer();
    CHECK(ret == true);

    // 启动facenet服务器并与facenet服务器的连接
    DEBUG("正在连接到facenet服务器...\n");
    ret = connectFaceNet();
    CHECK(ret == true);

    // 读取sm2服务器公钥
    FILE *fp = fopen(SM2_PUBLIC, "r");
    CHECK(fp, "无法打开" SM2_PUBLIC "\n");
    sm2_public_key_info_from_pem(&Global.SM2server, fp);
    fclose(fp);

    // 初始化SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_AVIF | IMG_INIT_JXL);
    TTF_Init();

    // 加载字体
    Global.font = TTF_OpenFont(TTF_PATH, 64);
    CHECK(Global.font != NULL, "TTF_OpenFont: %s\n", TTF_GetError());

    // 初始化线程保护
    initThread();

    // core gui
    if (gui_login() == LOGIN_SUCCESS)
    {
        gui_play();
    }

error:
    // 销毁线程保护
    destroyThread();

    // 销毁字体
    TTF_CloseFont(Global.font);

    // 结束SDL
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    // 关闭与facenet服务器的连接
    closeFaceNet();

    // 关闭与远程服务器的连接
    closeServer();

    return 0;
}
#endif
