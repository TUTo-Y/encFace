#include "main.h"

#undef main

int main(int argc, char *argv[])
{
    int ret;

    /************* 初始化设置 *************/
    srand(time(NULL));

    // 初始化gtk
    gtk_init(&argc, &argv);

    // // 设置调试控制台支持字符
    // DEB(SetConsoleOutputCP(CP_UTF8));

    // 初始化与远程服务器的连接
    DEBUG("正在连接到远程服务器...\n");
    ret = connectServer();
    CHECK(ret == true, "无法连接到远程服务器\n");

    // 启动facenet服务器并与facenet服务器的连接
    DEBUG("正在连接到facenet服务器...\n");
    ret = connectFaceNet();
    CHECK(ret == true, "无法连接到facenet服务器\n");

    // 初始化线程锁
    pthread_mutex_init(&Global.lock, NULL);

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

    // core gui
    gui_play();

error:
    // 销毁字体
    TTF_CloseFont(Global.font);

    // 结束SDL
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    // 销毁线程锁
    pthread_mutex_destroy(&Global.lock);

    // 关闭与facenet服务器的连接
    closeFaceNet();

    // 关闭与远程服务器的连接
    closeServer();

    return 0;
}

void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer data)
{
    char *path = (char *)data;
    size_t size = PATH_MAX;

    // 获取文件路径
    if (response_id == GTK_RESPONSE_ACCEPT)
    {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

        char *filename;
        filename = gtk_file_chooser_get_filename(chooser);
        g_strlcpy(path, filename, size);
        g_free(filename);
    }

    // 销毁对话框
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

bool selectImageFile(char *path, size_t size)
{
    GtkWidget *dialog;
    gint res;

    // 初始化
    *path = '\0';

    // 创建文件选择对话框
    dialog = gtk_file_chooser_dialog_new("选择图片文件",
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "取消",
                                         GTK_RESPONSE_CANCEL,
                                         "选择",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);
    // 设置文件选择对话框属性
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(GTK_FILE_FILTER(filter), "图片文件");
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*.[Bb][Mm][Pp]");
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*.[Gg][Ii][Ff]");
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*.[Jj][Pp][Gg]");
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*.[Jj][Pp][Ee][Gg]");
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*.[Ll][Bb][Mm]");
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*.[Pp][Cc][Xx]");
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*.[Pp][Nn][Gg]");
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*.[Pp][Nn][Mm]");
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*.[Ss][Vv][Gg]");
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*.[Tt][Gg][Aa]");
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*.[Tt][Ii][Ff][Ff]");
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*.[Ww][Ee][Bb][Pp]");
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*.[Xx][Cc][Ff]");
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*.[Xx][Pp][Mm]");
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), "*.[Xx][Vv]");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(on_dialog_response), path);
    g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 显示文件选择对话框
    gtk_widget_show(dialog);
    gtk_main();

    return *path != '\0';
}

bool choiceImage(
                 const SDL_Rect *windowRect,
                 SDL_Renderer *renderer,
                 SDL_Surface **surface, SDL_Texture **texture,
                 SDL_FRect *surfaceRect, float *scale, float *scale2,
                 list **face, list **faceSurface, list **faceTexture)
{
    int ret = 0;
    SDL_Surface *tmpSurface = NULL;
    
    // 初始化
    memset(Global.path, 0, sizeof(Global.path));

    // 选择图片
    ret = selectImageFile(Global.path, sizeof(Global.path));
    CHECK(ret == true, "未选择图片\n");

    // 清除原有数据
    if (CHECK_FLAG(image_is_choice))
    {
        // 释放资源
        listFree(face, (void (*)(void *))freeVector);
        listFree(faceSurface, (void (*)(void *))SDL_FreeSurface);
        listFree(faceTexture, (void (*)(void *))SDL_DestroyTexture);
        SDL_FreeSurface(*surface);
        SDL_DestroyTexture(*texture);

        // 清除标志
        CLEAR_FLAG(image_is_choice);
    }

    // 读取图片Surface
    tmpSurface = IMG_Load_RW(SDL_RWFromFile(Global.path, "rb"), 1);
    CHECK(tmpSurface, "无法读取图片文件 : %s\n", IMG_GetError());

    // 转化图片格式到四通道
    *surface = SDL_ConvertSurfaceFormat(tmpSurface, SDL_PIXELFORMAT_RGBA32, 0);
    CHECK(*surface, "转换图片格式失败 : %s\n", SDL_GetError());
    SDL_FreeSurface(tmpSurface);

    // 读取图片Texture
    *texture = SDL_CreateTextureFromSurface(renderer, *surface);
    CHECK(*texture, "创建图片纹理失败 : %s\n", SDL_GetError());

    // 调整图片大小和缩放
    resizeImage(windowRect, surfaceRect, (*surface)->w, (*surface)->h);
    *scale = surfaceRect->w / (*surface)->w;
    *scale2 = 1.0f;

    // 创建线程
    setThread(true);
    ret = pthread_create(&Global.thread, NULL, th, face);
    CHECK(ret == 0, "创建线程失败\n");

    // 设置已选择标志
    SET_FLAG(image_is_choice);
    return true;

error:
    if (tmpSurface)
        SDL_FreeSurface(tmpSurface);
    return false;
}

void *th(void *arg)
{
    int ret = 0;

    // 获取人脸特征向量
    ret = getFaceVector(Global.path, (list**)arg);
    CHECK(ret == true, "获取人脸特征向量失败\n");

    // 如果检测到人脸
    if (0 < listLen(*(list**)arg))
    {
        // 获取人脸信息
        ret = getFaceInfo(*(list**)arg);
        CHECK(ret == true, "获取人脸信息失败\n");
    }

    DEB(else { DEBUG("未检测到人脸\n"); });

error:
    setThread(false);
    return NULL;
}


// 检查线程是否存在
bool getThread()
{
    bool ret;
    pthread_mutex_lock(&Global.lock);
    ret = Global.thread_status != 0;
    pthread_mutex_unlock(&Global.lock);
    return ret;
}

// 设置线程状态
void setThread(bool status)
{
    pthread_mutex_lock(&Global.lock);
    Global.thread_status = status;
    pthread_mutex_unlock(&Global.lock);
}
