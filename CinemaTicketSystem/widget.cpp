#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{

    this->setFixedSize(800, 600);
    this->setWindowTitle("智能电影院售票系统");

}

Widget::~Widget()
{


}
