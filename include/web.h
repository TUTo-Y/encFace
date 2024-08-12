#ifndef _WEB_H
#define _WEB_H

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#ifdef _LINUX
#include <fcntl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
// 宏替换
#define socket(af, type, protocol) WSASocket(af, type, protocol, NULL, 0, 0)
#define bind(sock, addr, addrlen) bind(sock, addr, addrlen)
#define listen(sock, backlog) listen(sock, backlog)
#define accept(sock, addr, addrlen) accept(sock, addr, addrlen)
#define recv(sock, buf, len, flags) recv(sock, buf, len, flags)
#define send(sock, buf, len, flags) send(sock, buf, len, flags)
#define close(sock) closesocket(sock)
#endif

#include "config.h"
#include "enc.h"
#include "data.h"

typedef size_t MSG_TYPE;               // 消息类型
#define MSG_TYPE_SIZE sizeof(MSG_TYPE) // 消息类型长度
enum
{
    MSG_CLOSE = 0,  // 关闭连接消息
    MSG_SUCESS = 1, // 处理成功
    MSG_ERROR = 2,  // 处理失败

    MSG_GET_FACE_VECTOR, // 请求获取人脸特征向量
    MSG_FACE,            // 获取人脸特征向量
    MSG_FACE_END,        // 获取人脸特征向量结束

    MSG_GET_FACE_INFO, // 请求获取人脸信息

    MSG_UPLOAD_FACE_INFO, // 请求上传人物信息

    MSG_REGISTER,         // 注册请求
    MSG_REGISTER_USER_IN, // 注册用户存在
    MSG_REGISTER_USER_NO, // 注册用户不存在

    MSG_LOGIN,         // 登录请求
    MSG_LOGIN_USER_IN, // 注册用户存在
    MSG_LOGIN_USER_NO, // 注册用户不存在
};

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