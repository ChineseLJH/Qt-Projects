#ifndef INVENTORYMANAGER_H
#define INVENTORYMANAGER_H

#include <QObject>
#include <QList>
#include "figure.h"

class InventoryManager : public QObject
{
    Q_OBJECT

public:
    explicit InventoryManager(QObject *parent = nullptr);
    void initialize();
    const QList<Figure>& getFigures() const;
    bool saveInventory();

    // 修改此函数以接受购买数量
    bool purchaseFigure(const QString& figureId, int quantityToPurchase);

    // 新增一个函数，方便通过ID查找Figure
    const Figure* getFigureById(const QString& figureId) const;


signals:
    void inventoryChanged();
    void displayMessage(const QString& message, bool isError = false);

private:
    bool loadInventory();
    Figure* findFigureById(const QString& figureId);
    QString getWritableDataPath();

    QList<Figure> m_figures;
    QString m_writableFilePath;
};

#endif // INVENTORYMANAGER_H

