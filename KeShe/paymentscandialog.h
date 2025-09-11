#ifndef PAYMENTSCANDIALOG_H
#define PAYMENTSCANDIALOG_H

#include <QDialog>
#include <QVector> // 用于存储图片路径

class QLabel;
class QPushButton;

class PaymentScanDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PaymentScanDialog(const QVector<QString>& imagePaths, QWidget *parent = nullptr);
    ~PaymentScanDialog();

private slots:
    void showNextImage();
    void showPreviousImage();

private:
    void setupUI();
    void updateImageDisplay();

    QLabel* m_imageLabel;
    QPushButton* m_prevButton;
    QPushButton* m_nextButton;

    QVector<QString> m_imagePaths; // 存储所有图片路径
    int m_currentIndex; // 当前显示的图片索引
};

#endif // PAYMENTSCANDIALOG_H
