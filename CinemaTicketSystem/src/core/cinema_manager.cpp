#include "cinema_manager.h"
#include <cstring> // 用于 strcpy 等 C 标准库字符操作
#include <cstdio>  // 引入底层文件 I/O
#include <cstdlib>

CinemaManager::CinemaManager() : halls(nullptr), hallCount(3), movies(nullptr), movieCount(0) {
    // 构造函数中暂不分配大块内存，留给显式的 initializeData 调用
}

CinemaManager::~CinemaManager() {
    // 必须严格释放堆区内存，防止内存泄漏
    if (halls != nullptr) {
        for (int i = 0; i < hallCount; ++i) {
            freeSeatMatrix(halls[i].seatMatrix, halls[i].rows);
        }
        delete[] halls;
    }
    if (movies != nullptr) {
        delete[] movies;
    }
}

// 底层二维指针矩阵分配逻辑
int** CinemaManager::allocateSeatMatrix(int rows, int cols) {
    // 1. 分配一个指针数组，存放每一行的首地址
    int **matrix = new int*[rows];
    
    // 2. 为每一行分配具体的整型数组，并初始化为 0 (空闲)
    for (int i = 0; i < rows; ++i) {
        matrix[i] = new int[cols];
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = 0; 
        }
    }
    return matrix;
}

// 底层二维指针矩阵释放逻辑
void CinemaManager::freeSeatMatrix(int **matrix, int rows) {
    if (matrix == nullptr) return;
    for (int i = 0; i < rows; ++i) {
        delete[] matrix[i]; // 释放列
    }
    delete[] matrix;        // 释放行指针数组
}

void CinemaManager::initializeData(const char* exeDirPath) {
    // 1. 初始化 3 个影厅的内存 (保持原来的代码不变)
    hallCount = 3;
    halls = new Hall[hallCount];
    
    halls[0].hallId = 1; halls[0].totalSeats = 50; halls[0].rows = 5; halls[0].cols = 10;
    halls[0].seatMatrix = allocateSeatMatrix(5, 10);
    halls[1].hallId = 2; halls[1].totalSeats = 100; halls[1].rows = 10; halls[1].cols = 10;
    halls[1].seatMatrix = allocateSeatMatrix(10, 10);
    halls[2].hallId = 3; halls[2].totalSeats = 150; halls[2].rows = 10; halls[2].cols = 15;
    halls[2].seatMatrix = allocateSeatMatrix(10, 15);

    // 2. 动态拼装数据库文件的绝对物理路径
    char dbPath[512];
    snprintf(dbPath, sizeof(dbPath), "%s/assets/database.txt", exeDirPath);

    // 3. 调用操作系统 API 获取文件句柄
    FILE *file = fopen(dbPath, "r");
    if (!file) {
        // 如果文件不存在，分配 0 个内存，防止野指针
        movieCount = 0;
        movies = nullptr;
        return;
    }

    // 4. 第一次扫描：统计文件中的行数（确定需要分配的内存大小）
    movieCount = 0;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strlen(buffer) > 2) movieCount++; 
    }

    // 5. 在堆区动态分配精确大小的结构体数组
    movies = new Movie[movieCount];

    // 6. 将文件指针重置回文件头部，进行第二次扫描并拷贝数据
    rewind(file);
    int index = 0;
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strlen(buffer) <= 2) continue;

        // 利用 sscanf 从字符缓冲流中提取指定格式的数据，写入结构体内存地址
        // %[^,] 表示读取直到遇到逗号为止的所有字符
        sscanf(buffer, "%d,%[^,],%[^,],%[^,],%lf,%d,%d",
               &movies[index].id,
               movies[index].title,
               movies[index].posterPath,
               movies[index].showTime,
               &movies[index].price,
               &movies[index].duration,
               &movies[index].targetHallId);
               
        index++;
    }
    
    // 关闭文件句柄释放系统资源
    fclose(file);
}

// 暴露底层的电影数组指针
Movie* CinemaManager::getMovies() {
    return movies;
}

// 暴露电影数组的长度
int CinemaManager::getMovieCount() {
    return movieCount;
}

Hall* CinemaManager::getHallById(int id) {
    if (halls == nullptr) return nullptr;
    
    for (int i = 0; i < hallCount; ++i) {
        if (halls[i].hallId == id) {
            return &halls[i]; // 取出该结构体实例在堆区中的首地址并返回
        }
    }
    return nullptr; // 未命中时返回空指针防止越界
}