import sys
import os
import torch
import struct
import numpy as np

from seal import *
from PIL import Image
from phe import paillier
from facenet_pytorch import MTCNN, InceptionResnetV1

public_key_file = "BGV_public_key"

save_file = "tmp\\out"      # 保存输出结果的文件
box_file  = "tmp\\box"      # 保存box的文件
ver_file  = "tmp\\vector"   # 保存密文的文件
sucess    = b"sucess  "
not_face  = b"no face "
not_image = b"no image"
not_file  = b"no file "

# 写入msg
def write_to_file(filename, content):
    with open(filename, 'wb') as f:
        f.write(content)

def read(image_file):
    # 检查文件是否存在
    if not os.path.exists(image_file):
        write_to_file(save_file, not_file)
        return None
    if os.path.exists(save_file):
        os.remove(save_file)
    
    # 使用MTCNN来检测人脸
    img = Image.open(image_file).convert('RGB')  # 将图像转换为RGB格式
    # 检查img是否有效
    if img is None:
        write_to_file(save_file, not_image)
        return None
    
    # 创建一个MTCNN对象，用于人脸检测
    mtcnn = MTCNN()
    # 创建一个InceptionResnetV1对象，用于人脸识别
    resnet = InceptionResnetV1(pretrained='vggface2').eval()
    boxes, _ = mtcnn.detect(img)
    # 如果没有检测到人脸，返回None
    if boxes is None:
        write_to_file(save_file, not_face)
        return None
    
    # 使用BGV加密
    parms = EncryptionParameters (scheme_type.bgv)
    poly_modulus_degree = 2048 # 设置多项式模数的度(不知道为什么一修改这个值就报错，但是使用的数据是512个，剩下的填充为0，这就导致了解密后的数组长度为512)
    parms.set_poly_modulus_degree(poly_modulus_degree)
    parms.set_coeff_modulus(CoeffModulus.BFVDefault(poly_modulus_degree))
    # parms.set_coeff_modulus(CoeffModulus.Create(poly_modulus_degree, [60, 40, 40, 40, 40, 60]))
    parms.set_plain_modulus(PlainModulus.Batching(poly_modulus_degree, 40))
    context = SEALContext(parms) # 创建SEAL上下文
    
    # 读取公钥
    public_key = PublicKey()
    public_key.load(context, public_key_file)
    
    # 创建加密器
    encryptor = Encryptor(context, public_key)
    
    # 创建批处理编码器
    batch_encoder = BatchEncoder(context) # 批处理编码器
    
    # 向文件中写入特征向量的大小、box的数值和特征向量的值
    with open(save_file, 'wb') as sf:
        with open(box_file, 'wb') as bf:
            sf.write(sucess)
            count = 0
            
            for box in boxes:
                # 剪切人脸图像
                face = img.crop(box)
                # 检查face是否有效
                if face is None or face.size[0] == 0 or face.size[1] == 0:
                    continue
                face_tensor = mtcnn(face)
                # 检查face_tensor是否有效
                if face_tensor is None:
                    continue
                
                print('正在处理第' + str(count + 1) + '个人脸')
                
                # 保存box数据
                bf.write(struct.pack('4f', box[0], box[1], box[2], box[3]))

                # 使用InceptionResnetV1来获取人脸的特征向量
                embedding   = resnet(face_tensor.unsqueeze(0))

                # 首先将所有特征向量乘以1000000并转换为整数
                # 然后将向量编码为明文
                x_plain     = batch_encoder.encode(embedding.mul_(1000000).to(torch.int64).view(-1))

                # 加密明文
                x_cipher    = encryptor.encrypt(x_plain)

                # 保存加密后的数据
                with open(ver_file + str(count), "w") as vf:
                    x_cipher.save(vf.name)

                # 人物数量增加
                count += 1
                
            # 写入总数
            sf.write(struct.pack('i', count))

def main():
    # 检查参数
    if(len(sys.argv) == 1):
        write_to_file(save_file, not_file)
        return
    image_file = sys.argv[1]
    read(image_file)

if __name__ == "__main__":
    main()