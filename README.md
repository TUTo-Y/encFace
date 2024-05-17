- [encFace客户端模块](#encface客户端模块)
  - [环境依赖](#环境依赖)
  - [MTCNN和InceptionResnetV1的识别测试](#mtcnn和inceptionresnetv1的识别测试)
  - [目录](#目录)
- [主程序](#主程序)
- [人脸识别服务端](#人脸识别服务端)

# encFace客户端模块

1. 使用MTCNN模型识别图像中的人脸
2. 使用InceptionResnetV1模型提取所有人脸的512位特征向量
3. 将这些特征向量转化为整数
4. 将结果转化为明文形式进行BGV加密
5. 将加密结果使用ZUC加密
6. sm9加密zuc密钥
7. 将算法ID和加密后的数据传输给服务器
8. 接受服务器的信息反馈

## 环境依赖

使用C语言和python作为主要编程语言

| C语言依赖                                             | 描述                                   |
| ---- | ---- |
| [MinGW](https://github.com/niXman/mingw-builds-binaries) | 作为C语言编译器                        |
| [GmSSL](https://gitee.com/YoungWxy/GmSSL)                | 由北京大学自主开发的国产商用密码开源库 |
| [SDL](https://github.com/libsdl-org/SDL)                 | 渲染界面                               |
| [SDL_image](https://github.com/libsdl-org/SDL_image)     | 读取图片                               |
| [SDL_ttf](https://github.com/libsdl-org/SDL_ttf)         | 渲染字体                               |

| python环境                                                  | 描述                                 |
| ---- | ---- |
| [Python 3.12.0](https://www.python.org/downloads/windows/) | 程序运行的稳定python版本 |
| [facenet_pytorch](https://github.com/timesler/facenet-pytorch) | 用于加载MTCNN和InceptionResnetV1模型 |
| [SEAL-Python](https://github.com/Huelse/SEAL-Python)           | 用于实现BGV同态加密                  |

python安装依赖`pip install --upgrade torch numpy opencv-python pillow facenet-pytorch`，SEAL需要单独编译[SEAL-Python](https://github.com/Huelse/SEAL-Python)

## MTCNN和InceptionResnetV1的识别测试

在 `./test/face` 目录下存放了1400张人脸照片，包括200个人，每人7张照片，执行 `python test.py`

## 目录

- examples目录下存放BGV和sm9加密的例子和BGV和SM9公密钥的生成代码
- external目录下存放依赖
- include目录下存放头文件
- src目录下存放源文件
- obj目录存放编译时临时文件
- test目录下存放测试文件
- tmp存放程序运行时的临时文件

# 主程序

主程序使用C，运行makefile编译出encFace.exe即可

# 人脸识别服务端

由于C在运行模型和实现同态加密较为麻烦，所以将人脸特征向量提取和BGV加密部分放在python程序中，在程序运行时会创建一个本地python服务器程序与主程序使用socket进行交互
