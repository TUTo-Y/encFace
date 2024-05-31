/**
 * 加密函数
 */
#ifndef _ENC_H
#define _ENC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmssl/zuc.h>
#include <gmssl/sm9.h>

#include "data.h"

/**
 * \brief ZUC加密函数
 * \param msg 需要加密的数据
 * \param out 加密后的数据
 * \param key 密钥
 * \param iv  初始化向量
 */
bool zucEnc(const data *msg, data **out, uint8_t key[ZUC_KEY_SIZE], uint8_t iv[ZUC_KEY_SIZE]);

/**
 * \brief ZUC解密函数
 * \param msg 需要解密的数据
 * \param out 加密后的数据
 * \param key 密钥
 * \param iv  初始化向量
 */
bool zucDec(const data *msg, data **out, const uint8_t key[ZUC_KEY_SIZE], const uint8_t iv[ZUC_KEY_SIZE]);

#endif