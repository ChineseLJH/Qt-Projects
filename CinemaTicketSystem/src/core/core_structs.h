// core_structs.h 示例
struct Movie {
    int id;
    char name[50];          // 严格使用字符数组
    float price;
    int durationMinutes;
};

struct Hall {
    int hallId;
    int capacity;           // 50, 100 或 150
    int rows;
    int cols;
    int **seatMatrix;       // 核心考核点：二维指针数组管理座位状态
};