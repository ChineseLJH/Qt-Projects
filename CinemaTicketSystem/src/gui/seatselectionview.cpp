#include "seatselectionview.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox> // 用于弹出未选座的警告

SeatSelectionView::SeatSelectionView(QWidget *parent) 
    : QWidget(parent), currentHall(nullptr), selectedRow(-1), selectedCol(-1), currentSelectedBtn(nullptr)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QLabel *screenLabel = new QLabel("中央荧幕", this);
    screenLabel->setAlignment(Qt::AlignCenter);
    screenLabel->setStyleSheet("background-color: #333; color: white; padding: 10px; border-radius: 5px;");
    
    seatGridLayout = new QGridLayout();
    seatGridLayout->setSpacing(5);
    
    QPushButton *btnNext = new QPushButton("确认选座，去结算", this);
    btnNext->setFixedSize(150, 40);
    
    mainLayout->addWidget(screenLabel);
    mainLayout->addSpacing(30);
    mainLayout->addLayout(seatGridLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(btnNext, 0, Qt::AlignHCenter);
    
    // 提交逻辑：读取当前类的内存变量，若合法则向外抛出带参信号
    connect(btnNext, &QPushButton::clicked, this, [this]() {
        if (selectedRow == -1 || selectedCol == -1) {
            QMessageBox::warning(this, "提示", "请先在矩阵中选择一个空闲座位");
            return;
        }
        emit requestCheckout(selectedRow, selectedCol);
    });
}

void SeatSelectionView::loadHallData(Hall *hall)
{
    if (hall == nullptr || hall->seatMatrix == nullptr) return;
    currentHall = hall;

    // 每次加载新影厅时，重置类内部的内存状态指针
    selectedRow = -1;
    selectedCol = -1;
    currentSelectedBtn = nullptr;

    QLayoutItem *child;
    while ((child = seatGridLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    for (int i = 0; i < hall->rows; ++i) {
        for (int j = 0; j < hall->cols; ++j) {
            QPushButton *seatBtn = new QPushButton(this);
            seatBtn->setFixedSize(40, 40);

            int status = hall->seatMatrix[i][j];

            if (status == 1) {
                seatBtn->setStyleSheet("background-color: #E74C3C; border: none;"); // 红：已售
                seatBtn->setEnabled(false);
            } else {
                seatBtn->setStyleSheet("background-color: #ECF0F1; border: 1px solid #BDC3C7;"); // 白：空闲
                
                // 核心闭包逻辑：使用 [=] 以值传递方式捕获当前的 i, j 和 seatBtn 指针
                connect(seatBtn, &QPushButton::clicked, this, [=]() {
                    // 1. 如果之前已经有选中的按钮指针，将其 UI 显存状态重置为空闲色
                    if (currentSelectedBtn != nullptr) {
                        currentSelectedBtn->setStyleSheet("background-color: #ECF0F1; border: 1px solid #BDC3C7;");
                    }
                    
                    // 2. 将当前操作的按钮地址赋给跟踪指针，更新二维坐标
                    currentSelectedBtn = seatBtn;
                    selectedRow = i;
                    selectedCol = j;
                    
                    // 3. 将新锁定的按钮修改为绿色
                    seatBtn->setStyleSheet("background-color: #2ECC71; border: none;"); 
                });
            }

            seatGridLayout->addWidget(seatBtn, i, j);
        }
    }
}