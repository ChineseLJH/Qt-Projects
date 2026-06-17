#ifndef CHECKOUTVIEW_H
#define CHECKOUTVIEW_H

#include <QWidget>
#include <QLabel>
#include "../core/core_structs.h"

class CheckoutView : public QWidget
{
    Q_OBJECT

public:
    explicit CheckoutView(QWidget *parent = nullptr);
    
    // 提供给总控器的内存注入接口
    void loadOrderData(Movie *movie, Hall *hall, int row, int col);

signals:
    void requestBackToSeatSelection();
    void requestReturnHome();

private:
    QLabel *titleLabel; // 将其提升为成员变量以便修改文本
};

#endif // CHECKOUTVIEW_H