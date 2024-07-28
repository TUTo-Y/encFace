#include "user.h"

/**
 * \brief 检查用户是否存在
 * \param filename 用户文件名
 * \retturn
 */
// bool userCheckIn(const char *filename);

/**
 * \brief 向用户文件添加用户
 * \param filename 用户文件名
 * \param ID 用户名
 * \param sm2_key sm2用户密钥对
 * \param key ZUC密钥
 * \param iv ZUC初始化向量
 * \return
 */

bool userAdd(const char *filename, const char *ID, const SM2_KEY *sm2_key, const uint8_t key[ZUC_KEY_SIZE], const uint8_t iv[ZUC_KEY_SIZE])
{
    // 检查参数
    if (!filename || !ID || !sm2_key || !key || !iv)
        return false;

    // 打开用户文件
    FILE *file = fopen(filename, "ab");
    if (file == NULL)
        return false;

    size_t len = 0; // ID长度

    uint8_t public_key_info[512] = {0};           // sm2公钥信息
    uint8_t *public_key_info_p = public_key_info; // 保存sm2公钥信息
    size_t public_key_info_len = 0;               //  sm2公钥信息长度

    uint8_t private_key_info[512] = {0};            // sm2私钥信息
    uint8_t *private_key_info_p = private_key_info; // 保存sm2私钥信息
    size_t private_key_info_len = 0;                // sm2私钥信息长度

    // 保存用户名
    len = strlen(ID);
    fwrite(&len, sizeof(size_t), 1, file); // 保存用户名长度
    fwrite(ID, sizeof(char), len, file);   // 保存用户名

    // 保存sm2公钥
    sm2_public_key_info_to_der(sm2_key, &public_key_info_p, &public_key_info_len);
    fwrite(&public_key_info_len, sizeof(size_t), 1, file);              // 保存公钥长度
    fwrite(public_key_info_p, sizeof(char), public_key_info_len, file); // 保存公钥

    // 保存sm2私钥
    sm2_private_key_info_to_der(sm2_key, &private_key_info_p, &private_key_info_len);
    fwrite(&private_key_info_len, sizeof(size_t), 1, file);               // 保存私钥长度
    fwrite(private_key_info_p, sizeof(char), private_key_info_len, file); // 保存私钥

    // 保存ZUC密钥和初始化向量
    fwrite(key, sizeof(uint8_t), ZUC_KEY_SIZE, file);
    fwrite(iv, sizeof(uint8_t), ZUC_KEY_SIZE, file);

    fclose(file);
    return true;
}

/**
 * \brief 从用户文件中获取用户信息
 * \param filename 用户文件名
 * \param ID 用户名
 * \param sm2_key sm2用户密钥对
 * \param key ZUC密钥
 * \param iv ZUC初始化向量
 * \return
 */
bool userGet(const char *filename, const char *ID, SM2_KEY *sm2_key, uint8_t key[ZUC_KEY_SIZE], uint8_t iv[ZUC_KEY_SIZE])
{
    // 检查参数
    if (!filename || !ID || !sm2_key || !key || !iv)
        return false;

    // 打开用户文件
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
        return false;

    size_t len = 0;       // 用户名长度
    char name[512] = {0}; // 用户名

    uint8_t public_key_info[512] = {0};                 // sm2公钥信息
    const uint8_t *public_key_info_p = public_key_info; // 保存sm2公钥信息
    size_t public_key_info_len = 0;                     //  sm2公钥信息长度

    uint8_t private_key_info[512] = {0};                  // sm2私钥信息
    const uint8_t *private_key_info_p = private_key_info; // 保存sm2私钥信息
    size_t private_key_info_len = 0;                      // sm2私钥信息长度

    // 循环读取所有用户信息
    while (!feof(file))
    {
        // 读取用户名
        fread(&len, sizeof(size_t), 1, file);
        fread(name, sizeof(char), len, file);

        // 读取sm2公钥
        fread(&public_key_info_len, sizeof(size_t), 1, file);
        fread(public_key_info, sizeof(char), public_key_info_len, file);

        // 读取sm2私钥
        fread(&private_key_info_len, sizeof(size_t), 1, file);
        fread(private_key_info, sizeof(char), private_key_info_len, file);

        // 读取ZUC密钥
        fread(key, sizeof(uint8_t), ZUC_KEY_SIZE, file);

        // 读取ZUC初始化向量
        fread(iv, sizeof(uint8_t), ZUC_KEY_SIZE, file);

        // 对比用户名
        if (!strcmp(name, ID))
        {
            // 读取成功, 保存sm2公钥
            sm2_public_key_info_from_der(sm2_key, &public_key_info_p, &public_key_info_len);

            // 读取成功, 保存sm2私钥
            sm2_private_key_from_der(sm2_key, &private_key_info_p, &private_key_info_len);

            // 关闭文件
            fclose(file);
            return true;
        }
    }

    fclose(file);
    return false;
}