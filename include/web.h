#ifndef _WEB_H
#define _WEB_H

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "config.h"
#include "enc.h"
#include "data.h"

typedef size_t MSG_TYPE;               // 消息类型
#define MSG_TYPE_SIZE sizeof(MSG_TYPE) // 消息类型长度
#define MSG_CLOSE 0                    // 关闭连接消息
#define MSG_SUCESS 1                   // 处理成功
#define MSG_GET_FACE_VECTOR 2          // 请求获取人脸特征向量
#define MSG_GET_FACE_INFO 3            // 请求获取人脸信息

#define MSG_FACE 4     // 人脸数据
#define MSG_FACE_END 5 // 结束

/**
 * 客户端上传特征向量的数据格式:
 * 
 * 向远程服务器发送请求(size_t)
 * 客户端发送身份ID长度(size_t)
 * 客户端发送身份ID(byte*)
 * 客户端发送需要验证的人脸总数量(size_t)
 * 发送第一个人脸数据:
 *      经过sm9加密的ZUC密钥长度(size_t)
 *      经过sm9加密的ZUC密钥(byte*)
 *      经过ZUC加密后数据的长度(size_t)
 *      经过ZUC加密后数据的数据(byte*)
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
 * \brief 建立与远程服务器的连接
 * \return 是否成功
 */
bool connectServer();

/**
 * \brief 启动facenet服务器并与facenet服务器的连接
 * \return 是否成功
 */
bool connectFaceNet();

/**
 * \brief 关闭与远程服务器的连接
 */
void closeServer();

/**
 * \brief 关闭与facenet服务器的连接
 */
void closeFaceNet();

/**
 * \brief 从图片中提取特征向量
 * \param image 图片
 * \param head 链表头指针
 * \return 是否成功提取特征向量
 */
bool getFaceVector(const char *file, list **head);

/**
 * \brief 通过人脸特征向量获取人脸信息
 * \param face 人脸数据链表
 * \return 是否成功
 */
bool getFaceInfo(list *face);

#endif