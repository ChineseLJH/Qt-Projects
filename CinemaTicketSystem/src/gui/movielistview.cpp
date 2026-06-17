#include "movielistview.h"
#include <QLabel>
#include <QPushButton>
#include <cstdio> // 引入 snprintf

MovieListView::MovieListView(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QLabel *title = new QLabel("正在热映 (请选择排片场次)", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 20px; font-weight: bold; margin: 20px;");
    
    listLayout = new QVBoxLayout();
    listLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter); // 列表顶部水平居中对齐
    
    mainLayout->addWidget(title);
    mainLayout->addLayout(listLayout);
    mainLayout->addStretch(); // 把列表往上顶
}

void MovieListView::loadMovies(Movie *movies, int count)
{
    if (movies == nullptr) return;

    // 清理旧的 UI 节点缓存
    QLayoutItem *child;
    while ((child = listLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    // 遍历底层 Movie 指针数组
    for (int i = 0; i < count; ++i) {
        char btnText[128];
        // 使用纯 C 语言方式格式化字符串
        snprintf(btnText, sizeof(btnText), "《%s》 | 时长: %d 分钟 | 票价: %.1f 元", 
                 movies[i].title, movies[i].duration, movies[i].price);

        QPushButton *btn = new QPushButton(btnText, this);
        btn->setFixedSize(400, 60);
        btn->setStyleSheet("QPushButton { text-align: left; padding-left: 20px; font-size: 16px; }");

        int currentMovieId = movies[i].id;

        // 核心跳转映射：点击该按钮，向外发射带有当前电影 ID 的信号
        connect(btn, &QPushButton::clicked, this, [=]() {
            emit requestSeatSelection(currentMovieId);
        });

        listLayout->addWidget(btn);
    }
}