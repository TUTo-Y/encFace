/**
 * 特征向量的提取与处理
 */
#ifndef _VECTORT_H
#define _VECTORT_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <SDL2/SDL.h>

#include "config.h"
#include "data.h"
#include "web.h"

/**
 * \brief 释放vec数据
 */
void freeVector(vector *vec);

#endif // _VECTORT_H