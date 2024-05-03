#ifndef _WEB_H
#define _WEB_H

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <sys/types.h>

#include "config.h"
#include "enc.h"
#include "data.h"
#include "vector.h"

/**
 * 客户端上传特征向量的数据格式:
 *
 * 客户端发送身份ID长度(size_t)
 * 客户端发送身份ID(char*)
 * 客户端发送需要验证的人脸总数量(size_t)
 * 发送第一个人脸数据:
 *      经过sm9加密的ZUC密钥长度(size_t)
 *      经过sm9加密的ZUC密钥(char*)
 *      经过ZUC加密后数据的长度(size_t)
 *      经过ZUC加密后数据的数据(char*)
 * 发送第二个人脸数据:
 * ...
 */

/**
 * 服务端返回的数据格式:
 *
 * 第一个角色flag(int)
 * 第一个角色的数据(msg) //不存在则跳过
 * 第二个角色flag(int)
 * 第二个角色的数据(msg) //不存在则跳过
 * ...
 */

/**
 * \brief 通过人脸特征向量获取人脸信息
 * \param face 人脸数据链表
 * \return 是否成功
 */
bool get_face_info(list *face);

#endif