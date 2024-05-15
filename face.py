import os
import torch
import struct
import socket
import numpy as np
import cv2

from seal import *
from PIL import Image, ImageOps
from facenet_pytorch import MTCNN, InceptionResnetV1

public_key_file = "BGV_public_key"

MSG_TYPE_SIZE       = 8

MSG_CLOSE           = 0
MSG_SUCESS          = 1
MSG_GET_FACE_VECTOR = 2
MSG_GET_FACE_INFO   = 3
MSG_FACE            = 4
MSG_FACE_END        = 5

def read(s, mtcnn, resnet, encryptor, batch_encoder):
    # 接受文件名的长度
    file_len = struct.unpack('Q', s.recv(8))[0]
    
    # 接受文件名
    print("接收文件名")
    file = s.recv(file_len).decode('utf-16')

    # 检查文件是否存在
    if not os.path.exists(file):
        s.sendall(struct.pack('Q', MSG_FACE_END))
        return None
    
    # 读取Image
    img = Image.open(file).convert('RGB')  # 将图像转换为RGB格式
    if img is None:
        s.sendall(struct.pack('Q', MSG_FACE_END))
        return None
    
    # 对图片预处理
    image = ImageOps.autocontrast(img)  # 自动对比度
    image = ImageOps.equalize(image)    # 直方图均衡化

    # 将PIL图像转换为OpenCV图像
    image = cv2.cvtColor(np.array(image), cv2.COLOR_RGB2BGR)

    # 使用MTCNN检测人脸
    boxes, _ = mtcnn.detect(image)
    if boxes is None:
        s.sendall(struct.pack('Q', MSG_FACE_END))
        return None
    
    # 向文件中写入特征向量的大小、box的数值和特征向量的值
    count = 0
    for box in boxes:
        # 剪切人脸图像
        face = img.crop(box)

        # 检查face是否有效
        if face is None or face.size[0] <= 10 or face.size[1] <= 10:
            continue

        try:
            face_tensor = mtcnn(face)
        except:
            continue

        # 检查face_tensor是否有效
        if face_tensor is None:
            continue
        
        print("正在处理第" + str(count+1) + "个人脸")
        
        # 发送人脸消息
        s.sendall(struct.pack('Q', MSG_FACE))
        
        # 发送box数据
        s.sendall(struct.pack('4f', box[0], box[1], box[2], box[3]))

        # 使用InceptionResnetV1来获取人脸的特征向量
        embedding   = resnet(face_tensor.unsqueeze(0))
        
        # 首先将所有特征向量乘以1000000并转换为整数
        # 然后将向量编码为明文
        x_plain     = batch_encoder.encode(embedding.mul_(1000000).to(torch.int64).view(-1))

        # 加密明文
        x_cipher    = encryptor.encrypt(x_plain)
        
        # 将数据序列化为字节
        with open("tmp\\tmp", "wb") as vf:
            vf.close()
            x_cipher.save(vf.name)
        f = open("tmp\\tmp", "rb")
        x_cipher_bytes = f.read()
        f.close()

        # 发送数据大小
        s.sendall(struct.pack('Q', len(x_cipher_bytes)))
        
        # 发送数据
        s.sendall(x_cipher_bytes)
        
        count += 1
            
    s.sendall(struct.pack('Q', MSG_FACE_END))
    print('识别完成')
    
    if os.path.exists("tmp\\tmp"):
        os.remove("tmp\\tmp")

def main():
    
    # 创建一个MTCNN对象，用于人脸检测
    mtcnn = MTCNN()
    
    # 创建一个InceptionResnetV1对象，用于人脸识别
    resnet = InceptionResnetV1(pretrained='vggface2').eval()
    
    # 初始化BGV
    parms = EncryptionParameters (scheme_type.bgv)
    poly_modulus_degree = 8192
    parms.set_poly_modulus_degree(poly_modulus_degree)
    parms.set_coeff_modulus(CoeffModulus.BFVDefault(poly_modulus_degree))
    parms.set_plain_modulus(PlainModulus.Batching(poly_modulus_degree, 20))
    context = SEALContext(parms)
    
    # 读取公钥
    public_key = PublicKey()
    public_key.load(context, public_key_file)
    
    # 创建加密器
    encryptor = Encryptor(context, public_key)
    
    # 创建批处理编码器
    batch_encoder = BatchEncoder(context) # 批处理编码器

    # 创建一个socket对象
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # 连接服务器
    s.connect(("127.0.0.1", 8081))
    
    # 检查连接结果
    if socket is None:
        print("连接服务器失败")
        return
    
    while True:
        # 接收客户端数据
        flag = struct.unpack('Q', s.recv(8, socket.MSG_WAITALL))
        
        # 如果flag为0，表示客户端请求关闭连接
        if flag[0] == MSG_CLOSE:
            break
        
        # 如果flag为1，表示客户端请求处理人脸
        if flag[0] == MSG_GET_FACE_VECTOR:
            read(s, mtcnn, resnet, encryptor, batch_encoder)
        
    # 关闭服务器套接字
    s.close()

main()