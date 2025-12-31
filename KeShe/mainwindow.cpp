#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "inventorymanager.h"
#include "figure.h"
#include "figuredetaildialog.h"
#include "paymentscandialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QCloseEvent>
#include <QScrollArea>
#include <QMouseEvent>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_gridLayout(nullptr)
    , m_searchLineEdit(nullptr)
{
    ui->setupUi(this);

    // 设置窗口基本属性
    this->setFixedSize(800, 600);
    this->setWindowTitle("手办自助售卖系统");

    // 初始化UI组件
    setupInitialUI();

    // 初始化库存管理器
    m_inventoryManager = new InventoryManager(this);

    // 设置信号和槽的连接
    setupConnections();

    // 加载初始数据
    m_inventoryManager->initialize();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupInitialUI()
{
    // 创建中央窗口部件和主垂直布局
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // 创建和配置搜索输入框
    m_searchLineEdit = new QLineEdit(this);
    m_searchLineEdit->setPlaceholderText("按名称搜索手办...");
    m_searchLineEdit->setFixedHeight(30);
    m_searchLineEdit->setStyleSheet("padding: 5px; border-radius: 5px; border: 1px solid #ccc;");

    // 将搜索框添加到主布局的顶部
    mainLayout->addWidget(m_searchLineEdit);

    // 创建滚动区域，用于显示商品网格
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 创建一个容器来放置网格布局
    QWidget* gridContainer = new QWidget();

    // 创建网格布局
    m_gridLayout = new QGridLayout(gridContainer);
    m_gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_gridLayout->setSpacing(15);

    // 为商品卡片设置样式表，支持悬浮和缺货效果
    QString cardStyleSheet = R"(
        ClickableWidget {
            background-color: white;
            border: 1px solid #ddd;
            border-radius: 8px;
            transition: all 0.2s ease-in-out;
        }
        ClickableWidget:hover {
            border-color: #0078d7;
            transform: scale(1.05);
        }
        ClickableWidget[outOfStock="true"] {
            background-color: #f2f2f2;
        }
        ClickableWidget[outOfStock="true"]:hover {
            border-color: #c0392b;
            transform: none;
        }
    )";
    gridContainer->setStyleSheet(cardStyleSheet);

    // 将网格容器放入滚动区域
    scrollArea->setWidget(gridContainer);

    // 将滚动区域添加到主布局
    mainLayout->addWidget(scrollArea);

    // 设置主窗口的中央部件
    setCentralWidget(centralWidget);
}

void MainWindow::setupConnections()
{
    // 当库存数据变化时，刷新界面显示
    connect(m_inventoryManager, &InventoryManager::inventoryChanged, this, &MainWindow::updateDisplay);

    // 当需要显示提示信息时，更新状态栏
    connect(m_inventoryManager, &InventoryManager::displayMessage, this, &MainWindow::showStatusMessage);

    // 当搜索框文本变化时，刷新界面显示
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::updateDisplay);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 在关闭窗口前，保存当前的库存状态
    m_inventoryManager->saveInventory();

    // 调用父类的同名函数，完成窗口关闭
    QMainWindow::closeEvent(event);
}

void MainWindow::clearLayout(QLayout* layout)
{
    if (layout == nullptr)
    {
        return;
    }

    QLayoutItem* item;
    // 循环移除并删除布局中的所有项目
    while ((item = layout->takeAt(0)) != nullptr)
    {
        if (item->widget())
        {
            delete item->widget();
        }
        delete item;
    }
}

void MainWindow::updateDisplay()
{
    // 更新前先清空布局
    clearLayout(m_gridLayout);

    // 从搜索框获取用户输入的文本
    QString searchText = m_searchLineEdit->text();
    QString trimmedSearchText = searchText.trimmed();

    const auto& allFigures = m_inventoryManager->getFigures();

    // 根据搜索文本过滤需要显示的商品
    QList<Figure> figuresToShow;
    if (trimmedSearchText.isEmpty())
    {
        // 如果搜索框为空，显示所有商品
        figuresToShow = allFigures;
    }
    else
    {
        // 否则，只显示名称匹配的商品
        for (const Figure& fig : allFigures)
        {
            bool nameMatches = fig.name.contains(trimmedSearchText, Qt::CaseInsensitive);
            if (nameMatches)
            {
                figuresToShow.append(fig);
            }
        }
    }

    int row = 0;
    int col = 0;
    const int maxColumns = 4; // 每行最多显示4个

    for (const Figure& fig : figuresToShow)
    {
        // 创建一个可点击的卡片Widget
        ClickableWidget* cardWidget = new ClickableWidget();
        cardWidget->setFixedSize(160, 200);
        cardWidget->setCursor(Qt::PointingHandCursor);

        // 如果库存为0，设置一个自定义属性用于CSS样式
        bool isOutOfStock = (fig.quantity == 0);
        if (isOutOfStock)
        {
            cardWidget->setProperty("outOfStock", true);
        }

        // 创建卡片内部的垂直布局
        QVBoxLayout* cardLayout = new QVBoxLayout(cardWidget);

        // -- 图片 --
        QLabel* imageLabel = new QLabel();
        QPixmap originalPixmap(fig.imagePath);
        if(originalPixmap.isNull())
        {
            imageLabel->setText("无图片");
        }
        else
        {
            QPixmap scaledPixmap = originalPixmap.scaled(140, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            imageLabel->setPixmap(scaledPixmap);
        }
        imageLabel->setAlignment(Qt::AlignCenter);

        // -- 商品信息 --
        QString nameText = QString("<b>%1</b>").arg(fig.name);
        QString priceText = QString("¥%1").arg(fig.price, 0, 'f', 2);
        QString infoText = nameText + "<br>" + priceText;
        QLabel* infoLabel = new QLabel(infoText);
        infoLabel->setAlignment(Qt::AlignCenter);
        infoLabel->setWordWrap(true);

        // -- 库存状态 --
        QLabel* stockLabel = new QLabel();
        stockLabel->setAlignment(Qt::AlignCenter);
        if (fig.quantity > 0)
        {
            QString stockInfo = QString("库存: %1").arg(fig.quantity);
            stockLabel->setText(stockInfo);
            stockLabel->setStyleSheet("color: green;");
        }
        else
        {
            stockLabel->setText("已售罄");
            stockLabel->setStyleSheet("color: red; font-weight: bold;");
        }

        // 将所有组件添加到卡片布局中
        cardLayout->addWidget(imageLabel);
        cardLayout->addWidget(infoLabel);
        cardLayout->addWidget(stockLabel);
        cardLayout->addStretch();

        // 连接卡片的点击信号
        QString figureId = fig.id;
        connect(cardWidget, &ClickableWidget::clicked, this, [this, figureId]() {
            showFigureDetails(figureId);
        });

        // 将卡片添加到网格布局中
        m_gridLayout->addWidget(cardWidget, row, col);

        // 更新下一个卡片的位置
        col++;
        if (col >= maxColumns)
        {
            col = 0;
            row++;
        }
    }
}

void MainWindow::showFigureDetails(const QString& figureId)
{
    const Figure* fig = m_inventoryManager->getFigureById(figureId);
    if (fig == nullptr)
    {
        return;
    }

    // 创建并显示商品详情对话框
    FigureDetailDialog dialog(*fig, this);
    int dialogResult = dialog.exec();

    // 如果用户点击了“购买”
    if (dialogResult == QDialog::Accepted)
    {
        int quantity = dialog.getSelectedQuantity();

        // 定义支付二维码图片的路径
        QVector<QString> paymentImages;
        paymentImages.append("C:/Qt-Projects/KeShe/payment/alipay.jpg");
        paymentImages.append("C:/Qt-Projects/KeShe/payment/wechatpay.png");

        // 弹出支付对话框
        PaymentScanDialog paymentDialog(paymentImages, this);
        int paymentResult = paymentDialog.exec();

        if (paymentResult == QDialog::Accepted)
        {
            // 如果用户确认支付，则处理购买逻辑
            m_inventoryManager->purchaseFigure(figureId, quantity);
        }
        else
        {
            // 如果用户取消支付
            m_inventoryManager->displayMessage("支付已取消。", false);
        }
    }
}

void MainWindow::showStatusMessage(const QString& message, bool isError)
{
    if (isError)
    {
        ui->statusbar->setStyleSheet("color: red;");
    }
    else
    {
        ui->statusbar->setStyleSheet(""); // 恢复默认颜色
    }
    // 显示消息，5秒后自动消失
    ui->statusbar->showMessage(message, 5000);
}
