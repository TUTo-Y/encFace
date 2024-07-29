#include "main.h"
#undef main

DEB(int test(int argc, char **argv));

int main(int argc, char *argv[])
{
    DEB(
        if (argc > 1) {
            return test(argc, argv);
        })

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

    // 初始化线程锁
    pthread_mutex_init(&Global.lock, NULL);

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

    // core gui
    if (gui_login() == LOGIN_SUCCESS)
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

    // 获取文件路径
    if (response_id == GTK_RESPONSE_ACCEPT)
    {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

        char *filename;
        filename = gtk_file_chooser_get_filename(chooser);
        g_strlcpy((gchar *)data, filename, PATH_MAX);
        g_free(filename);
    }

    // 销毁对话框
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

bool selectImageFile(char *path, size_t size)
{
    GtkWidget *dialog;
    gint res;

    if (path == NULL || size == 0)
    {
        return false;
    }
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

    const char *patterns[] = {
        "*.[Bb][Mm][Pp]", "*.[Gg][Ii][Ff]", "*.[Jj][Pp][Gg]", "*.[Jj][Pp][Ee][Gg]",
        "*.[Ll][Bb][Mm]", "*.[Pp][Cc][Xx]", "*.[Pp][Nn][Gg]", "*.[Pp][Nn][Mm]",
        "*.[Ss][Vv][Gg]", "*.[Tt][Gg][Aa]", "*.[Tt][Ii][Ff][Ff]", "*.[Ww][Ee][Bb][Pp]",
        "*.[Xx][Cc][Ff]", "*.[Xx][Pp][Mm]", "*.[Xx][Vv]"};

    for (size_t i = 0; i < sizeof(patterns) / sizeof(patterns[0]); ++i)
        gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter), patterns[i]);

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
    ret = Global.thread_status;
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
