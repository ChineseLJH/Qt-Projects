#ifndef CINEMA_MANAGER_H
#define CINEMA_MANAGER_H

#include "core_structs.h" // 引入我们之前定义的 Movie, Hall, Ticket 结构体

class CinemaManager {
public:
    CinemaManager();
    ~CinemaManager();

    // 初始化测试数据（3个影厅与排片）
    void initializeData(const char* exeDirPath);

    // 提供对底层结构体的只读/读写指针访问
    Hall* getHallById(int id);
    Movie* getMovies();
    int getMovieCount();

private:
    Hall *halls;        // 指向影厅数组首地址的指针
    int hallCount;

    Movie *movies;      // 指向电影数组首地址的指针
    int movieCount;

    // 核心考点：纯 C 语言风格的二维指针数组动态分配与释放
    int** allocateSeatMatrix(int rows, int cols);
    void freeSeatMatrix(int **matrix, int rows);
};

#endif // CINEMA_MANAGER_H