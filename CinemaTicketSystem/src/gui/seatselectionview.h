#ifndef SEATSELECTIONVIEW_H
#define SEATSELECTIONVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QList>
#include <QPoint>
#include <QSpinBox>
#include "../core/core_structs.h"

class SeatSelectionView : public QWidget
{
    Q_OBJECT
public:
    explicit SeatSelectionView(QWidget *parent = nullptr);
    void loadHallData(Hall *hall);

signals:
    void requestCheckout(QList<QPoint> seats);
    void requestBackToMovieList();

private:
    QVBoxLayout *seatMainLayout;
    Hall *currentHall;
    
    QList<QPoint> selectedSeats; 
    
    QSpinBox *peopleCountSpin;
    QPushButton *btnSmartSelect;
    
    void executeSmartSelection();
    void renderSeats(); 

    QList<QList<QPoint>> cachedBfsResults; // 记录所有满足条件的连续座位方案
    int currentBfsIndex;                   // 当前展示到第几种方案
    int lastSearchedCount;                 // 记录上一次搜索的人数，如果人数变化则清空缓存重新搜索
};

#endif