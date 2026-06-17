#ifndef MOVIELISTVIEW_H
#define MOVIELISTVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include "../core/cinema_manager.h" // 引入底层管理器

class MovieListView : public QWidget
{
    Q_OBJECT
public:
    explicit MovieListView(QWidget *parent = nullptr);

    // 修改接口：直接接收管理器指针，以便内部反查影厅座位
    void loadMovies(CinemaManager *manager);

signals:
    void requestSeatSelection(int movieId);

private:
    QVBoxLayout *listLayout;
};

#endif // MOVIELISTVIEW_H