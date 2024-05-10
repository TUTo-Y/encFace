#include "enc.h"

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
    for (int i = 0; i < ZUC_KEY_SIZE; i++)
    {
        key[i] = rand() % 256;
        iv[i] = rand() % 256;
    }

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

// /**
//  * \brief 获取一个RSA密钥对，并保存为PEM格式
//  * \param pri_key 私钥
//  * \param pub_key 公钥
//  * \retrun 是否成功
//  */
// bool getRSAKeyPair(data **pri_key, data **pub_key)
// {
//     if (pri_key == NULL || pub_key == NULL)
//         return;

//     int ret = 0;
//     RSA *keypair = RSA_new();
//     BIGNUM *bne = BN_new();
//     size_t size;

//     // 设置RSA公钥指数
//     ret = BN_set_word(bne, RSA_F4);
//     if (ret != 1)
//     {
//         ERR("BN_set_word failed\n");
//         return false;
//     }

//     // 生成一个新的RSA密钥对，密钥长度为4096位
//     ret = RSA_generate_key_ex(keypair, 4096, bne, NULL);
//     if (ret != 1)
//     {
//         ERR("RSA_generate_key_ex failed\n");
//         RSA_free(keypair);
//         return false;
//     }

//     // 创建BIO
//     BIO *bp_public = BIO_new(BIO_s_mem());
//     BIO *bp_private = BIO_new(BIO_s_mem());

//     // 将RSA密钥对写入BIO
//     PEM_write_bio_RSAPrivateKey(bp_private, keypair, NULL, NULL, 0, NULL, NULL);
//     PEM_write_bio_RSAPublicKey(bp_public, keypair);

//     // 写入公钥
//     size = BIO_pending(bp_public);
//     *pub_key = Malloc(size + 1);
//     BIO_read(bp_public, (*pub_key)->data, size);
//     (*pub_key)->data[size] = '\0';

//     // 写入私钥
//     size = BIO_pending(bp_private);
//     *pri_key = Malloc(size + 1);
//     BIO_read(bp_private, (*pri_key)->data, size);
//     (*pri_key)->data[size] = '\0';

//     // 释放资源
//     BIO_free_all(bp_public);
//     BIO_free_all(bp_private);
//     RSA_free(keypair);
//     BN_free(bne);
//     return true;
// }

// /**
//  * \brief 从内存中读取PEM格式的私钥
//  */
// RSA *readPrivateKeyFromMem(const char *pri_key)
// {
//     BIO *bio = BIO_new_mem_buf(pri_key, -1);
//     if (bio == NULL)
//     {
//         ERR("Failed to create BIO for private key\n");
//         return NULL;
//     }

//     RSA *rsa = RSA_new();
//     rsa = PEM_read_bio_RSAPrivateKey(bio, &rsa, NULL, NULL);

//     BIO_free(bio);
//     return rsa;
// }

// /**
//  * \brief 从内存中读取PEM格式的公钥
//  */
// RSA *readPublicKeyFromMem(const char *pub_key)
// {
//     BIO *bio = BIO_new_mem_buf(pub_key, -1);
//     if (bio == NULL)
//     {
//         ERR("Failed to create BIO for public key\n");
//         return NULL;
//     }

//     RSA *rsa = RSA_new();
//     rsa = PEM_read_bio_RSAPublicKey(bio, &rsa, NULL, NULL);

//     BIO_free(bio);
//     return rsa;
// }