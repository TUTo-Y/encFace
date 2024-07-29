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
#include <gmssl/sm2.h>

#include "data.h"

/**
 * \brief 随即生成ZUC密钥和初始化向量
 */
void zucKeyVi(uint8_t key[ZUC_KEY_SIZE], uint8_t iv[ZUC_KEY_SIZE]);

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

/**
 * \brief 获取sm2公钥信息
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