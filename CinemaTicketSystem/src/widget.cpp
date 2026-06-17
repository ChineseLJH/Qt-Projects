#include "widget.h"
// 引入刚刚写好的头文件
#include "gui/homeview.h"
#include "gui/movielistview.h"
#include "gui/seatselectionview.h"
#include "gui/checkoutview.h"
#include <QCoreApplication>
#include <QList>
#include <QPoint>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    this->resize(1024, 768); 
    this->setWindowTitle("智能电影院售票系统");

    QString exePath = QCoreApplication::applicationDirPath();
    
    manager = new CinemaManager();
    manager->initializeData(exePath.toUtf8().constData());

    setupUi();

    movieListView->loadMovies(manager);
    connectSignals();
}

Widget::~Widget() {
    // 释放底层核心管理器的内存
    delete manager;
}

void Widget::setupUi()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    mainStack = new QStackedWidget(this);

    // 1. 实例化各个页面对象（在堆上分配内存）
    homeView = new HomeView(this);
    movieListView = new MovieListView(this);
    seatSelectionView = new SeatSelectionView(this);
    checkoutView = new CheckoutView(this);

    // 2. 按枚举顺序将页面压入堆栈容器
    mainStack->addWidget(homeView);          // Index 0 (HOME_PAGE)
    mainStack->addWidget(movieListView);     // Index 1 (MOVIE_LIST_PAGE)
    mainStack->addWidget(seatSelectionView); // Index 2 (SEAT_SELECTION_PAGE)
    mainStack->addWidget(checkoutView);      // Index 3 (CHECKOUT_PAGE)

    mainLayout->addWidget(mainStack);

    // 设置程序启动时默认显示第 0 页
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
        int targetHallId = (movieId == 101) ? 1 : 2; 
        Hall *targetHall = manager->getHallById(targetHallId);
        seatSelectionView->loadHallData(targetHall);
        navigateTo(SEAT_SELECTION_PAGE);
    });

    // 核心修正 1：形参必须接收 QList 容器以匹配新的路由协议
    connect(seatSelectionView, &SeatSelectionView::requestCheckout, this, [=](QList<QPoint> seats) {
        Movie *targetMovie = nullptr;
        for(int i = 0; i < manager->getMovieCount(); ++i) {
            if(manager->getMovies()[i].id == currentMovieId) {
                targetMovie = &manager->getMovies()[i];
                break;
            }
        }
        int targetHallId = (currentMovieId == 101) ? 1 : 2; 
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

    // 核心修正 2：承接结算页数据并执行持久化内存调度
    connect(checkoutView, &CheckoutView::requestReturnHome, this, [=](QList<QPoint> confirmedSeats) {
        int count = confirmedSeats.size();
        if (count > 0) {
            int hallId = (currentMovieId == 101) ? 1 : 2; 
            
            // 内存降维：将对象容器的离散坐标提取至连续的堆区原生数组中
            int *rawRows = new int[count];
            int *rawCols = new int[count];
            for(int i = 0; i < count; ++i) {
                rawRows[i] = confirmedSeats[i].x();
                rawCols[i] = confirmedSeats[i].y();
            }

            QString exePath = QCoreApplication::applicationDirPath();
            manager->saveBookedSeats(hallId, rawRows, rawCols, count, exePath.toUtf8().constData());

            // 严格对齐 new 与 delete 边界
            delete[] rawRows;
            delete[] rawCols;
        }

        navigateTo(HOME_PAGE);
    });
}

void Widget::navigateTo(Page page)
{
    // 修改堆栈的当前索引
    // 底层机制是隐藏当前显示的 QWidget 指针，并调用目标 QWidget 指针的 show() 方法引发重绘
    mainStack->setCurrentIndex(page);
}