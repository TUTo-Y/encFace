#ifndef _MSG_H_
#define _MSG_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "draw.h"

enum
{
    guiMsgEnum_Success = 0,
    guiMsgEnum_Warning,
    guiMsgEnum_Error,
    guiMsgEnum_Add,        // 当前正在添加中
    guiMsgEnum_Add_Start,  // 将消息添加进入开始
    guiMsgEnum_Add_Middle, // 将消息添加进入一半
};

// 一条消息未渲染的消息
typedef struct _guiNoRender
{
    char msg[0x100]; // 消息内容
    int flag;  // 消息标志
} guiNoRender;

// 一条消息
typedef struct _guiM
{
    SDL_Texture *Texture; // 消息纹理
    int time;             // 消息刚开始显示时间
} guiM;

typedef struct _gui_msg
{
    int flag;      // 消息标志
    list proc;     // 正在显示的消息列表
    list watt;     // 等待显示的消息队列
    list NoRender; // 未渲染的消息队列

    TTF_Font *Font; // 消息字体
    SDL_Color colorSuccessFD; // 正确消息字体颜色
    SDL_Color colorSuccessBK; // 正确消息背景颜色
    SDL_Color colorWarningFD; // 警告消息字体颜色
    SDL_Color colorWarningBK; // 警告消息背景颜色
    SDL_Color colorErrorFD;   // 错误消息字体颜色
    SDL_Color colorErrorBK;   // 错误消息背景颜色

    SDL_Rect Rect; // 第一条消息的位置
    float scale;   // 消息文本高度所占缩放比例
    int interval;  // 间隔长度
    int timeLive;  // 消息显示时间(ms)
    int timeFade;  // 消息消失时间(ms)
    int direction; // 消息移动方向(向下1, 向上-1)

    SDL_Renderer *Renderer; // 渲染器

    int time; // 动画时间

    pthread_mutex_t lock; // 互斥锁
} guiMsg;

/**
 * \brief 初始化消息
 * \param gm 消息指针
 * \param Font 消息字体
 * \param colorSuccessFD 正确消息字体颜色(可选)
 * \param colorSuccessBK 正确消息背景颜色(可选)
 * \param colorWarningFD 警告消息字体颜色(可选)
 * \param colorWarningBK 警告消息背景颜色(可选)
 * \param colorErrorFD 错误消息字体颜色(可选)
 * \param colorErrorBK 错误消息背景颜色(可选)
 * \param Rect 第一条消息的位置
 * \param scale 消息文本高度所占缩放比例
 * \param interval 间隔长度
 * \param timeLive 消息显示时间(ms)
 * \param timeFade 消息消失时间(ms)
 * \param direction 消息移动方向(向下1, 向上-1)
 * \param Renderer 渲染器
 * \return void
 */
void gmInit(guiMsg *gm,
            TTF_Font *Font,            // 消息字体
            SDL_Color *colorSuccessFD, // 正确消息字体颜色(可选)
            SDL_Color *colorSuccessBK, // 正确消息背景颜色(可选)
            SDL_Color *colorWarningFD, // 警告消息字体颜色(可选)
            SDL_Color *colorWarningBK, // 警告消息背景颜色(可选)
            SDL_Color *colorErrorFD,   // 错误消息字体颜色(可选)
            SDL_Color *colorErrorBK,   // 错误消息背景颜色(可选)

            SDL_Rect Rect, // 第一条消息的位置
            float scale,   // 消息文本高度所占缩放比例
            int interval,  // 间隔长度
            int timeLive,  // 消息显示时间(ms)
            int timeFade,  // 消息消失时间(ms)
            int direction, // 消息移动方向(向下1, 向上-1)

            SDL_Renderer *Renderer // 渲染器
);

/**
 * \brief 渲染消息
 * \param gm 消息指针
 * \return void
 */
void gmRender(guiMsg *gm);

/**
 * \brief 添加消息
 * \param gm 消息指针
 * \param msg 消息内容
 * \param flag 消息标志
 */
void gmAdd(guiMsg *gm, char *msg, int flag);

/**
 * \brief 删除所有消息
 */
void gmDestroy(guiMsg *gm);

#endif // _MSG_H_