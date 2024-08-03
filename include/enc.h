/**
 * 函数
 */
#ifndef _ENC_H
#define _ENC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmssl/zuc.h>
#include <gmssl/sm2.h>

#include "config.h"
#include "data_base.h"

/**
 * \brief 随即生成ZUC密钥和初始化向量
 */
void zucKeyVi(uint8_t key[ZUC_KEY_SIZE], uint8_t iv[ZUC_IV_SIZE]);

/**
 * \brief ZUC加密解密函数
 * \param in 输入数据
 * \param out 输出数据
 * \param size 数据大小
 * \param state ZUC状态
 * \return 成功返回true, 失败返回false
 */
bool zucEnc(const byte *in, byte *out, size_t size, ZUC_STATE *state);

/**
 * \brief 获取sm2公钥到pem
 * \param key sm2公钥
 * \param out sm2公钥信息
 */
void sm2_public_key_info_to_pem_data(const SM2_KEY *key, data **out);

/*
 * \brief 从pem数据中获取sm2公钥信息
 * \param key sm2公钥
 * \param data pem数据
 * \param size pem数据大小
 */
void sm2_public_key_info_from_pem_data(SM2_KEY *key, const char *data, size_t size);

#endif