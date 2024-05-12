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
 * \brief 从图片中提取特征向量
 * \param image 图片
 * \param head 链表头指针
 * \return 是否成功提取特征向量
 */
bool get_face_vector(const char *image, list **head);

/**
 * \brief 释放vec数据
 */
void freeVector(vector *vec);

#endif // _VECTORT_H