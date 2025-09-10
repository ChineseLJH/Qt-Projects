#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "inventorymanager.h"
#include "figure.h"
#include "figuredetaildialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QCloseEvent>
#include <QScrollArea>
#include <QMouseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_gridLayout(nullptr)
{
    ui->setupUi(this);
    setFixedSize(800, 600);
    setWindowTitle("手办自助售卖系统");
    setupInitialUI(); // This function was missing its implementation
    m_inventoryManager = new InventoryManager(this);
    setupConnections(); // This function was missing its implementation
    m_inventoryManager->initialize();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// =================================================================
// 【ADDED】 Implementation for setupInitialUI
// This function sets up the main scrollable area for the figures.
// =================================================================
void MainWindow::setupInitialUI()
{
    // Create a central widget and a layout for it
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Create a scroll area to contain the grid of figures
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true); // Allow the widget inside to resize
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Disable horizontal scroll bar

    // Create a container widget for the grid layout inside the scroll area
    QWidget* gridContainer = new QWidget();
    m_gridLayout = new QGridLayout(gridContainer);
    m_gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft); // Align items to the top-left
    m_gridLayout->setSpacing(15);

    // Set the grid container as the widget for the scroll area
    scrollArea->setWidget(gridContainer);

    // Add the scroll area to the main layout
    mainLayout->addWidget(scrollArea);

    // Set the central widget of the main window
    setCentralWidget(centralWidget);
}

// =================================================================
// 【ADDED】 Implementation for setupConnections
// This function connects signals and slots between manager and UI.
// =================================================================
void MainWindow::setupConnections()
{
    connect(m_inventoryManager, &InventoryManager::inventoryChanged, this, &MainWindow::updateDisplay);
    connect(m_inventoryManager, &InventoryManager::displayMessage, this, &MainWindow::showStatusMessage);
}

// =================================================================
// 【ADDED】 Implementation for closeEvent
// Ensures inventory is saved when the application closes.
// =================================================================
void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event); // Prevents "unused parameter" warning
    m_inventoryManager->saveInventory();
    QMainWindow::closeEvent(event);
}

// =================================================================
// 【ADDED】 Implementation for clearLayout
// A helper function to remove all widgets from a layout.
// =================================================================
void MainWindow::clearLayout(QLayout* layout)
{
    if (!layout) {
        return;
    }
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

// =================================================================
// 【ADDED】 Implementation for showStatusMessage
// Displays a message in the window's status bar.
// =================================================================
void MainWindow::showStatusMessage(const QString& message, bool isError)
{
    if (isError) {
        ui->statusbar->setStyleSheet("color: red;");
    } else {
        ui->statusbar->setStyleSheet(""); // Reset to default color
    }
    ui->statusbar->showMessage(message, 5000); // Show for 5 seconds
}


// 【CORE MODIFICATION】 Update the item display area
void MainWindow::updateDisplay()
{
    clearLayout(m_gridLayout);

    const auto& figures = m_inventoryManager->getFigures();
    int row = 0;
    int col = 0;
    const int maxColumns = 4;

    for (const Figure& fig : figures) {
        ClickableWidget* cardWidget = new ClickableWidget();
        cardWidget->setFixedSize(160, 180);
        cardWidget->setStyleSheet(
            "ClickableWidget { background-color: white; border: 1px solid #ddd; border-radius: 8px; }"
            "ClickableWidget:hover { border: 1px solid #0078d7; }"
            );
        cardWidget->setCursor(Qt::PointingHandCursor);

        QVBoxLayout* cardLayout = new QVBoxLayout(cardWidget);

        QLabel* imageLabel = new QLabel();
        QPixmap pixmap(fig.imagePath);
        if(pixmap.isNull()){
            imageLabel->setText("No Image");
        } else {
            imageLabel->setPixmap(pixmap.scaled(140, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        imageLabel->setAlignment(Qt::AlignCenter);

        QString info = QString("<b>%1</b><br>¥%2")
                           .arg(fig.name)
                           .arg(fig.price, 0, 'f', 2);
        QLabel* infoLabel = new QLabel(info);
        infoLabel->setAlignment(Qt::AlignCenter);
        infoLabel->setWordWrap(true);

        cardLayout->addWidget(imageLabel);
        cardLayout->addWidget(infoLabel);

        connect(cardWidget, &ClickableWidget::clicked, this, [this, figId = fig.id]() {
            showFigureDetails(figId);
        });

        m_gridLayout->addWidget(cardWidget, row, col);

        col++;
        if (col >= maxColumns) {
            col = 0;
            row++;
        }
    }
}

// 【NEW SLOT】 To show the item detail dialog
void MainWindow::showFigureDetails(const QString& figureId)
{
    const Figure* fig = m_inventoryManager->getFigureById(figureId);
    if (!fig) {
        return;
    }

    FigureDetailDialog dialog(*fig, this);
    if (dialog.exec() == QDialog::Accepted) {
        int quantity = dialog.getSelectedQuantity();
        m_inventoryManager->purchaseFigure(figureId, quantity);
    }
}
