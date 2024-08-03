#ifndef GUI_EDIT_H_
#define GUI_EDIT_H_

#include "gui_play.h"

/**
 * \brief 编辑界面
 * \param renderer 渲染器
 * \param backSurface 背景表面
 * \param msg 消息
 * \param p 个人信息
 * \return int 返回值 关闭界面为0, 否则为1
 */
int guiEdit(SDL_Renderer *renderer, SDL_Surface *backSurface, guiMsg *msg, personal *p);

#endif // GUI_EDIT_H_