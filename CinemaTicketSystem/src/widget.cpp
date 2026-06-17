#include "widget.h"
// 引入刚刚写好的头文件
#include "gui/homeview.h"
#include "gui/movielistview.h"
#include "gui/seatselectionview.h"
#include "gui/checkoutview.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    this->resize(1024, 768); 
    this->setWindowTitle("智能电影院售票系统");

    // 1. 在堆区分配底层核心管理器的内存，并初始化所有结构体数据
    manager = new CinemaManager();
    manager->initializeData();

    setupUi();

    // 2. 将底层内存中的电影数据注入到排片视图中
    movieListView->loadMovies(manager->getMovies(), manager->getMovieCount());

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
    // 利用堆区变量或 Lambda 静态捕获来维持状态流转。
    // 这里使用 shared_ptr 或外部变量，最直接的方式是在 Lambda 外部定义一个静态变量记录 movieId。
    static int currentMovieId = -1;

    connect(homeView, &HomeView::requestBookTicket, this, [=]() {
        navigateTo(MOVIE_LIST_PAGE);
    });

    connect(movieListView, &MovieListView::requestSeatSelection, this, [=](int movieId) {
        currentMovieId = movieId; // 记录当前选择的电影ID
        int targetHallId = (movieId == 101) ? 1 : 2; 
        Hall *targetHall = manager->getHallById(targetHallId);
        seatSelectionView->loadHallData(targetHall);
        navigateTo(SEAT_SELECTION_PAGE);
    });

    // 捕获带有底层矩阵物理坐标的选座信号
    connect(seatSelectionView, &SeatSelectionView::requestCheckout, this, [=](int row, int col) {
        // 反查指针
        Movie *targetMovie = nullptr;
        for(int i = 0; i < manager->getMovieCount(); ++i) {
            if(manager->getMovies()[i].id == currentMovieId) {
                targetMovie = &manager->getMovies()[i];
                break;
            }
        }
        int targetHallId = (currentMovieId == 101) ? 1 : 2; 
        Hall *targetHall = manager->getHallById(targetHallId);

        // 注入指针数据到结算页
        checkoutView->loadOrderData(targetMovie, targetHall, row, col);
        navigateTo(CHECKOUT_PAGE);
    });

    connect(checkoutView, &CheckoutView::requestBackToSeatSelection, this, [=]() {
        navigateTo(SEAT_SELECTION_PAGE);
    });

    connect(checkoutView, &CheckoutView::requestReturnHome, this, [=]() {
        navigateTo(HOME_PAGE);
    });
}

void Widget::navigateTo(Page page)
{
    // 修改堆栈的当前索引
    // 底层机制是隐藏当前显示的 QWidget 指针，并调用目标 QWidget 指针的 show() 方法引发重绘
    mainStack->setCurrentIndex(page);
}