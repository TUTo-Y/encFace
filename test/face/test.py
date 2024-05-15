import numpy as np
import torch
from PIL import Image, ImageOps
from facenet_pytorch import MTCNN, InceptionResnetV1
from scipy.spatial import distance

# 阈值
threshold = 1.2

# 存储人脸特征向量
face_embedding = []

# 人数
people_num = 6
# 每个人的照片数量
photo_num = 6

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
            if(not np.array_equal(face_embedding[(i-1)*photo_num + j-1], [0 * 512]) and not np.array_equal(face_embedding[(i-1)*photo_num + k-1], [0 * 512])):
                if is_same_person(face_embedding[(i-1)*photo_num + j-1], face_embedding[(i-1)*photo_num + k-1]):
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
                    if not is_same_person(face_embedding[(i-1)*photo_num + j-1], face_embedding[(k-1)*photo_num + l-1]):
                        part_diff_compare += 1
                    total_diff_compare += 1

print('\n')

# 结果
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
