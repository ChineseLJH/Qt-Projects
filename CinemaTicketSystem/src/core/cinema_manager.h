#ifndef CINEMA_MANAGER_H
#define CINEMA_MANAGER_H

#include "core_structs.h"

class CinemaManager {
public:
    CinemaManager();
    ~CinemaManager();

    void initializeData(const char* exeDirPath);
    
    // 新增：接收 UI 层传递的 C 语言裸指针数组并执行持久化
    void saveBookedSeats(int hallId, const int* rows, const int* cols, int count, const char* exeDirPath);

    Hall* getHallById(int id);
    Movie* getMovies();
    int getMovieCount();

private:
    Hall *halls;
    int hallCount;

    Movie *movies;
    int movieCount;

    int** allocateSeatMatrix(int rows, int *colsPerRow);
    void freeSeatMatrix(int **matrix, int rows);

    void quickSortSeats(SeatRecord* arr, int low, int high);
    int partitionSeats(SeatRecord* arr, int low, int high);
};

#endif // CINEMA_MANAGER_H