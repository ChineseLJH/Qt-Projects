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
    connect(btnConfirm, &QPushButton::clicked, this, &CheckoutView::requestReturnHome);
}

void CheckoutView::loadOrderData(Movie *movie, Hall *hall, int row, int col)
{
    if (!movie || !hall) return;

    // 使用纯 C 语言数组拼接最终的业务展示文本
    char orderInfo[256];
    snprintf(orderInfo, sizeof(orderInfo), 
             "电影：%s\n"
             "影厅：%d 号厅\n"
             "座位：%d 排 %d 座\n\n"
             "应付金额：%.2f 元", 
             movie->title, hall->hallId, row + 1, col + 1, movie->price); // 物理索引转逻辑索引(+1)

    titleLabel->setText(orderInfo);
}