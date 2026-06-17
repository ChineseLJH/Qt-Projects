#include "homeview.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

HomeView::HomeView(QWidget *parent) : QWidget(parent)
{
    // 核心物理修正：强制向底层渲染引擎申请 QWidget 的背景重绘权限
    this->setAttribute(Qt::WA_StyledBackground, true);
    
    // 绑定内存标识符，供 QSS 引擎精准寻址匹配
    this->setObjectName("homeView");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0); // 取消边缘保护间距

    QLabel *titleLabel = new QLabel("智能电影院售票系统", this);
    titleLabel->setObjectName("mainTitleLabel"); // 绑定 ID
    titleLabel->setAlignment(Qt::AlignCenter);

    QPushButton *btnStart = new QPushButton("开始购票", this);
    btnStart->setObjectName("startBtn");         // 绑定 ID
    btnStart->setFixedSize(260, 60);             // 限制内存渲染区块的最大宽高
    btnStart->setCursor(Qt::PointingHandCursor);

    // 计算二维纵轴坐标，利用弹性系数分配剩余显存区域
    layout->addStretch(1); 
    layout->addWidget(titleLabel);
    layout->addSpacing(80); // 固定 80 像素的物理间距
    layout->addWidget(btnStart, 0, Qt::AlignHCenter); 
    layout->addStretch(1); 

    // 绑定事件寄存器跳转
    connect(btnStart, &QPushButton::clicked, this, &HomeView::requestBookTicket);
}