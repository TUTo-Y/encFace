#include "enc.h"

void zucKeyVi(uint8_t key[ZUC_KEY_SIZE], uint8_t iv[ZUC_IV_SIZE])
{
    // 初始化key和iv
    for (int i = 0; i < ZUC_KEY_SIZE; i++)
    {
        key[i] = rand() % 256;
        iv[i] = rand() % 256;
    }
}

bool zucEnc(const byte *in, byte *out, size_t size, ZUC_STATE *state)
{
    // 检测数据是否为空
    if (in == NULL || out == NULL && state == NULL)
        return false;

    // 生成密钥流
    size_t k_n = TO32(size) / sizeof(ZUC_UINT32);
    ZUC_UINT32 *k = malloc(k_n * sizeof(ZUC_UINT32));
    zuc_generate_keystream(state, k_n, k);

    // 加密数据
    for (int i = 0; i < k_n; i++)
        ((ZUC_UINT32 *)out)[i] = ((ZUC_UINT32 *)in)[i] ^ k[i];

    // 释放资源
    free(k);
    return true;
}

void sm2_public_key_info_to_pem_data(const SM2_KEY *key, data **out)
{
    if (!key || !out)
        return;

    FILE *file = tmpfile();
    size_t size = 0;

    // 将sm2公钥信息写入文件
    sm2_public_key_info_to_pem(key, file);

    // 读取文件大小
    fseek(file, 0L, SEEK_END);
    size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    // 读取文件
    *out = Malloc(size);
    fread((*out)->data, 1, size, file);

    fclose(file);
}

void sm2_public_key_info_from_pem_data(SM2_KEY *key, const char *data, size_t size)
{
    if (!key || !data || size == 0)
        return;

    FILE *file = tmpfile();

    // 写入pem
    fwrite(data, 1, size, file);
    fseek(file, 0L, SEEK_SET);

    // 读取pem
    memset(key, 0, sizeof(SM2_KEY));
    sm2_public_key_info_from_pem(key, file);

    fclose(file);
}