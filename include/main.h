#ifndef _MAIN_H
#define _MAIN_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <locale.h>
#include <stdbool.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <gtk-3.0/gtk/gtk.h>

#include "config.h"
#include "data.h"
#include "gui_login.h"
#include "gui_play.h"
#include "enc.h"
#include "web.h"
#include "draw.h"

/**
 * \brief 选择图片
 */
bool selectImageFile(char *path, size_t size);

// 检查线程是否存在
bool getThread();

// 设置线程状态
void setThread(bool status);

#endif // _MAIN_H