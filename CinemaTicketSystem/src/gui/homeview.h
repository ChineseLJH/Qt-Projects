#ifndef HOMEVIEW_H
#define HOMEVIEW_H

#include <QWidget>

class HomeView : public QWidget
{
    Q_OBJECT // 只要用到信号槽，必须加上这个宏，供底层的 MOC（元对象编译器）解析

public:
    explicit HomeView(QWidget *parent = nullptr);

signals:
    // 自定义信号：只需声明，不需要写函数体（底层 MOC 会自动生成回调函数列表）
    void requestBookTicket(); 
};

#endif // HOMEVIEW_H