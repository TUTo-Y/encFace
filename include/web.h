#ifndef _WEB_H
#define _WEB_H

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "config.h"
#include "enc.h"
#include "data.h"

typedef size_t MSG_TYPE;               // 消息类型
#define MSG_TYPE_SIZE sizeof(MSG_TYPE) // 消息类型长度
#define MSG_CLOSE 0                    // 关闭连接消息
#define MSG_SUCESS 1                   // 处理成功

#define MSG_GET_FACE_VECTOR 2 // 请求获取人脸特征向量
#define MSG_GET_FACE_INFO 3   // 请求获取人脸信息

#define MSG_FACE 4     // 人脸数据
#define MSG_FACE_END 5 // 结束

#define MSG_REGISTER 6         // 注册请求
#define MSG_REGISTER_USER_IN 7 // 注册用户存在
#define MSG_REGISTER_USER_NO 8 // 注册用户不存在

#define MSG_LOGIN 9 // 登录请求

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

/**
 * \brief 注册
 * \param ID 用户ID
 * \param skey 服务器sm2公钥
 * \param mkey 保存用户sm2密钥对
 */
bool registerUser(const char *ID, const SM2_KEY *skey, SM2_KEY *mkey);

/**
 * \brief 登录
 */
bool loginUser(const char *ID);

#endif