#include "homeview.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

HomeView::HomeView(QWidget *parent) : QWidget(parent)
{
    // 1. 在堆内存中分配一个垂直布局管理器，并将当前界面(this)设为它的父节点
    QVBoxLayout *layout = new QVBoxLayout(this);

    // 2. 动态分配一个标签，设置文字居中
    QLabel *titleLabel = new QLabel("欢迎来到售票系统", this);
    titleLabel->setAlignment(Qt::AlignCenter);

    // 3. 动态分配一个按钮
    QPushButton *btnStart = new QPushButton("开始购票", this);
    btnStart->setFixedSize(120, 40); // 暂时给按钮一个固定大小，防止被拉伸得太难看

    // 4. 将这两个组件指针添加到布局管理器中
    layout->addWidget(titleLabel);
    layout->addWidget(btnStart, 0, Qt::AlignHCenter); // 让按钮在水平方向居中

    // 5. 核心底层逻辑：信号转发
    // 将按钮自带的 clicked() 信号，连接到我们自定义的 requestBookTicket() 信号上。
    // 这意味着点击按钮时，HomeView 会向外部广播 "我要购票" 的内存事件。
    connect(btnStart, &QPushButton::clicked, this, &HomeView::requestBookTicket);
}