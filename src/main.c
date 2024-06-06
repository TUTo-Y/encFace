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
    ret = pthread_mutex_init(&Global.lock, NULL);

    // 读取sm2服务器公钥
    FILE *fp = fopen(SM2_PUBLIC, "r");
    CHECK(fp, "无法打开"SM2_PUBLIC"\n");
    sm2_public_key_info_from_pem(&Global.SM2server, fp);
    fclose(fp);

    // 初始化SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_AVIF | IMG_INIT_JXL);
    TTF_Init();

    // 加载字体
    Global.font = TTF_OpenFont(TTF_PATH, 64);
    CHECK(Global.font != NULL, "TTF_OpenFont: %s\n", TTF_GetError());

    // core gui
    if(gui_login() == LOGIN_SUCCESS)
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

static void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer data)
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
