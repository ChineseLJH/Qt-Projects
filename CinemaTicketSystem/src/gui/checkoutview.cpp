#include "checkoutview.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <cstdio>

CheckoutView::CheckoutView(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 初始化时分配内存
    titleLabel = new QLabel("等待计算订单...", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 18px; line-height: 1.5;");

    QPushButton *btnCancel = new QPushButton("返回修改", this);
    QPushButton *btnConfirm = new QPushButton("确认支付", this);
    
    btnCancel->setFixedSize(150, 40);
    btnConfirm->setFixedSize(150, 40);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(btnCancel);
    buttonLayout->addWidget(btnConfirm);
    buttonLayout->setAlignment(Qt::AlignCenter);

    mainLayout->addStretch();
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(30);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();

    connect(btnCancel, &QPushButton::clicked, this, &CheckoutView::requestBackToSeatSelection);
    connect(btnConfirm, &QPushButton::clicked, this, [=](){
        emit requestReturnHome(pendingSeats);
    });
}

void CheckoutView::loadOrderData(Movie *movie, Hall *hall, QList<QPoint> seats)
{
    if (!movie || !hall || seats.isEmpty()) return;
    pendingSeats = seats;

    // 解析所有座位的逻辑索引（物理索引+1）
    char seatsStr[256] = "";
    for (int i = 0; i < seats.size(); ++i) {
        char temp[32];
        snprintf(temp, sizeof(temp), "%d排%d座 ", seats[i].x() + 1, seats[i].y() + 1);
        strcat(seatsStr, temp); // 内存追加拼装
    }

    double totalPrice = movie->price * seats.size();

    char orderInfo[512];
    snprintf(orderInfo, sizeof(orderInfo), 
             "电影：%s\n"
             "影厅：%d 号厅\n"
             "座位：%s\n\n"
             "应付总金额：%.2f 元", 
             movie->title, hall->hallId, seatsStr, totalPrice); 

    titleLabel->setText(orderInfo);
}