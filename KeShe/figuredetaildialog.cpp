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
    setWindowTitle("商品详情");
    setFixedSize(450, 250); // 给对话框一个固定的大小
    setupUi(figure);
}

FigureDetailDialog::~FigureDetailDialog() {}

void FigureDetailDialog::setupUi(const Figure& figure) {
    // --- 左侧：大图 ---
    m_imageLabel = new QLabel;
    QPixmap pixmap(figure.imagePath);
    if(pixmap.isNull()) {
        m_imageLabel->setText("无图片");
    } else {
        m_imageLabel->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    m_imageLabel->setFixedSize(200, 200);

    // --- 右侧：详细信息和操作 ---
    m_nameLabel = new QLabel(figure.name);
    m_nameLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    m_nameLabel->setWordWrap(true);

    m_priceLabel = new QLabel(QString("单价: ¥ %1").arg(figure.price, 0, 'f', 2));
    m_priceLabel->setStyleSheet("font-size: 14px; color: #c0392b;");

    m_stockLabel = new QLabel(QString("剩余库存: %1 件").arg(figure.quantity));

    // 购买数量选择
    QHBoxLayout* quantityLayout = new QHBoxLayout;
    quantityLayout->addWidget(new QLabel("购买数量:"));
    m_quantitySpinBox = new QSpinBox;
    m_quantitySpinBox->setMinimum(1);
    m_quantitySpinBox->setMaximum(figure.quantity); // 最多只能购买库存数量
    m_quantitySpinBox->setEnabled(figure.quantity > 0); // 库存为0时禁用
    quantityLayout->addWidget(m_quantitySpinBox);

    // 右侧信息的垂直布局
    QVBoxLayout* rightLayout = new QVBoxLayout;
    rightLayout->addWidget(m_nameLabel);
    rightLayout->addWidget(m_priceLabel);
    rightLayout->addWidget(m_stockLabel);
    rightLayout->addStretch();
    rightLayout->addLayout(quantityLayout);

    // --- 底部：确认和取消按钮 ---
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &FigureDetailDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FigureDetailDialog::reject);
    // 如果库存为0，禁用OK按钮
    if (figure.quantity == 0) {
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }

    // --- 整体布局 ---
    QVBoxLayout* mainVLayout = new QVBoxLayout;
    QHBoxLayout* contentLayout = new QHBoxLayout;
    contentLayout->addWidget(m_imageLabel);
    contentLayout->addLayout(rightLayout);

    mainVLayout->addLayout(contentLayout);
    mainVLayout->addWidget(buttonBox);

    setLayout(mainVLayout);
}

int FigureDetailDialog::getSelectedQuantity() const {
    return m_quantitySpinBox->value();
}
