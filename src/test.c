#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "main.h"
#include "gui_button.h"

#if 0

int main()
{
    char str1[0x20] = {"HelloWorld!"};
    char str2[0x20] = {"我是tuto"};
    char str3[0x20] = {"123456"};
    char str1_[0x20] = {0};
    char str2_[0x20] = {0};
    char str3_[0x20] = {0};
    char key[16] = {0};
    char iv[16] = {0};
    ZUC_STATE zuc;
    zucKeyIv(key, iv);
    zuc_init(&zuc, key, iv);
    zuc_encrypt(&zuc, str1, 0x20, str1_);
    zuc_init(&zuc, key, iv);
    zuc_encrypt(&zuc, str1_, 0x20, str1);
    printf("%s\n", str1);
    zuc_init(&zuc, key, iv);
    zuc_encrypt(&zuc, str2, 0x20, str2_);
    zuc_encrypt(&zuc, str3, 0x20, str3_);
    zuc_encrypt(&zuc, str2_, 0x20, str2);
    zuc_encrypt(&zuc, str3_, 0x20, str3);
    printf("%s\n", str2);
    printf("%s\n", str3);

    return 0;
}
#endif