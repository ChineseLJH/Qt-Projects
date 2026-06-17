#ifndef SEATSELECTIONVIEW_H
#define SEATSELECTIONVIEW_H

#include <QWidget>
#include <QGridLayout>
#include <QPushButton> // 确保引入 QPushButton
#include "../core/core_structs.h"

class SeatSelectionView : public QWidget
{
    Q_OBJECT

public:
    explicit SeatSelectionView(QWidget *parent = nullptr);
    void loadHallData(Hall *hall);

signals:
    // 修改信号签名，强制要求携带物理内存坐标传递给主路由
    void requestCheckout(int row, int col); 

private:
    QGridLayout *seatGridLayout;
    Hall *currentHall;
    
    // 新增：用于记录当前选中的坐标，-1 表示未选择
    int selectedRow;
    int selectedCol;
    
    // 新增：记录当前被选中按钮的堆内存地址，用于状态重置
    QPushButton *currentSelectedBtn; 
};

#endif // SEATSELECTIONVIEW_H