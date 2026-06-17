#include "movielistview.h"
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QPixmap>
#include <QCoreApplication>
#include <QStyle> // 核心引入：用于刷新底层样式树
#include <cstdio>

MovieListView::MovieListView(QWidget *parent) : QWidget(parent)
{
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setObjectName("movieListView");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    
    QLabel *title = new QLabel("正在热映", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: #F8F9FA; font-size: 28px; font-weight: bold; margin-bottom: 20px;");
    
    listLayout = new QVBoxLayout();
    listLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter); 
    listLayout->setSpacing(20);
    
    mainLayout->addWidget(title);
    mainLayout->addLayout(listLayout);
    mainLayout->addStretch();
}

void MovieListView::loadMovies(CinemaManager *manager)
{
    if (manager == nullptr || manager->getMovies() == nullptr) return;

    QLayoutItem *child;
    while ((child = listLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    Movie *movies = manager->getMovies();
    int count = manager->getMovieCount();
    QString exePath = QCoreApplication::applicationDirPath();

    for (int i = 0; i < count; ++i) {
        // 1. 数据寻址与座位统计逻辑
        Hall *hall = manager->getHallById(movies[i].targetHallId);
        int totalSeats = (hall != nullptr) ? hall->totalSeats : 0;
        int occupiedSeats = 0;
        if (hall != nullptr && hall->seatMatrix != nullptr && hall->colsPerRow != nullptr) {
            for (int r = 0; r < hall->rows; ++r) {
                // 核心修正：内层循环的边界必须动态读取当前行的列数
                for (int c = 0; c < hall->colsPerRow[r]; ++c) { 
                    if (hall->seatMatrix[r][c] == 1) occupiedSeats++;
                }
            }
        }

        // 2. 实例化卡片容器，移除内联样式，仅绑定对象名称
        QFrame *card = new QFrame(this);
        card->setFixedSize(700, 160);
        card->setObjectName("movieCard"); 

        QHBoxLayout *cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(15, 15, 20, 15);
        cardLayout->setSpacing(20);

        // 3. 实例化海报标签
        QLabel *posterLabel = new QLabel(card);
        posterLabel->setFixedSize(90, 130);
        
        QString imgPath = exePath + "/" + movies[i].posterPath;
        QPixmap posterImg(imgPath);
        if (!posterImg.isNull()) {
            posterLabel->setPixmap(posterImg.scaled(posterLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        } else {
            posterLabel->setText("暂无海报");
            posterLabel->setAlignment(Qt::AlignCenter);
        }

        // 4. 建立文本排版树
        QVBoxLayout *infoLayout = new QVBoxLayout();
        infoLayout->setSpacing(5);
        
        QLabel *titleLabel = new QLabel(movies[i].title, card);
        titleLabel->setObjectName("movieTitle"); // 映射到外部 QSS

        char detailStr[256];
        snprintf(detailStr, sizeof(detailStr), "放映时间: %s  |  时长: %d 分钟  |  票价: %.1f 元", 
                 movies[i].showTime, movies[i].duration, movies[i].price);
        QLabel *detailLabel = new QLabel(detailStr, card);
        detailLabel->setObjectName("movieDetail"); // 映射到外部 QSS

        char seatStr[128];
        snprintf(seatStr, sizeof(seatStr), "座位情况: %d / %d (已售 %d)", 
                 (totalSeats - occupiedSeats), totalSeats, occupiedSeats);
        QLabel *seatLabel = new QLabel(seatStr, card);
        seatLabel->setObjectName("movieSeat"); // 映射到外部 QSS
        
        // 核心改动：通过元对象动态属性控制颜色，消除 C++ 中的样式硬编码
        if (occupiedSeats >= totalSeats) {
            seatLabel->setProperty("isFull", true);
        } else {
            seatLabel->setProperty("isFull", false);
        }
        
        // 物理刷新指令：动态属性改变后，必须强行通知样式引擎重新计算该节点的渲染规则
        seatLabel->style()->unpolish(seatLabel);
        seatLabel->style()->polish(seatLabel);

        infoLayout->addWidget(titleLabel);
        infoLayout->addWidget(detailLabel);
        infoLayout->addWidget(seatLabel);
        infoLayout->addStretch();

        // 5. 实例化按钮
        QPushButton *bookBtn = new QPushButton("选座购票", card);
        bookBtn->setObjectName("movieBookBtn"); // 映射到外部 QSS
        bookBtn->setFixedSize(120, 45);
        bookBtn->setCursor(Qt::PointingHandCursor);

        int currentMovieId = movies[i].id;
        connect(bookBtn, &QPushButton::clicked, this, [=]() {
            emit requestSeatSelection(currentMovieId);
        });

        // 6. 拓扑结构组装
        cardLayout->addWidget(posterLabel);
        cardLayout->addLayout(infoLayout);
        cardLayout->addStretch();
        cardLayout->addWidget(bookBtn, 0, Qt::AlignVCenter);

        listLayout->addWidget(card);
    }
}