#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmssl/sm2.h>

int main()
{
    SM2_KEY key;
    SM2_KEY key2;
    SM2_KEY public;
    SM2_KEY private;
    FILE *fp = NULL;

    // 生产密钥对
    sm2_key_generate(&key);

    // 保存公钥
    fp = fopen("sm2_public_key.pem", "w");
    sm2_public_key_info_to_pem(&key, fp);
    fclose(fp);

    // 保存私钥
    fp = fopen("sm2_private_key.pem", "w");
    // sm2_private_key_info_to_pem(&key, fp);
    sm2_private_key_info_encrypt_to_pem(&key, "password", fp);
    fclose(fp);

    // 读取公钥
    fp = fopen("sm2_public_key.pem", "r");
    sm2_public_key_info_from_pem(&key2, fp);
    // sm2_public_key_info_from_pem(&public, fp);
    fclose(fp);

    // 读取私钥
    fp = fopen("sm2_private_key.pem", "r");
    sm2_private_key_info_decrypt_from_pem(&key2, "password", fp);
    // sm2_private_key_info_decrypt_from_pem(&private, "password", fp);
    fclose(fp);

    char data[] = "Hello World!";

    // 加密
    char cipher[SM2_MAX_CIPHERTEXT_SIZE] = {0};
    size_t cipherlen = 0;

    printf("加密前: %s\n大小=%d\n", data, strlen(data));
    sm2_encrypt(&key2, (uint8_t *)data, strlen(data), cipher, &cipherlen);

    // 解密
    char plain[SM2_MAX_PLAINTEXT_SIZE] = {0};
    size_t plainlen = 0;

    sm2_decrypt(&key2, (uint8_t *)cipher, cipherlen, plain, &plainlen);
    printf("解密后: %s\n大小=%d\n", plain, plainlen);
    

    return 0;
}