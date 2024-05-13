#include "vector.h"

/**
 * \brief 释放vec数据
 */
void freeVector(vector *vec)
{
    if (vec)
    {
        if (vec->v)
            Free(vec->v);
        free(vec);
    }
}