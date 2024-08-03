#ifndef _GUI_PLAY_H
#define _GUI_PLAY_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <gtk-3.0/gtk/gtk.h>

#include "th.h"
#include "web.h"
#include "data.h"
#include "draw.h"
#include "gui_msg.h"
#include "gui_text.h"
#include "gui_edit.h"
#include "gui_button.h"

enum
{
    guiPlayEnum,
    guiPlayEnum_ban, // Play1界面被禁用

    guiPlayEnum_button_ban,            // 按钮被禁用(不再渲染和处理消息)
    guiPlayEnum_button_selected,       // 按钮被选中
    guiPlayEnum_button_selected_init,  // 按钮被选中动画开始
    guiPlayEnum_button_selected_start, // 按钮被选中动画
    guiPlayEnum_button_leave_init,     // 按钮离开动画开始
    guiPlayEnum_button_leave_start,    // 按钮离开动画
    guiPlayEnum_button_pressed,        // 按钮被按下
    guiPlayEnum_button_pressed_init,   // 按钮被选中动画开始
    guiPlayEnum_button_pressed_start,  // 按钮被选中动画
    guiPlayEnum_button_free_init,      // 按钮被释放动画开始
    guiPlayEnum_button_free_start,     // 按钮被释放动画

    guiPlayEnum_watt_init, // 处理等待
    guiPlayEnum_watt_end,  // 处理等待结束

    guiPlayEnum_image_ban,   // 图片被禁用(不再渲染和处理消息)
    guiPlayEnum_image_enter, // 图片被按下

    guiPlayEnum_choice,      // 选择了该图片
    guiPlayEnum_choice_enter // 确认选择了该图片
};

// 界面一数据
typedef struct _gui_Play
{
    int flag; // 界面标志

    float buttonRadius;          // 圆角半径
    SDL_FPoint buttonCenter;     // 圆心
    SDL_Texture **buttonTexture; // 按钮纹理
    SDL_Texture *buttonTextureS; // 按钮纹理选择时
    SDL_Texture *buttonTextureP; // 按钮纹理按下时

    int imageW, imageH;        // 图片实际大小
    float scale1;              // 图片第一次缩放比例(缩放后:实际)
    float scale2;              // 图片第二次缩放比例
    SDL_FRect imageRect;       // 图片位置
    SDL_Texture *imageTexture; // 图片纹理

    list *figure; // 人脸信息列表

    int time;  // 动画时间
    int time2; // 动画时间2

    personal *choiceNode; // 被选中的节点
    SDL_Point choicePos;  // 选择的坐标

    guiMsg *msg;            // 消息队列
    SDL_Renderer *renderer; // 渲染器
} guiPlay;

// 获取人脸信息的线程参数
typedef struct _faceThreadParam
{
    char PATH[100]; // 图片路径
    guiMsg *msg;    // 消息队列

    list *figure; // 人脸信息列表
} faceThreadParam;

/**
 * \brief 选择图片
 */
bool selectImageFile(char *path, size_t size);

/**
 * \brief 渲染用户信息
 */
void renderUserData(personal *p, SDL_Renderer *renderer);

/**
 * \brief gui_play界面
 */
void gui_play();

#endif // _GUI_PLAY_H