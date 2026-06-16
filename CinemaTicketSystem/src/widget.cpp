#include "widget.h"
// 后续建好子视图后，在这里 include 它们的头文件
// #include "homeview.h"
// #include "movielistview.h"
// #include "seatselectionview.h"
// #include "checkoutview.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    // 配置主窗口基础属性
    this->resize(1024, 768); 
    this->setWindowTitle("智能电影院售票系统");

    // 核心初始化逻辑
    setupUi();
    connectSignals();
}

Widget::~Widget()
{
    // 析构函数。由于我们传入了 this 作为 parent，
    // Qt 的对象树会自动释放 mainStack 和各个 View 的内存。
    // 但如果你后续在这里 new 了底层的二维矩阵等纯 C++ 结构，必须在这里手动 delete。
}

void Widget::setupUi()
{
    // 1. 初始化最外层的主布局
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0); // 取消边缘留白，让画面铺满

    // 2. 初始化堆栈容器
    mainStack = new QStackedWidget(this);

    // 3. 实例化各个子视图 (传入 this 作为 parent)
    // homeView = new HomeView(this);
    // movieListView = new MovieListView(this);
    // seatSelectionView = new SeatSelectionView(this);
    // checkoutView = new CheckoutView(this);

    // 4. 将视图按顺序压入栈中 (索引与头文件中的 Page 枚举严格对应)
    // mainStack->addWidget(homeView);          // Index 0
    // mainStack->addWidget(movieListView);     // Index 1
    // mainStack->addWidget(seatSelectionView); // Index 2
    // mainStack->addWidget(checkoutView);      // Index 3

    // 5. 将堆栈容器放入主布局中
    mainLayout->addWidget(mainStack);

    // 6. 默认显示第一页：系统入口页
    mainStack->setCurrentIndex(HOME_PAGE);
}

void Widget::connectSignals()
{
    // 信号路由中心。
    // 举例：捕获 HomeView 发出的“开始选座”信号，并触发页面跳转
    // connect(homeView, &HomeView::requestBookTicket, this, [=]() {
    //     navigateTo(MOVIE_LIST_PAGE);
    // });
}

void Widget::navigateTo(Page page)
{
    // 核心流转逻辑：切换堆栈窗口的当前索引
    mainStack->setCurrentIndex(page);
}