#include "cinema_manager.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>

CinemaManager::CinemaManager() : halls(nullptr), hallCount(3), movies(nullptr), movieCount(0) {}

CinemaManager::~CinemaManager() {
    if (halls != nullptr) {
        for (int i = 0; i < hallCount; ++i) {
            freeSeatMatrix(halls[i].seatMatrix, halls[i].rows);
            delete[] halls[i].colsPerRow; // 释放新增加的列宽记录数组
        }
        delete[] halls;
    }
    if (movies != nullptr) delete[] movies;
}

// 核心改动 1：根据传入的每行列数数组，动态分配非规则二维矩阵
int** CinemaManager::allocateSeatMatrix(int rows, int *colsPerRow) {
    int **matrix = new int*[rows];
    for (int i = 0; i < rows; ++i) {
        matrix[i] = new int[colsPerRow[i]];
        for (int j = 0; j < colsPerRow[i]; ++j) {
            matrix[i][j] = 0; // 0代表空闲
        }
    }
    return matrix;
}

void CinemaManager::freeSeatMatrix(int **matrix, int rows) {
    if (matrix == nullptr) return;
    for (int i = 0; i < rows; ++i) delete[] matrix[i];
    delete[] matrix;
}

void CinemaManager::initializeData(const char* exeDirPath) {
    hallCount = 3;
    halls = new Hall[hallCount];
    
    // 初始化一号厅：构造一个靠近荧幕少，远离荧幕多的梯形（扇形）内存结构
    halls[0].hallId = 1; halls[0].totalSeats = 50; halls[0].rows = 5;
    halls[0].colsPerRow = new int[5]{6, 8, 10, 12, 14}; // 累加正好 50 座
    halls[0].seatMatrix = allocateSeatMatrix(5, halls[0].colsPerRow);

    // 二号厅和三号厅同理，这里简写测试
    halls[1].hallId = 2; halls[1].totalSeats = 100; halls[1].rows = 5;
    halls[1].colsPerRow = new int[5]{16, 18, 20, 22, 24}; 
    halls[1].seatMatrix = allocateSeatMatrix(5, halls[1].colsPerRow);

    halls[2].hallId = 3; halls[2].totalSeats = 150; halls[2].rows = 6;
    halls[2].colsPerRow = new int[6]{20, 22, 24, 26, 28, 30}; 
    halls[2].seatMatrix = allocateSeatMatrix(6, halls[2].colsPerRow);

    // 加载电影数据 (与之前相同)
    char dbPath[512];
    snprintf(dbPath, sizeof(dbPath), "%s/assets/database.txt", exeDirPath);
    FILE *file = fopen(dbPath, "r");
    if (file) {
        movieCount = 0;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), file)) if (strlen(buffer) > 2) movieCount++;
        movies = new Movie[movieCount];
        rewind(file);
        int index = 0;
        while (fgets(buffer, sizeof(buffer), file)) {
            if (strlen(buffer) <= 2) continue;
            sscanf(buffer, "%d,%[^,],%[^,],%[^,],%lf,%d,%d",
                   &movies[index].id, movies[index].title, movies[index].posterPath,
                   movies[index].showTime, &movies[index].price, &movies[index].duration,
                   &movies[index].targetHallId);
            index++;
        }
        fclose(file);
    }

    // 核心改动 2：加载座位持久化文件
    char seatPath[512];
    snprintf(seatPath, sizeof(seatPath), "%s/assets/seats.txt", exeDirPath);
    FILE *seatFile = fopen(seatPath, "r");
    if (seatFile) {
        char sBuffer[128];
        while (fgets(sBuffer, sizeof(sBuffer), seatFile)) {
            int hId, r, c;
            if (sscanf(sBuffer, "%d,%d,%d", &hId, &r, &c) == 3) {
                // 找到对应的影厅内存块，修改其底层状态
                for(int i=0; i<hallCount; i++){
                    if(halls[i].hallId == hId && r < halls[i].rows && c < halls[i].colsPerRow[r]){
                        halls[i].seatMatrix[r][c] = 1; // 标记为已售
                    }
                }
            }
        }
        fclose(seatFile);
    }
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

// 快速排序：分区基准逻辑
int CinemaManager::partitionSeats(SeatRecord* arr, int low, int high) {
    SeatRecord pivot = arr[high]; // 选取末尾元素作为基准点
    int i = low - 1;              // 维护一个小于基准点的连续区间边界
    
    for (int j = low; j < high; j++) {
        bool isLess = false;
        // 多级权重比较：场次 ID 优先，行号次之，列号最后
        if (arr[j].hallId < pivot.hallId) {
            isLess = true;
        } else if (arr[j].hallId == pivot.hallId) {
            if (arr[j].row < pivot.row) {
                isLess = true;
            } else if (arr[j].row == pivot.row && arr[j].col < pivot.col) {
                isLess = true;
            }
        }
        
        // 如果当前节点值更小，将其交换至前半区
        if (isLess) {
            i++;
            SeatRecord temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    // 将基准点归位到分界线处
    SeatRecord temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    return i + 1;
}

// 快速排序：递归调用栈
void CinemaManager::quickSortSeats(SeatRecord* arr, int low, int high) {
    if (low < high) {
        int pi = partitionSeats(arr, low, high);
        quickSortSeats(arr, low, pi - 1);  // 递归处理左半区
        quickSortSeats(arr, pi + 1, high); // 递归处理右半区
    }
}

// I/O 写入与状态同步
void CinemaManager::saveBookedSeats(int hallId, const int* rows, const int* cols, int count, const char* exeDirPath) {
    char seatPath[512];
    snprintf(seatPath, sizeof(seatPath), "%s/assets/seats.txt", exeDirPath);

    // 1. 扫描物理文件获取已有记录总数
    int existingCount = 0;
    FILE* file = fopen(seatPath, "r");
    if (file) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), file)) existingCount++;
        fclose(file);
    }

    // 2. 动态分配内存以容纳 全量数据
    int totalCount = existingCount + count;
    if (totalCount == 0) return;
    SeatRecord* records = new SeatRecord[totalCount];

    // 3. 将物理文件内容映射到堆内存
    int index = 0;
    file = fopen(seatPath, "r");
    if (file) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), file)) {
            sscanf(buffer, "%d,%d,%d", &records[index].hallId, &records[index].row, &records[index].col);
            index++;
        }
        fclose(file);
    }

    // 4. 追加当前次操作产生的新座位数据，并同步修改底层的二维指针矩阵状态
    Hall* hall = getHallById(hallId);
    for (int i = 0; i < count; i++) {
        records[index].hallId = hallId;
        records[index].row = rows[i];
        records[index].col = cols[i];
        
        // 实时更新当前运行实例的内存矩阵标识
        if (hall && rows[i] < hall->rows && cols[i] < hall->colsPerRow[rows[i]]) {
            hall->seatMatrix[rows[i]][cols[i]] = 1; 
        }
        index++;
    }

    // 5. 对内存中的连续结构体数组执行快速排序
    if (totalCount > 1) {
        quickSortSeats(records, 0, totalCount - 1);
    }

    // 6. 重置文件读写指针，以全量覆盖模式刷入磁盘
    file = fopen(seatPath, "w");
    if (file) {
        for (int i = 0; i < totalCount; i++) {
            fprintf(file, "%d,%d,%d\n", records[i].hallId, records[i].row, records[i].col);
        }
        fclose(file);
    }

    // 7. 必须释放堆区内存，防止内存泄漏
    delete[] records;
}