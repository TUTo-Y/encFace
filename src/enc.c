#include "enc.h"

/**
 * \brief 随即生成ZUC密钥和初始化向量
 */
void zucKeyVi(uint8_t key[ZUC_KEY_SIZE], uint8_t iv[ZUC_KEY_SIZE])
{
    // 初始化key和iv
    for (int i = 0; i < ZUC_KEY_SIZE; i++)
    {
        key[i] = rand() % 256;
        iv[i] = rand() % 256;
    }
}

/**
 * \brief ZUC加密函数
 * \param msg 需要加密的数据
 * \param out 加密后的数据
 * \param key 密钥
 * \param iv  初始化向量
 */
bool zucEnc(const data *msg, data **out, uint8_t key[ZUC_KEY_SIZE], uint8_t iv[ZUC_KEY_SIZE])
{

    ZUC_STATE state = {0};

    // 检测数据是否为空
    if (msg == NULL || msg->data == NULL)
        return false;

    // 初始化key和iv
    zucKeyVi(key, iv);

    // 使用key和iv初始化ZUC
    zuc_init(&state, key, iv);

    // 生成密钥流
    size_t k_n = TO32(msg->size) / sizeof(ZUC_UINT32);
    ZUC_UINT32 *k = malloc(k_n * sizeof(ZUC_UINT32));
    zuc_generate_keystream(&state, k_n, k);

    // 加密数据
    *out = Malloc(msg->size); // chunk关于8字节对齐, 所以无需对齐手动msg的data
    for (int i = 0; i < k_n; i++)
        ((ZUC_UINT32 *)(*out)->data)[i] = ((ZUC_UINT32 *)msg->data)[i] ^ k[i];

    // 释放资源
    free(k);
    return true;
}

/**
 * \brief ZUC解密函数
 * \param msg 需要解密的数据
 * \param out 加密后的数据
 * \param key 密钥
 * \param iv  初始化向量
 */
bool zucDec(const data *msg, data **out, const uint8_t key[ZUC_KEY_SIZE], const uint8_t iv[ZUC_KEY_SIZE])
{
    ZUC_STATE state = {0};

    if (msg == NULL || msg->data == NULL || msg->size == 0 || out == NULL)
        return false;

    // 使用key和iv初始化ZUC
    zuc_init(&state, key, iv);

    // 生成密钥流
    size_t k_n = TO32(msg->size) / sizeof(ZUC_UINT32);
    ZUC_UINT32 *k = malloc(k_n * sizeof(ZUC_UINT32));
    zuc_generate_keystream(&state, k_n, k);

    // 解密数据
    *out = Malloc(msg->size); // chunk关于8字节对齐, 所以无需对齐手动msg的data
    for (int i = 0; i < k_n; i++)
        ((ZUC_UINT32 *)(*out)->data)[i] = ((ZUC_UINT32 *)msg->data)[i] ^ k[i];

    // 释放资源
    free(k);
    return true;
}

/**
 * \brief 获取sm2公钥信息
 * \param key sm2公钥
 * \param out sm2公钥信息
 */
void sm2_public_key_info_to_pem_data(const SM2_KEY *key, data **out)
{
    FILE *file = tmpfile();
    size_t size = 0;

    if (!key || !out)
        return;

    // 将sm2公钥信息写入文件
    sm2_public_key_info_to_pem(key, file);

    // 读取文件大小
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    // 读取文件
    *out = Malloc(size);
    (*out)->size = size;
    fread((*out)->data, 1, size, file);

    fclose(file);
}