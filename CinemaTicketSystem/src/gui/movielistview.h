#ifndef MOVIELISTVIEW_H
#define MOVIELISTVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include "../core/core_structs.h" // 引入底层结构体

class MovieListView : public QWidget
{
    Q_OBJECT
public:
    explicit MovieListView(QWidget *parent = nullptr);

    // 核心接口：接收纯 C++ 的电影数组首地址和数量
    void loadMovies(Movie *movies, int count);

signals:
    // 触发此信号时，携带着用户选择的影片 ID
    void requestSeatSelection(int movieId);

private:
    QVBoxLayout *listLayout; // 专门用来存放电影列表的局部布局
};

#endif // MOVIELISTVIEW_H