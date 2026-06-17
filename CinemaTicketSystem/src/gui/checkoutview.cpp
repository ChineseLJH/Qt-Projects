#include "checkoutview.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPixmap>
#include <QCoreApplication>
#include <cstdio>

CheckoutView::CheckoutView(QWidget *parent) : QWidget(parent)
{
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setObjectName("checkoutView");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 40, 50, 40);
    mainLayout->setAlignment(Qt::AlignCenter);

    // 1. 顶部状态：大大的支付图标
    QLabel *statusIcon = new QLabel("✓", this);
    statusIcon->setObjectName("statusIcon");
    statusIcon->setAlignment(Qt::AlignCenter);

    QLabel *statusText = new QLabel("订单已锁定，请确认支付", this);
    statusText->setObjectName("statusText");
    statusText->setAlignment(Qt::AlignCenter);

    // 2. 核心：电子票据卡片 (Ticket Card)
    QFrame *ticketCard = new QFrame(this);
    ticketCard->setObjectName("ticketCard");
    ticketCard->setFixedSize(500, 320);

    QHBoxLayout *ticketLayout = new QHBoxLayout(ticketCard);
    ticketLayout->setContentsMargins(20, 20, 20, 20);
    ticketLayout->setSpacing(25);

    // 左侧：缩略海报
    posterLabel = new QLabel(ticketCard);
    posterLabel->setFixedSize(140, 200);
    posterLabel->setObjectName("checkoutPoster");

    // 右侧：订单详情
    QVBoxLayout *detailLayout = new QVBoxLayout();
    movieTitleLabel = new QLabel(ticketCard);
    movieTitleLabel->setObjectName("checkoutTitle");

    infoLabel = new QLabel(ticketCard);
    infoLabel->setObjectName("checkoutInfo");

    seatsLabel = new QLabel(ticketCard);
    seatsLabel->setObjectName("checkoutSeats");
    seatsLabel->setWordWrap(true); // 自动换行

    // 虚线分割线 (QFrame 模拟)
    QFrame *line = new QFrame(ticketCard);
    line->setFrameShape(QFrame::HLine);
    line->setObjectName("ticketLine");

    priceLabel = new QLabel(ticketCard);
    priceLabel->setObjectName("checkoutPrice");

    detailLayout->addWidget(movieTitleLabel);
    detailLayout->addWidget(infoLabel);
    detailLayout->addWidget(seatsLabel);
    detailLayout->addStretch();
    detailLayout->addWidget(line);
    detailLayout->addWidget(priceLabel);

    ticketLayout->addWidget(posterLabel);
    ticketLayout->addLayout(detailLayout);

    // 3. 底部操作按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(30);

    QPushButton *btnBack = new QPushButton("返回修改", this);
    btnBack->setObjectName("btnSecondary");
    btnBack->setFixedSize(160, 50);

    QPushButton *btnPay = new QPushButton("确认支付", this);
    btnPay->setObjectName("btnPrimary");
    btnPay->setFixedSize(160, 50);

    btnLayout->addWidget(btnBack);
    btnLayout->addWidget(btnPay);
    btnLayout->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(statusIcon);
    mainLayout->addWidget(statusText);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(ticketCard, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(40);
    mainLayout->addLayout(btnLayout);

    connect(btnBack, &QPushButton::clicked, this, &CheckoutView::requestBackToSeatSelection);
    connect(btnPay, &QPushButton::clicked, this, [=](){
        emit requestReturnHome(pendingSeats);
    });
}

void CheckoutView::loadOrderData(Movie *movie, Hall *hall, QList<QPoint> seats)
{
    if (!movie || !hall || seats.isEmpty()) return;
    pendingSeats = seats;

    // 加载图片
    QString exePath = QCoreApplication::applicationDirPath();
    QPixmap pix(exePath + "/../" + movie->posterPath);
    posterLabel->setPixmap(pix.scaled(posterLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    movieTitleLabel->setText(movie->title);
    infoLabel->setText(QString("场次: %1 | %2号厅").arg(movie->showTime).arg(hall->hallId));
    
    QString sStr = "座位: ";
    for(auto p : seats) sStr += QString("%1排%2座 ").arg(p.x()+1).arg(p.y()+1);
    seatsLabel->setText(sStr);

    double total = movie->price * seats.size();
    priceLabel->setText(QString("￥%1").arg(total, 0, 'f', 2));
}