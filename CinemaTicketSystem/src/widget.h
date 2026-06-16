#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QVBoxLayout>

// 前向声明各个子视图类（这些类你后续需要单独创建）
class HomeView;
class MovieListView;
class SeatSelectionView;
class CheckoutView;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    // 定义页面枚举，用于路由控制
    enum Page {
        HOME_PAGE = 0,
        MOVIE_LIST_PAGE,
        SEAT_SELECTION_PAGE,
        CHECKOUT_PAGE
    };

public slots:
    // 页面跳转控制器
    void navigateTo(Page page);

private:
    QStackedWidget *mainStack; // 视图堆栈，控制当前显示哪个页面
    QVBoxLayout *mainLayout;   // 主布局，确保内部元素自动拉伸适配

    // 各个子页面的实例指针
    HomeView *homeView;
    MovieListView *movieListView;
    SeatSelectionView *seatSelectionView;
    CheckoutView *checkoutView;

    void setupUi();            // 初始化 UI 与布局的纯 C++ 实现
    void connectSignals();     // 统一管理所有的信号与槽
};

#endif // WIDGET_H