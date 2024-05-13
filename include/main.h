#ifndef _MAIN_H
#define _MAIN_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <stdbool.h>
#include <winsock2.h>
#include <windows.h>
#include <commdlg.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <pthread.h>

#include "config.h"
#include "data.h"
#include "vector.h"
#include "gui.h"
#include "enc.h"
#include "web.h"

/**
 * \brief 调用WindowsAPI选择图片
 */
bool selectImage(wchar_t *path, size_t size);

/**
 * \brief 加载人脸
 */
bool loadFace();

#endif // _MAIN_H