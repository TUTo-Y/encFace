/**
 * 生成SM9密钥
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmssl/sm9.h>

#define ID "adminTUTo"
#define passwd "123456"

int main()
{
    uint8_t *data = NULL;
    size_t size = 0;
    FILE *fp = NULL;
    SM9_ENC_MASTER_KEY master;
    SM9_ENC_KEY key;
    sm9_enc_master_key_generate(&master);
    sm9_enc_master_key_extract_key(&master, ID, strlen(ID), &key);

    // 主密钥信息
    fp = fopen("master_key_info.pem", "wb");
    sm9_enc_master_key_info_encrypt_to_pem(&master, passwd, fp);
    fclose(fp);

    // 加密主公钥
    fp = fopen("master_public_key.pem", "wb");
    sm9_enc_master_public_key_to_pem(&master, fp);
    fclose(fp);

    // 加密密钥信息
    fp = fopen("key_info.pem", "wb");
    sm9_enc_key_info_encrypt_to_pem(&key, passwd, fp);
    fclose(fp);


    return 0;
}