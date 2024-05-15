#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmssl/sm9.h>

#define ID "admin.com"
#define passwd "123456"

// #define SM9_MAX_PLAINTEXT_SIZE 255
// #define SM9_MAX_CIPHERTEXT_SIZE 367

int main()
{
    SM9_ENC_MASTER_KEY master;
    SM9_ENC_KEY key;

    char msg[SM9_MAX_PLAINTEXT_SIZE] = { "HelloWorld!" };
    char out[SM9_MAX_CIPHERTEXT_SIZE] = { 0 };
    size_t size = 0;
    FILE *fp = NULL;

    /* 读取主公钥进行加密 */
    fp = fopen("master_public_key.pem", "rb");
    sm9_enc_master_public_key_from_pem(&master, fp);
    fclose(fp);

    /* 读取加密密钥用于解密 */
    fp = fopen("key_info.pem", "rb");
    sm9_enc_key_info_decrypt_from_pem(&key, passwd, fp);
    fclose(fp);

    printf("加密前:%s\n", msg);
    /* 加密 */
    sm9_encrypt(&master, ID, strlen(ID), msg, strlen(msg) + 1, out, &size);
    printf("加密后:%s\n", out);
    printf("加密后长度:%d\n", size);
    
    /* 解密 */
    sm9_decrypt(&key, ID, strlen(ID), out, size, msg, &size);
    printf("解密后:%s\n", msg);

    return 0;
}