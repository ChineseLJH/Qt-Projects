#include "figuredetaildialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QPixmap>

FigureDetailDialog::FigureDetailDialog(const Figure& figure, QWidget *parent)
    : QDialog(parent)
{
    // 设置对话框的基本属性
    this->setWindowTitle("商品详情");
    this->setFixedSize(450, 250);

    // 调用函数来构建界面
    setupUi(figure);
}

FigureDetailDialog::~FigureDetailDialog() {}

void FigureDetailDialog::setupUi(const Figure& figure)
{
    // --- 左侧区域：商品大图 ---
    m_imageLabel = new QLabel(this);
    m_imageLabel->setFixedSize(200, 200);
    QPixmap pixmap(figure.imagePath);
    if(pixmap.isNull())
    {
        m_imageLabel->setText("无图片");
    }
    else
    {
        QPixmap scaledPixmap = pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_imageLabel->setPixmap(scaledPixmap);
    }

    // --- 右侧区域：详细信息和操作 ---
    // 商品名称
    m_nameLabel = new QLabel(this);
    m_nameLabel->setText(figure.name);
    m_nameLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    m_nameLabel->setWordWrap(true);

    // 商品价格
    m_priceLabel = new QLabel(this);
    QString priceString = QString("单价: ¥ %1").arg(figure.price, 0, 'f', 2);
    m_priceLabel->setText(priceString);
    m_priceLabel->setStyleSheet("font-size: 14px; color: #c0392b;");

    // 库存信息
    m_stockLabel = new QLabel(this);
    QString stockString = QString("剩余库存: %1 件").arg(figure.quantity);
    m_stockLabel->setText(stockString);

    // --- 购买数量选择 ---
    QLabel* quantityTextLabel = new QLabel("购买数量:", this);
    m_quantitySpinBox = new QSpinBox(this);
    m_quantitySpinBox->setMinimum(1);
    m_quantitySpinBox->setMaximum(figure.quantity); // 最多可购买数量为库存量

    // 如果库存为0，则禁用选择框
    bool isOutOfStock = (figure.quantity == 0);
    m_quantitySpinBox->setEnabled(!isOutOfStock);

    // 用于放置数量选择控件的水平布局
    QHBoxLayout* quantityLayout = new QHBoxLayout();
    quantityLayout->addWidget(quantityTextLabel);
    quantityLayout->addWidget(m_quantitySpinBox);

    // --- 右侧信息的垂直布局 ---
    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->addWidget(m_nameLabel);
    rightLayout->addWidget(m_priceLabel);
    rightLayout->addWidget(m_stockLabel);
    rightLayout->addStretch(); // 添加一个弹簧，将下面的控件推到底部
    rightLayout->addLayout(quantityLayout);

    // --- 底部：确认和取消按钮 ---
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setText("购买");
    buttonBox->button(QDialogButtonBox::Cancel)->setText("取消");

    // 如果没库存，禁用购买按钮
    if (isOutOfStock)
    {
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }

    // 连接按钮信号到对话框的槽
    connect(buttonBox, &QDialogButtonBox::accepted, this, &FigureDetailDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FigureDetailDialog::reject);

    // --- 整体布局 ---
    // 中间内容区域的水平布局
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->addWidget(m_imageLabel);
    contentLayout->addLayout(rightLayout);

    // 整个对话框的主垂直布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(contentLayout);
    mainLayout->addWidget(buttonBox);

    // 应用主布局
    this->setLayout(mainLayout);
}

int FigureDetailDialog::getSelectedQuantity() const
{
    return m_quantitySpinBox->value();
}
