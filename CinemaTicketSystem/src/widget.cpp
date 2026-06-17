#include "widget.h"
#include "gui/homeview.h"
#include "gui/movielistview.h"
#include "gui/seatselectionview.h"
#include "gui/checkoutview.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QList>
#include <QPoint>
#include <qrencode.h>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    this->resize(1024, 768); 
    this->setWindowTitle("智能电影院售票系统");

    // 1. 核心修正：利用物理相对路径，从 build/ 目录向上跳转一级，将寻址基址强行重定向至源码根目录
    QString sourceRootPath = QCoreApplication::applicationDirPath() + "/../";
    
    manager = new CinemaManager();
    // 确保初始化时直接读取源码目录下的文本数据
    manager->initializeData(sourceRootPath.toUtf8().constData());

    setupUi();

    // 构造函数中不再执行一次性载入，移交给路由控制器动态接管
    connectSignals();
}

Widget::~Widget() {
    delete manager;
}

void Widget::setupUi()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    mainStack = new QStackedWidget(this);

    homeView = new HomeView(this);
    movieListView = new MovieListView(this);
    seatSelectionView = new SeatSelectionView(this);
    checkoutView = new CheckoutView(this);

    mainStack->addWidget(homeView);          
    mainStack->addWidget(movieListView);     
    mainStack->addWidget(seatSelectionView); 
    mainStack->addWidget(checkoutView);      

    mainLayout->addWidget(mainStack);
    mainStack->setCurrentIndex(HOME_PAGE);
}

void Widget::connectSignals()
{
    static int currentMovieId = -1;

    connect(homeView, &HomeView::requestBookTicket, this, [=]() {
        navigateTo(MOVIE_LIST_PAGE);
    });

    connect(movieListView, &MovieListView::requestSeatSelection, this, [=](int movieId) {
        currentMovieId = movieId; 
        int targetHallId = (movieId == 101) ? 1 : (movieId == 102 ? 2 : 3); 
        Hall *targetHall = manager->getHallById(targetHallId);
        seatSelectionView->loadHallData(targetHall);
        navigateTo(SEAT_SELECTION_PAGE);
    });

    connect(seatSelectionView, &SeatSelectionView::requestCheckout, this, [=](QList<QPoint> seats) {
        Movie *targetMovie = nullptr;
        for(int i = 0; i < manager->getMovieCount(); ++i) {
            if(manager->getMovies()[i].id == currentMovieId) {
                targetMovie = &manager->getMovies()[i];
                break;
            }
        }
        int targetHallId = (currentMovieId == 101) ? 1 : (currentMovieId == 102 ? 2 : 3); 
        Hall *targetHall = manager->getHallById(targetHallId);

        checkoutView->loadOrderData(targetMovie, targetHall, seats);
        navigateTo(CHECKOUT_PAGE);
    });

    connect(seatSelectionView, &SeatSelectionView::requestBackToMovieList, this, [=]() {
        navigateTo(MOVIE_LIST_PAGE);
    });

    connect(checkoutView, &CheckoutView::requestBackToSeatSelection, this, [=]() {
        navigateTo(SEAT_SELECTION_PAGE);
    });

    connect(checkoutView, &CheckoutView::requestReturnHome, this, [=](QList<QPoint> confirmedSeats) {
        int count = confirmedSeats.size();
        if (count > 0) {
            int hallId = (currentMovieId == 101) ? 1 : (currentMovieId == 102 ? 2 : 3); 
            
            // 1. 获取目标电影指针用于拼接票据字符串
            Movie *targetMovie = nullptr;
            for(int i = 0; i < manager->getMovieCount(); ++i) {
                if(manager->getMovies()[i].id == currentMovieId) {
                    targetMovie = &manager->getMovies()[i];
                    break;
                }
            }

            // 2. 拼接二维码内容缓冲 (UTF-8 编码)
            QString seatStr;
            for(auto p : confirmedSeats) seatStr += QString("%1排%2座 ").arg(p.x()+1).arg(p.y()+1);
            QString ticketData = QString("电影: %1\n场次: %2\n影厅: %3号厅\n座位: %4")
                                 .arg(targetMovie ? targetMovie->title : "未知")
                                 .arg(targetMovie ? targetMovie->showTime : "未知")
                                 .arg(hallId)
                                 .arg(seatStr);

            // 3. 调用底层 C 函数执行里德-所罗门纠错编码
            // 返回值 QRcode 结构体包含了一个一维数组 data，其每个字节的最低有效位(LSB)代表像素黑白
            QRcode *qr = QRcode_encodeString(ticketData.toUtf8().constData(), 0, QR_ECLEVEL_L, QR_MODE_8, 1);
            
            if (qr != nullptr) {
                // 4. 将底层的 1D 数组映射为 2D 显存像素
                int qrWidth = qr->width;
                int scale = 8; // 物理像素放大倍率（每个二维码数据点映射为 8x8 的物理像素块）
                QImage qrImg(qrWidth * scale, qrWidth * scale, QImage::Format_RGB32);
                
                QPainter painter(&qrImg);
                painter.fillRect(qrImg.rect(), Qt::white); // 强制白底，防止深色模式下扫码仪解析失败
                painter.setBrush(Qt::black);
                painter.setPen(Qt::NoPen);

                for (int y = 0; y < qrWidth; ++y) {
                    for (int x = 0; x < qrWidth; ++x) {
                        // 底层协议：data 数组中的字节，按位与 1，若为 1 则该点为黑色模块
                        if (qr->data[y * qrWidth + x] & 1) {
                            painter.drawRect(x * scale, y * scale, scale, scale);
                        }
                    }
                }
                QRcode_free(qr); // 释放 C 库在堆区分配的内存，防止泄漏

                // 5. 构建电子票据专属的 QDialog 对话框
                QDialog ticketDialog(this);
                ticketDialog.setWindowTitle("出票成功");
                ticketDialog.setFixedSize(400, 500);
                ticketDialog.setStyleSheet("QDialog { background-color: #2C3E50; }");

                QVBoxLayout *dialogLayout = new QVBoxLayout(&ticketDialog);
                dialogLayout->setAlignment(Qt::AlignCenter);
                dialogLayout->setSpacing(20);

                QLabel *lblTitle = new QLabel("购票成功！", &ticketDialog);
                lblTitle->setStyleSheet("color: #2ECC71; font-size: 24px; font-weight: bold;");
                lblTitle->setAlignment(Qt::AlignCenter);

                QLabel *lblQr = new QLabel(&ticketDialog);
                lblQr->setPixmap(QPixmap::fromImage(qrImg));
                lblQr->setAlignment(Qt::AlignCenter);
                lblQr->setStyleSheet("border: 10px solid white; border-radius: 8px; background-color: white;");

                QLabel *lblHint = new QLabel("请截图保存此二维码\n入场时请在闸机处出示扫码", &ticketDialog);
                lblHint->setStyleSheet("color: #BDC3C7; font-size: 14px; line-height: 1.5;");
                lblHint->setAlignment(Qt::AlignCenter);

                QPushButton *btnFinish = new QPushButton("完成并返回首页", &ticketDialog);
                btnFinish->setFixedSize(200, 45);
                btnFinish->setCursor(Qt::PointingHandCursor);
                btnFinish->setStyleSheet(
                    "QPushButton { background-color: #E67E22; color: white; font-size: 16px; font-weight: bold; border-radius: 6px; border: none; }"
                    "QPushButton:hover { background-color: #D35400; }"
                );
                connect(btnFinish, &QPushButton::clicked, &ticketDialog, &QDialog::accept);

                dialogLayout->addStretch();
                dialogLayout->addWidget(lblTitle);
                dialogLayout->addWidget(lblQr);
                dialogLayout->addWidget(lblHint);
                dialogLayout->addWidget(btnFinish, 0, Qt::AlignHCenter);
                dialogLayout->addStretch();

                // 阻塞主线程，直到用户点击完成
                ticketDialog.exec();
            }

            // 6. 执行物理磁盘覆盖逻辑
            int *rawRows = new int[count];
            int *rawCols = new int[count];
            for(int i = 0; i < count; ++i) {
                rawRows[i] = confirmedSeats[i].x();
                rawCols[i] = confirmedSeats[i].y();
            }

            QString exePath = QCoreApplication::applicationDirPath() + "/../";
            manager->saveBookedSeats(hallId, rawRows, rawCols, count, exePath.toUtf8().constData());

            delete[] rawRows;
            delete[] rawCols;
        }

        navigateTo(HOME_PAGE);
    });
}

void Widget::navigateTo(Page page)
{
    // 3. 核心修正：切入拦截钩子。一旦状态机路由指向排片列表页，强制重新遍历内存矩阵并重建 UI 卡片
    if (page == MOVIE_LIST_PAGE) {
        movieListView->loadMovies(manager); 
    }
    mainStack->setCurrentIndex(page);
}