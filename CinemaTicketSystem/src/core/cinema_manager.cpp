#include "cinema_manager.h"
#include <cstring> // 用于 strcpy 等 C 标准库字符操作

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

void CinemaManager::initializeData() {
    // 分配 3 个影厅的连续内存
    halls = new Hall[hallCount];

    // 初始化一号厅 (50座：5行10列)
    halls[0].hallId = 1;
    halls[0].totalSeats = 50;
    halls[0].rows = 5;
    halls[0].cols = 10;
    halls[0].seatMatrix = allocateSeatMatrix(5, 10);

    // 初始化二号厅 (100座：10行10列)
    halls[1].hallId = 2;
    halls[1].totalSeats = 100;
    halls[1].rows = 10;
    halls[1].cols = 10;
    halls[1].seatMatrix = allocateSeatMatrix(10, 10);

    // 初始化三号厅 (150座：10行15列)
    halls[2].hallId = 3;
    halls[2].totalSeats = 150;
    halls[2].rows = 10;
    halls[2].cols = 15;
    halls[2].seatMatrix = allocateSeatMatrix(10, 15);

    // 模拟写入部分已售状态 (例如一号厅的第3行第4列已售)
    // 底层寻址等价于 *(*(halls[0].seatMatrix + 2) + 3) = 1;
    halls[0].seatMatrix[2][3] = 1; 

    // 分配并初始化电影数据 (严格使用字符数组和 strncpy)
    movieCount = 2;
    movies = new Movie[movieCount];
    
    movies[0].id = 101;
    strncpy(movies[0].title, "Inception", sizeof(movies[0].title) - 1);
    movies[0].price = 45.0;
    movies[0].duration = 148;

    movies[1].id = 102;
    strncpy(movies[1].title, "The Matrix", sizeof(movies[1].title) - 1);
    movies[1].price = 40.0;
    movies[1].duration = 136;
}

Hall* CinemaManager::getHallById(int id) {
    for (int i = 0; i < hallCount; ++i) {
        if (halls[i].hallId == id) {
            return &halls[i]; // 返回对应结构体的内存地址
        }
    }
    return nullptr;
}

// 暴露底层的电影数组指针
Movie* CinemaManager::getMovies() {
    return movies;
}

// 暴露电影数组的长度
int CinemaManager::getMovieCount() {
    return movieCount;
}