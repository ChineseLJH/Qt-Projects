#include "inventorymanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

// =================================================================
// 【ADDED】 Implementation for the constructor
// =================================================================
InventoryManager::InventoryManager(QObject *parent)
    : QObject(parent)
{
    m_writableFilePath = getWritableDataPath();
}

// =================================================================
// 【ADDED】 Implementation for initialize
// Loads inventory or creates a default one if it doesn't exist.
// =================================================================
void InventoryManager::initialize()
{
    if (!loadInventory()) {
        qDebug() << "Could not load inventory, creating default set.";
        // Create some default figures if loading fails
        m_figures = {
            {"saber", "Artoria Pendragon", 888.0, 10, ":/images/saber.png"},
            {"rem", "Rem", 750.0, 8, ":/images/rem.png"},
            {"miku", "Hatsune Miku", 920.0, 15, ":/images/miku.png"},
            {"kurumi", "Kurumi Tokisaki", 820.0, 5, ":/images/kurumi.png"}
        };
        saveInventory();
    }
    emit inventoryChanged();
}

// =================================================================
// 【ADDED】 Implementation for loadInventory
// Reads the figure data from a JSON file.
// =================================================================
bool InventoryManager::loadInventory()
{
    QFile file(m_writableFilePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open inventory file for reading:" << m_writableFilePath;
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray()) {
        qWarning() << "Inventory file is not a valid JSON array.";
        return false;
    }

    m_figures.clear();
    QJsonArray array = doc.array();
    for (const QJsonValue& value : array) {
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

// =================================================================
// 【ADDED】 Implementation for saveInventory
// Writes the current figure data to a JSON file.
// =================================================================
bool InventoryManager::saveInventory()
{
    qDebug() << "Attempting to SAVE inventory to:" << m_writableFilePath;

    QJsonArray array;
    for (const Figure& fig : m_figures) {
        QJsonObject obj;
        obj["id"] = fig.id;
        obj["name"] = fig.name;
        obj["price"] = fig.price;
        obj["quantity"] = fig.quantity;
        obj["imagePath"] = fig.imagePath;
        array.append(obj);
    }

    QFile file(m_writableFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Cannot open inventory file for writing:" << m_writableFilePath;
        return false;
    }

    file.write(QJsonDocument(array).toJson());
    file.close();
    return true;
}

// =================================================================
// 【ADDED】 Implementation for findFigureById (non-const version)
// Finds a figure by its ID and returns a modifiable pointer.
// =================================================================
Figure* InventoryManager::findFigureById(const QString& figureId)
{
    for (Figure& fig : m_figures) {
        if (fig.id == figureId) {
            return &fig;
        }
    }
    return nullptr;
}

// =================================================================
// 【ADDED】 Implementation for getWritableDataPath
// Gets a reliable path to store the data file.
// =================================================================
QString InventoryManager::getWritableDataPath()
{
    return QString("C:/Qt-Projects/KeShe/data.json");
}

const QList<Figure>& InventoryManager::getFigures() const
{
    return m_figures;
}

// Modified purchase function
bool InventoryManager::purchaseFigure(const QString& figureId, int quantityToPurchase) {
    Figure* figure = findFigureById(figureId);
    if (figure) {
        if (quantityToPurchase <= 0) {
            emit displayMessage("Error: Purchase quantity must be greater than 0!", true);
            return false;
        }
        if (figure->quantity >= quantityToPurchase) {
            figure->quantity -= quantityToPurchase;
            emit displayMessage(QString("Successfully purchased %1 of '%2'!").arg(quantityToPurchase).arg(figure->name));
            emit inventoryChanged();
            saveInventory();
            return true;
        } else {
            emit displayMessage(QString("Not enough stock for '%1'!").arg(figure->name), true);
            return false;
        }
    }
    emit displayMessage("Error: Specified item not found!", true);
    return false;
}

const Figure* InventoryManager::getFigureById(const QString& figureId) const
{
    for (const Figure& fig : m_figures) {
        if (fig.id == figureId) {
            return &fig;
        }
    }
    return nullptr;
}
