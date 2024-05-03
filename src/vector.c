#include "vector.h"

/**
 * \brief 从图片中提取特征向量
 * \param image 图片
 * \param head 链表头指针
 * \return 是否成功提取特征向量
 */
bool get_face_vector(const char *image, list **head)
{
    vector vec = {0};
    FILE *file = NULL;
    FILE *boxfile = NULL;
    int count = 0;
    size_t size = 0;
    int ret = 0;
    char cmd[PATH_MAX + 0x10] = {0};

    // 检查参数
    CHECK(image && head, "提取特征向量时传参错误\n");

    // 检测是否存在tmp目录
    if (access(TMP, F_OK) == -1)
    {
        // 如果文件夹不存在，创建它
        ret = mkdir(TMP);
        CHECK(ret != -1, "无法创建tmp目录\n");
    }

    // 删除旧文件
    ret = remove(SAVE_FILE);
    CHECK(ret == 0 || ENOENT == errno, "无法删除旧文件，请检查是否正在使用\n");
    ret = remove(BOX_FILE);
    CHECK(ret == 0 || ENOENT == errno, "无法删除旧文件，请检查是否正在使用\n");

    // 创建命令
    sprintf(cmd, "python face.py \"%s\"", image);

    // 执行
    ret = system(cmd);
    CHECK(ret == 0, "无法执行命令\n"); // 检查是否执行成功

    // 读取识别数据
    file = fopen(SAVE_FILE, "rb");
    CHECK(file, "读取save文件失败\n");

    // 检测识别结果并读取人物count
    char result[0x10] = {0};
    fread(result, 1, SIG_SIZE, file); // 读取sig
    CHECK(strncmp(result, NOT_FACE, SIG_SIZE) || strncmp(result, NOT_IMAGE, SIG_SIZE) || strncmp(result, NOT_FILE, SIG_SIZE),
          "读取到错误解析结果: %s\n", result);
    fread(&count, 1, sizeof(count), file); // 读取count
    CHECK(count > 0, "未检测到人脸\n");
    fclose(file);

    // 打开box文件
    boxfile = fopen(BOX_FILE, "rb");
    CHECK(boxfile, "无法读取box文件\n");

    // 读取box文件和vector文件
    for (int i = 0; i < count; i++)
    {
        memset(&vec, 0, sizeof(vec));
        // gui test测试
        // vec.flag = rand() % 3;
        // strcpy(vec.info.name, "郑德泓666");
        // vec.info.age = rand() % 100;
        // vec.info.sex = rand() % 3;

        // 读取位置
        fread(&vec.rect, 1, sizeof(vec.rect), boxfile);
        // 修改位置为SDL_FRect
        vec.rect.w -= vec.rect.x;
        vec.rect.h -= vec.rect.y;

        // 读取特征向量
        sprintf(cmd, "%s%d", VER_FILE, i);
        file = fopen(cmd, "rb");
        CHECK(file, "无法读取加密的特征向量文件\n");
        fseek(file, 0, SEEK_END);
        size = ftell(file);
        fseek(file, 0, SEEK_SET);
        vec.v = Malloc(size);
        CHECK(vec.v, "malloc failed\n");
        fread(vec.v->data, 1, size, file);
        fclose(file);
        remove(cmd);

        // 写入链表
        CHECK(addData(head, &vec, sizeof(vec), true) == true, "添加链表失败\n");
    }
    fclose(boxfile);

    remove(SAVE_FILE);
    remove(BOX_FILE);
    return true;

error:
    if (file)
        fclose(file);
    if (boxfile)
        fclose(boxfile);
    freeList(head, (void (*)(void *))freeVector);
    remove(SAVE_FILE);
    remove(BOX_FILE);
    return false;
}

/**
 * \brief 释放vec数据
 */
void freeVector(vector *vec)
{
    if (vec)
    {
        if (vec->v)
            Free(vec->v);
        free(vec);
    }
}