#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class InventoryManager;
class QGridLayout;
// 【新增】一个可点击的自定义Widget，用于商品卡片
class ClickableWidget : public QWidget
{
    Q_OBJECT
public:
    using QWidget::QWidget; // 使用父类的构造函数
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent *event) override { emit clicked(); }
};


namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void updateDisplay();
    // 修改槽函数，不再直接购买，而是显示详情
    void showFigureDetails(const QString& figureId);
    void showStatusMessage(const QString& message, bool isError);

private:
    void setupConnections();
    void clearLayout(QLayout* layout);
    void setupInitialUI();

    Ui::MainWindow* ui;
    InventoryManager* m_inventoryManager;
    QGridLayout* m_gridLayout;
};

#endif // MAINWINDOW_H

