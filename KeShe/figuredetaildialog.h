#ifndef FIGUREDETAILDIALOG_H
#define FIGUREDETAILDIALOG_H

#include <QDialog>
#include "figure.h"

// 前向声明
namespace Ui { class FigureDetailDialog; }
class QSpinBox;
class QLabel;

class FigureDetailDialog : public QDialog
{
    Q_OBJECT

public:
    // 构造函数接收一个Figure对象的常量引用来展示数据
    explicit FigureDetailDialog(const Figure& figure, QWidget *parent = nullptr);
    ~FigureDetailDialog();

    // 公共接口，用于获取用户选择的购买数量
    int getSelectedQuantity() const;

private:
    // 在这个类里，我们不使用 .ui 文件，而是用代码手写UI，展示另一种方式
    void setupUi(const Figure& figure);

    QLabel* m_imageLabel;
    QLabel* m_nameLabel;
    QLabel* m_priceLabel;
    QLabel* m_stockLabel;
    QSpinBox* m_quantitySpinBox; // 允许用户选择购买数量
};

#endif // FIGUREDETAILDIALOG_H
