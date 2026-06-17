#include "widget.h"
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
            
            int *rawRows = new int[count];
            int *rawCols = new int[count];
            for(int i = 0; i < count; ++i) {
                rawRows[i] = confirmedSeats[i].x();
                rawCols[i] = confirmedSeats[i].y();
            }

            // 2. 核心修正：持久化写入时同样重定向至源码根目录，直接覆写原始的 seats.txt
            QString sourceRootPath = QCoreApplication::applicationDirPath() + "/../";
            manager->saveBookedSeats(hallId, rawRows, rawCols, count, sourceRootPath.toUtf8().constData());

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