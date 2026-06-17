#ifndef CHECKOUTVIEW_H
#define CHECKOUTVIEW_H

#include <QWidget>
#include <QLabel>
#include <QList>
#include <QPoint>
#include <QFrame> // 引入 QFrame 用于制作票据卡片
#include "../core/core_structs.h"

class CheckoutView : public QWidget
{
    Q_OBJECT
public:
    explicit CheckoutView(QWidget *parent = nullptr);
    void loadOrderData(Movie *movie, Hall *hall, QList<QPoint> seats);

signals:
    void requestBackToSeatSelection();
    void requestReturnHome(QList<QPoint> confirmedSeats);

private:
    // UI 组件指针
    QLabel *movieTitleLabel;
    QLabel *infoLabel;      // 放映时间、影厅
    QLabel *seatsLabel;     // 具体座位
    QLabel *priceLabel;     // 总价
    QLabel *posterLabel;    // 缩略海报
    
    QList<QPoint> pendingSeats;
};

#endif