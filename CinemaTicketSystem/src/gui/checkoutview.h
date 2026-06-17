#ifndef CHECKOUTVIEW_H
#define CHECKOUTVIEW_H

#include <QWidget>
#include <QLabel>
#include <QList>
#include <QPoint>
#include "../core/core_structs.h"

class CheckoutView : public QWidget
{
    Q_OBJECT

public:
    explicit CheckoutView(QWidget *parent = nullptr);
    
    // 提供给总控器的内存注入接口
    void loadOrderData(Movie *movie, Hall *hall, QList<QPoint> seats);

signals:
    void requestBackToSeatSelection();
    void requestReturnHome(QList<QPoint> confirmedSeats); // 确认支付时返回坐标

private:
    QLabel *titleLabel; // 将其提升为成员变量以便修改文本
    QList<QPoint> pendingSeats;
};

#endif // CHECKOUTVIEW_H