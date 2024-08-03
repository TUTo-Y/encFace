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
#define MSG_ERROR 2                    // 处理失败

#define MSG_GET_FACE_VECTOR 3 // 请求获取人脸特征向量
#define MSG_GET_FACE_INFO 4   // 请求获取人脸信息

#define MSG_FACE 5     // 人脸数据
#define MSG_FACE_END 6 // 结束

#define MSG_REGISTER 7         // 注册请求
#define MSG_REGISTER_USER_IN 8 // 注册用户存在
#define MSG_REGISTER_USER_NO 9 // 注册用户不存在

#define MSG_LOGIN 10         // 登录请求
#define MSG_LOGIN_USER_IN 11 // 注册用户存在
#define MSG_LOGIN_USER_NO 12  // 注册用户不存在

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
 * \brief 从facenet服务器获取人脸特征向量
 * \param file 图片
 * \param l 人物列表
 * \return 是否成功提取特征向量
 */
bool getFaceVector(const char *file, list *l);

/**
 * \brief 从远程服务器获取人物信息
 * \param p 人物信息
 * \return 是否成功
 */
bool getFaceInfo(personal *p);

/**
 * \brief 上传人物数据
 * \param p 人物信息
 * \return 是否成功
 */
bool uploadFaceInfo(const personal *p);

/**
 * \brief 注册
 * \param ID 用户ID
 * \param skey 服务器sm2公钥
 * \param ukey 保存用户sm2密钥对
 * \return 是否成功
 * \retval 0 成功
 * \retval 2 其他错误
 * \retval 1 服务器端已有用户数据
 */
int registerUser(const char *ID, const SM2_KEY *skey, SM2_KEY *ukey);

/**
 * \brief 登录
 * \param ID 用户ID
 * \param skey 服务器sm2公钥
 * \param ukey 用户sm2密钥对
 * \return 是否成功
 * \retval 0 成功
 * \retval 2 服务器端无用户数据
 * \retval 1 其他错误
 */
int loginUser(const char *ID, const SM2_KEY *skey, const SM2_KEY *ukey);

#endif