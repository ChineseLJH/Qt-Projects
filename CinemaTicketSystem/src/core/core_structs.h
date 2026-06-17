#ifndef CORE_STRUCTS_H
#define CORE_STRUCTS_H

// 影片核心结构体
struct Movie {
    int id;
    char title[64];         // 电影名称
    char posterPath[128];   // 海报图片在硬盘上的相对地址
    char showTime[16];      // 放映时间 (例如 "14:30")
    double price;
    int duration;
    int targetHallId;       // 该电影绑定的影厅 ID
};

// 影厅核心结构体
struct Hall {
    int hallId;
    int totalSeats;         // 50, 100, 或 150
    int rows;
    int cols;
    int **seatMatrix;       // 核心考点：二维指针矩阵。0=空闲, 1=已售, 2=锁定
};

// 订单/票务结构体
struct Ticket {
    char orderId[32];
    int movieId;
    int hallId;
    int seatRow;
    int seatCol;
    double finalPrice;
};

#endif // CORE_STRUCTS_H