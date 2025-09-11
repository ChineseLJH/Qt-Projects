#include "paymentscandialog.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QPixmap>
#include <QDebug>
#include <QSpacerItem>

PaymentScanDialog::PaymentScanDialog(const QVector<QString>& imagePaths, QWidget *parent)
    : QDialog(parent)
    , m_imagePaths(imagePaths)
    , m_currentIndex(0)
{
    this->setWindowTitle("扫描支付");
    this->setFixedSize(450, 600);

    // 调用UI设置函数
    setupUI();
    // 显示第一张支付码图片
    updateImageDisplay();
}

PaymentScanDialog::~PaymentScanDialog()
{
}

void PaymentScanDialog::setupUI()
{
    // 创建主垂直布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    // 顶部提示标签
    QLabel* titleLabel = new QLabel("请扫描二维码完成支付：");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 5px;");
    mainLayout->addWidget(titleLabel);

    // 添加一个固定的垂直间距
    QSpacerItem* topSpacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);
    mainLayout->addSpacerItem(topSpacer);

    // 使用网格布局来精确控制图片和按钮的位置
    QGridLayout* imageGridLayout = new QGridLayout();
    imageGridLayout->setSpacing(5);

    // 创建用于显示二维码的标签
    m_imageLabel = new QLabel();
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setScaledContents(true);
    m_imageLabel->setMinimumSize(300, 300);
    m_imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_imageLabel->setStyleSheet("border: 1px solid #ddd; background-color: #f8f8f8;");

    // 创建向左切换的按钮
    m_prevButton = new QPushButton("←");
    m_prevButton->setFixedSize(40, 40);
    m_prevButton->setStyleSheet("font-size: 16px; font-weight: bold;");

    // 创建向右切换的按钮
    m_nextButton = new QPushButton("→");
    m_nextButton->setFixedSize(40, 40);
    m_nextButton->setStyleSheet("font-size: 16px; font-weight: bold;");

    // 将控件添加到网格布局中
    // 按钮在第0行，第0和第2列；图片在第0行，第1列
    imageGridLayout->addWidget(m_prevButton, 0, 0, Qt::AlignCenter);
    imageGridLayout->addWidget(m_imageLabel, 0, 1);
    imageGridLayout->addWidget(m_nextButton, 0, 2, Qt::AlignCenter);

    // 设置列的拉伸因子，让中间的图片列占据多余空间
    imageGridLayout->setColumnStretch(0, 0); // 左列不拉伸
    imageGridLayout->setColumnStretch(1, 1); // 中间列拉伸
    imageGridLayout->setColumnStretch(2, 0); // 右列不拉伸

    // 将网格布局添加到主布局
    mainLayout->addLayout(imageGridLayout);

    // 添加另一个固定的垂直间距
    QSpacerItem* bottomSpacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);
    mainLayout->addSpacerItem(bottomSpacer);

    // 底部的确认和取消按钮
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText("确认支付");
    buttonBox->button(QDialogButtonBox::Ok)->setMinimumHeight(35);
    buttonBox->button(QDialogButtonBox::Cancel)->setText("取消支付");
    buttonBox->button(QDialogButtonBox::Cancel)->setMinimumHeight(35);
    mainLayout->addWidget(buttonBox);

    // 连接信号
    connect(m_prevButton, &QPushButton::clicked, this, &PaymentScanDialog::showPreviousImage);
    connect(m_nextButton, &QPushButton::clicked, this, &PaymentScanDialog::showNextImage);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void PaymentScanDialog::updateImageDisplay()
{
    // 如果没有图片路径，显示提示信息
    if (m_imagePaths.isEmpty())
    {
        m_imageLabel->setText("无支付码");
        m_prevButton->setEnabled(false);
        m_nextButton->setEnabled(false);
        return;
    }

    // 确保索引在有效范围内
    if (m_currentIndex < 0)
    {
        m_currentIndex = 0;
    }
    if (m_currentIndex >= m_imagePaths.size())
    {
        m_currentIndex = m_imagePaths.size() - 1;
    }

    // 加载并显示图片
    QString currentPath = m_imagePaths[m_currentIndex];
    QPixmap pixmap(currentPath);
    if (pixmap.isNull())
    {
        m_imageLabel->setText("图片加载失败");
        qWarning() << "加载支付图片失败:" << currentPath;
    }
    else
    {
        m_imageLabel->setPixmap(pixmap);
    }

    // 根据当前索引更新按钮的可用状态
    bool canGoBack = (m_currentIndex > 0);
    m_prevButton->setEnabled(canGoBack);

    bool canGoForward = (m_currentIndex < m_imagePaths.size() - 1);
    m_nextButton->setEnabled(canGoForward);
}

void PaymentScanDialog::showNextImage()
{
    // 如果不是最后一张，则切换到下一张
    if (m_currentIndex < m_imagePaths.size() - 1)
    {
        m_currentIndex++;
        updateImageDisplay();
    }
}

void PaymentScanDialog::showPreviousImage()
{
    // 如果不是第一张，则切换到上一张
    if (m_currentIndex > 0)
    {
        m_currentIndex--;
        updateImageDisplay();
    }
}
