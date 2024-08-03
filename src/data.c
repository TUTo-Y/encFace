#include "data.h"
// 全局数据
struct _Global Global = {0};

void freePersonal(personal *p)
{
    if(p->infoTexture)
        SDL_DestroyTexture(p->infoTexture);

    if(p->vector)
        Free(p->vector);

    free(p);
}