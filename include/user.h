#ifndef _USER_H
#define _USER_H

#include <stdio.h>
#include <stdbool.h>
#include <gmssl/zuc.h>
#include <gmssl/sm2.h>


/**
 * \brief 检查用户是否存在
 * \param filename 用户文件名
 * \retturn
*/
bool userCheckIn(const char *filename);

/**
 * \brief 向用户文件添加用户
 * \param filename 用户文件名
 * \param ID 用户名
 * \param sm2_key sm2用户密钥对
 * \param key ZUC密钥
 * \param iv ZUC初始化向量
 * \return
*/
bool userAdd(const char *filename, const char *ID, const SM2_KEY *sm2_key, const uint8_t key[ZUC_KEY_SIZE], const uint8_t iv[ZUC_KEY_SIZE]);

/**
 * \brief 从用户文件中获取用户信息
 * \param filename 用户文件名
 * \param ID 用户名
 * \param sm2_key sm2用户密钥对
 * \param key ZUC密钥
 * \param iv ZUC初始化向量
 * \return
*/
bool userGet(const char *filename, const char *ID, SM2_KEY *sm2_key, uint8_t key[ZUC_KEY_SIZE], uint8_t iv[ZUC_KEY_SIZE]);

#endif // _USER_H