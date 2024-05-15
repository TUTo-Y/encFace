import numpy as np
import torch
from PIL import Image, ImageOps
from facenet_pytorch import MTCNN, InceptionResnetV1
from scipy.spatial import distance

# 阈值
threshold = 0.9

# 存储人脸特征向量
face_embedding = []

# 人数
people_num = 200
# 每个人的照片数量
photo_num = 7

# 未检出出的人脸数量
no_face = 0

# 相同人脸对比总次数
total_same_compare = 0
# 不同人脸对比总次数
total_diff_compare = 0

# 不同数量的人脸对比成功次数
part_same_compare = 0
# 相同数量的人脸对比成功次数
part_diff_compare = 0

# 相同人的欧氏距离
O_same = []
# 不同人的欧氏距离
O_diff = []

# 创建MTCNN模型
mtcnn = MTCNN()
# 创建InceptionResnetV1模型
resnet = InceptionResnetV1(pretrained='vggface2').eval()


def is_same_person(embedding1, embedding2):
    global threshold
    # 计算两个特征向量之间的欧氏距离
    dist = distance.euclidean(embedding1, embedding2)
    # 如果距离小于阈值，则认为是同一个人
    return dist/1000000.0 < threshold

def read_image(img_path):
    global no_face

    # 读取图片
    img = Image.open(img_path).convert('RGB')
    
    # 对图片预处理
    image = ImageOps.autocontrast(img)  # 自动对比度
    image = ImageOps.equalize(image)    # 直方图均衡化

    # 使用MTCNN进行人脸检测
    face = mtcnn(img)

    # 检查是否检测到人脸
    if face is not None:
        # 添加一个额外的维度
        face = face.unsqueeze(0)

        # 提取人脸特征向量
        embedding = resnet(face).mul_(1000000).to(torch.int64).view(-1).detach().numpy()
        
        return embedding
    else:
        print(f"未检测到人脸 {img_path}")
        no_face += 1
        return [0 * 512]


# 提取所有人的特征向量
for i in range(1, people_num + 1):
    print('正在提取第' + str(i) + '个人的人脸特征')
    for j in range(1, photo_num + 1):
        face_embedding.append(read_image(f'./FERET-{i:03}/{j:02}.tif'))

# 对比所有相同人脸
print('对比所有相同人脸')
for i in range(1, people_num + 1):
    print('正在对比第' + str(i) + '个人的所有相同人脸')
    for j in range(1, photo_num):
        for k in range(j+1, photo_num + 1):
            # 确保两个特征向量都存在
            if(not np.array_equal(face_embedding[(i-1)*photo_num + j-1], [0 * 512]) and not np.array_equal(face_embedding[(i-1)*photo_num + k-1], [0 * 512])):
                
                # 获取两个特征向量的欧氏距离
                dist = distance.euclidean(face_embedding[(i-1)*photo_num + j-1], face_embedding[(i-1)*photo_num + k-1])/1000000.0
                # 添加
                O_same.append(dist)
                # 如果距离小于阈值，则认为是同一个人
                if dist < threshold:
                    part_same_compare += 1
                total_same_compare += 1
                
# 对比所有不同人脸
print('对比所有不同人脸')
for i in range(1, people_num):
    print('正在对比第' + str(i) + '个人的所有不同人脸')
    for j in range(1, photo_num + 1):
        for k in range(i+1, people_num + 1):
            for l in range(1, photo_num + 1):
                if(not np.array_equal(face_embedding[(i-1)*photo_num + j-1], [0 * 512]) and not np.array_equal(face_embedding[(k-1)*photo_num + l-1], [0 * 512])):
                    
                    # 获取两个特征向量的欧氏距离
                    dist = distance.euclidean(face_embedding[(i-1)*photo_num + j-1], face_embedding[(k-1)*photo_num + l-1])/1000000.0
                    # 添加
                    O_diff.append(dist)
                    # 如果距离大于阈值，则认为是不同的人
                    if dist >= threshold:
                        part_diff_compare += 1
                    total_diff_compare += 1

print('\n')

# 相同人的欧氏距离
print('相同人的欧氏距离对比结果:')
print('最小值:', min(O_same))
print('最大值:', max(O_same))
print('平均值:', sum(O_same) / len(O_same))
print('0.0 <= 欧氏距离 < 0.1 : {:.3f}%'.format(100 * len([d for d in O_same if 0.0 <= d < 0.1]) / len(O_same)))
print('0.1 <= 欧氏距离 < 0.2 : {:.3f}%'.format(100 * len([d for d in O_same if 0.1 <= d < 0.2]) / len(O_same)))
print('0.2 <= 欧氏距离 < 0.3 : {:.3f}%'.format(100 * len([d for d in O_same if 0.2 <= d < 0.3]) / len(O_same)))
print('0.3 <= 欧氏距离 < 0.4 : {:.3f}%'.format(100 * len([d for d in O_same if 0.3 <= d < 0.4]) / len(O_same)))
print('0.4 <= 欧氏距离 < 0.5 : {:.3f}%'.format(100 * len([d for d in O_same if 0.4 <= d < 0.5]) / len(O_same)))
print('0.5 <= 欧氏距离 < 0.6 : {:.3f}%'.format(100 * len([d for d in O_same if 0.5 <= d < 0.6]) / len(O_same)))
print('0.6 <= 欧氏距离 < 0.7 : {:.3f}%'.format(100 * len([d for d in O_same if 0.6 <= d < 0.7]) / len(O_same)))
print('0.7 <= 欧氏距离 < 0.8 : {:.3f}%'.format(100 * len([d for d in O_same if 0.7 <= d < 0.8]) / len(O_same)))
print('0.8 <= 欧氏距离 < 0.9 : {:.3f}%'.format(100 * len([d for d in O_same if 0.8 <= d < 0.9]) / len(O_same)))
print('0.9 <= 欧氏距离 < 1.0 : {:.3f}%'.format(100 * len([d for d in O_same if 0.9 <= d < 1.0]) / len(O_same)))
print('1.0 <= 欧氏距离 < 1.1 : {:.3f}%'.format(100 * len([d for d in O_same if 1.0 <= d < 1.1]) / len(O_same)))
print('1.1 <= 欧氏距离 < 1.2 : {:.3f}%'.format(100 * len([d for d in O_same if 1.1 <= d < 1.2]) / len(O_same)))
print('1.2 <= 欧氏距离 < 1.3 : {:.3f}%'.format(100 * len([d for d in O_same if 1.2 <= d < 1.3]) / len(O_same)))
print('1.3 <= 欧氏距离 < 1.4 : {:.3f}%'.format(100 * len([d for d in O_same if 1.3 <= d < 1.4]) / len(O_same)))
print('1.4 <= 欧氏距离 < 1.5 : {:.3f}%'.format(100 * len([d for d in O_same if 1.4 <= d < 1.5]) / len(O_same)))
print('1.5 <= 欧氏距离 < 1.6 : {:.3f}%'.format(100 * len([d for d in O_same if 1.5 <= d < 1.6]) / len(O_same)))
print('1.6 <= 欧氏距离 < 1.7 : {:.3f}%'.format(100 * len([d for d in O_same if 1.6 <= d < 1.7]) / len(O_same)))
print('1.7 <= 欧氏距离 < 1.8 : {:.3f}%'.format(100 * len([d for d in O_same if 1.7 <= d < 1.8]) / len(O_same)))
print('1.8 <= 欧氏距离 < 1.9 : {:.3f}%'.format(100 * len([d for d in O_same if 1.8 <= d < 1.9]) / len(O_same)))
print('1.9 <= 欧氏距离 < 2.0 : {:.3f}%'.format(100 * len([d for d in O_same if 1.9 <= d < 2.0]) / len(O_same)))
print('\n')

# 相同人的欧氏距离
print('不同人的欧氏距离对比结果:')
print('最小值:', min(O_diff))
print('最大值:', max(O_diff))
print('平均值:', sum(O_diff) / len(O_diff))
print('0.0 <= 欧氏距离 < 0.1 : {:.3f}%'.format(100 * len([d for d in O_diff if 0.0 <= d < 0.1]) / len(O_same)))
print('0.1 <= 欧氏距离 < 0.2 : {:.3f}%'.format(100 * len([d for d in O_diff if 0.1 <= d < 0.2]) / len(O_same)))
print('0.2 <= 欧氏距离 < 0.3 : {:.3f}%'.format(100 * len([d for d in O_diff if 0.2 <= d < 0.3]) / len(O_same)))
print('0.3 <= 欧氏距离 < 0.4 : {:.3f}%'.format(100 * len([d for d in O_diff if 0.3 <= d < 0.4]) / len(O_same)))
print('0.4 <= 欧氏距离 < 0.5 : {:.3f}%'.format(100 * len([d for d in O_diff if 0.4 <= d < 0.5]) / len(O_same)))
print('0.5 <= 欧氏距离 < 0.6 : {:.3f}%'.format(100 * len([d for d in O_diff if 0.5 <= d < 0.6]) / len(O_same)))
print('0.6 <= 欧氏距离 < 0.7 : {:.3f}%'.format(100 * len([d for d in O_diff if 0.6 <= d < 0.7]) / len(O_same)))
print('0.7 <= 欧氏距离 < 0.8 : {:.3f}%'.format(100 * len([d for d in O_diff if 0.7 <= d < 0.8]) / len(O_same)))
print('0.8 <= 欧氏距离 < 0.9 : {:.3f}%'.format(100 * len([d for d in O_diff if 0.8 <= d < 0.9]) / len(O_diff)))
print('0.9 <= 欧氏距离 < 1.0 : {:.3f}%'.format(100 * len([d for d in O_diff if 0.9 <= d < 1.0]) / len(O_diff)))
print('1.0 <= 欧氏距离 < 1.1 : {:.3f}%'.format(100 * len([d for d in O_diff if 1.0 <= d < 1.1]) / len(O_diff)))
print('1.1 <= 欧氏距离 < 1.2 : {:.3f}%'.format(100 * len([d for d in O_diff if 1.1 <= d < 1.2]) / len(O_diff)))
print('1.2 <= 欧氏距离 < 1.3 : {:.3f}%'.format(100 * len([d for d in O_diff if 1.2 <= d < 1.3]) / len(O_diff)))
print('1.3 <= 欧氏距离 < 1.4 : {:.3f}%'.format(100 * len([d for d in O_diff if 1.3 <= d < 1.4]) / len(O_diff)))
print('1.4 <= 欧氏距离 < 1.5 : {:.3f}%'.format(100 * len([d for d in O_diff if 1.4 <= d < 1.5]) / len(O_diff)))
print('1.5 <= 欧氏距离 < 1.6 : {:.3f}%'.format(100 * len([d for d in O_diff if 1.5 <= d < 1.6]) / len(O_diff)))
print('1.6 <= 欧氏距离 < 1.7 : {:.3f}%'.format(100 * len([d for d in O_diff if 1.6 <= d < 1.7]) / len(O_diff)))
print('1.7 <= 欧氏距离 < 1.8 : {:.3f}%'.format(100 * len([d for d in O_diff if 1.7 <= d < 1.8]) / len(O_diff)))
print('1.8 <= 欧氏距离 < 1.9 : {:.3f}%'.format(100 * len([d for d in O_diff if 1.8 <= d < 1.9]) / len(O_diff)))
print('1.9 <= 欧氏距离 < 2.0 : {:.3f}%'.format(100 * len([d for d in O_diff if 1.9 <= d < 2.0]) / len(O_diff)))
print('2.0 <= 欧氏距离 < 2.1 : {:.3f}%'.format(100 * len([d for d in O_diff if 2.0 <= d < 2.1]) / len(O_diff)))
print('2.1 <= 欧氏距离 < 2.2 : {:.3f}%'.format(100 * len([d for d in O_diff if 2.1 <= d < 2.2]) / len(O_diff)))
print('2.2 <= 欧氏距离 < 2.3 : {:.3f}%'.format(100 * len([d for d in O_diff if 2.2 <= d < 2.3]) / len(O_diff)))
print('2.3 <= 欧氏距离 < 2.4 : {:.3f}%'.format(100 * len([d for d in O_diff if 2.3 <= d < 2.4]) / len(O_diff)))

# 总
print('当阈值为' + str(threshold) + '时:')

print('\n')
print('MTCNN检测结果:')
print('{:<20}{} / {}'.format('未检测到人脸数量:', no_face, (people_num * photo_num) ))
print('{:<20}{:.3f}%'.format('检测人脸的准确率:', (people_num * photo_num - no_face) / (people_num * photo_num) * 100))

print('\n')
print('InceptionResnetV1对比结果:')
print('{:<20}{} / {}'.format('相同人脸对比成功次数:', part_same_compare, total_same_compare))
print('{:<20}{} / {}'.format('不同人脸对比成功次数:', part_diff_compare, total_diff_compare))
print('{:<20}{} / {}'.format('全部人脸对比成功次数:', part_same_compare + part_diff_compare, total_same_compare + total_diff_compare))
print('{:<20}{:.3f}%'.format('全部人脸对比的成功率:', (part_same_compare + part_diff_compare) / (total_same_compare + total_diff_compare) * 100))

