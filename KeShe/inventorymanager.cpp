#include "inventorymanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

InventoryManager::InventoryManager(QObject *parent)
    : QObject(parent)
{
    // 在构造时就确定数据文件的路径
    m_writableFilePath = getWritableDataPath();
}

void InventoryManager::initialize()
{
    bool isLoaded = loadInventory();
    if (!isLoaded)
    {
        // 如果加载文件失败，通过信号发送错误消息
        emit displayMessage("错误：无法加载库存文件 data.json，请检查文件是否存在。", true);
    }

    // 发送信号通知UI刷新显示
    emit inventoryChanged();
}

bool InventoryManager::loadInventory()
{
    QFile inventoryFile(m_writableFilePath);

    // 尝试以只读模式打开文件
    bool canOpenFile = inventoryFile.open(QIODevice::ReadOnly);
    if (!canOpenFile)
    {
        qWarning() << "无法打开库存文件进行读取:" << m_writableFilePath;
        return false;
    }

    // 读取文件所有内容
    QByteArray fileData = inventoryFile.readAll();
    inventoryFile.close();

    // 将读取的数据解析为JSON文档
    QJsonDocument doc = QJsonDocument::fromJson(fileData);

    // 检查JSON的根元素是否为数组
    if (!doc.isArray())
    {
        qWarning() << "库存文件格式错误，根元素不是一个有效的JSON数组。";
        return false;
    }

    // 清空旧数据
    m_figures.clear();

    QJsonArray jsonArray = doc.array();
    // 遍历JSON数组，解析每个商品对象
    for (const QJsonValue& value : jsonArray)
    {
        QJsonObject obj = value.toObject();

        Figure fig;
        fig.id = obj["id"].toString();
        fig.name = obj["name"].toString();
        fig.price = obj["price"].toDouble();
        fig.quantity = obj["quantity"].toInt();
        fig.imagePath = obj["imagePath"].toString();

        m_figures.append(fig);
    }

    return true;
}

bool InventoryManager::saveInventory()
{
    qDebug() << "尝试保存库存到文件:" << m_writableFilePath;

    QJsonArray figureArray;

    // 遍历内存中的商品列表，转换为JSON对象
    for (const Figure& fig : m_figures)
    {
        QJsonObject figureObject;
        figureObject["id"] = fig.id;
        figureObject["name"] = fig.name;
        figureObject["price"] = fig.price;
        figureObject["quantity"] = fig.quantity;
        figureObject["imagePath"] = fig.imagePath;

        figureArray.append(figureObject);
    }

    QJsonDocument doc(figureArray);

    QFile inventoryFile(m_writableFilePath);
    // 以只写和覆盖模式打开文件
    bool canOpenFile = inventoryFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    if (!canOpenFile)
    {
        qWarning() << "无法打开库存文件进行写入:" << m_writableFilePath;
        return false;
    }

    // 将JSON数据写入文件
    inventoryFile.write(doc.toJson());
    inventoryFile.close();

    return true;
}

Figure* InventoryManager::findFigureById(const QString& figureId)
{
    // 遍历查找具有匹配ID的商品
    for (int i = 0; i < m_figures.size(); ++i)
    {
        if (m_figures[i].id == figureId)
        {
            // 返回一个指向该商品的可修改指针
            return &m_figures[i];
        }
    }
    // 如果找不到，返回空指针
    return nullptr;
}

QString InventoryManager::getWritableDataPath()
{
    // 为了简单起见，这里使用一个固定的硬编码路径
    return QString("C:/Qt-Projects/KeShe/data.json");
}

const QList<Figure>& InventoryManager::getFigures() const
{
    return m_figures;
}

bool InventoryManager::purchaseFigure(const QString& figureId, int quantityToPurchase)
{
    Figure* figure = findFigureById(figureId);

    // 检查商品是否存在
    if (figure == nullptr)
    {
        emit displayMessage("错误: 指定的商品未找到!", true);
        return false;
    }

    // 检查购买数量是否合法
    if (quantityToPurchase <= 0)
    {
        emit displayMessage("错误: 购买数量必须大于0!", true);
        return false;
    }

    // 检查库存是否充足
    bool hasEnoughStock = (figure->quantity >= quantityToPurchase);
    if (hasEnoughStock)
    {
        // 扣减库存
        figure->quantity = figure->quantity - quantityToPurchase;

        // 发送成功消息
        QString successMsg = QString("成功购买 '%2' x %1!").arg(quantityToPurchase).arg(figure->name);
        emit displayMessage(successMsg);

        // 通知UI刷新
        emit inventoryChanged();

        // 保存到文件
        saveInventory();

        return true;
    }
    else
    {
        // 库存不足
        QString errorMsg = QString("'%1' 库存不足!").arg(figure->name);
        emit displayMessage(errorMsg, true);
        return false;
    }
}

const Figure* InventoryManager::getFigureById(const QString& figureId) const
{
    for (const Figure& fig : m_figures)
    {
        if (fig.id == figureId)
        {
            return &fig;
        }
    }
    return nullptr;
}
