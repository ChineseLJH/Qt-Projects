#include "seatselectionview.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QQueue>
#include <QSet>
#include <algorithm>
#include <queue>
#include <cmath>
#include <QMap>

struct SeatNode {
    int r;
    int c;
    int cost;
    bool operator<(const SeatNode& other) const {
        return cost > other.cost; // 最小堆
    }
};

SeatSelectionView::SeatSelectionView(QWidget *parent) 
    : QWidget(parent), currentHall(nullptr), currentBfsIndex(-1), lastSearchedCount(-1)
{
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setObjectName("seatSelectionView");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 20, 40, 20);
    
    QHBoxLayout *topBar = new QHBoxLayout();
    
    QPushButton *btnBack = new QPushButton("← 返回排片", this);
    btnBack->setObjectName("btnBack");
    btnBack->setFixedSize(120, 40);
    btnBack->setCursor(Qt::PointingHandCursor);
    connect(btnBack, &QPushButton::clicked, this, &SeatSelectionView::requestBackToMovieList);

    QLabel *lblSmart = new QLabel("购票人数:", this);
    lblSmart->setStyleSheet("color: #ECF0F1; font-size: 16px;");
    
    peopleCountSpin = new QSpinBox(this);
    // 需求修复：支持两位数，范围设为 1~99
    peopleCountSpin->setRange(1, 99); 
    peopleCountSpin->setFixedSize(80, 40);
    peopleCountSpin->setStyleSheet("background-color: #34495E; color: white; font-size: 16px; border-radius: 4px; padding-left: 10px;");

    btnSmartSelect = new QPushButton("智能选座 (BFS搜索)", this);
    btnSmartSelect->setObjectName("btnSmart");
    btnSmartSelect->setFixedSize(180, 40);
    btnSmartSelect->setCursor(Qt::PointingHandCursor);
    connect(btnSmartSelect, &QPushButton::clicked, this, &SeatSelectionView::executeSmartSelection);

    topBar->addWidget(btnBack);
    topBar->addStretch();
    topBar->addWidget(lblSmart);
    topBar->addWidget(peopleCountSpin);
    topBar->addWidget(btnSmartSelect);

    QLabel *screenLabel = new QLabel("中央荧幕", this);
    screenLabel->setAlignment(Qt::AlignCenter);
    screenLabel->setStyleSheet("background-color: #7F8C8D; color: white; padding: 10px; border-radius: 20px; font-weight: bold; margin: 0 100px;");
    
    seatMainLayout = new QVBoxLayout(); 
    seatMainLayout->setSpacing(8);

    QPushButton *btnNext = new QPushButton("确认选座，去结算", this);
    btnNext->setObjectName("movieBookBtn");
    btnNext->setFixedSize(200, 50);
    btnNext->setCursor(Qt::PointingHandCursor);

    connect(btnNext, &QPushButton::clicked, this, [=]() {
        if (selectedSeats.size() != peopleCountSpin->value()) {
            QMessageBox::warning(this, "提示", QString("您需要选择 %1 个座位，当前已选 %2 个。")
                                                .arg(peopleCountSpin->value()).arg(selectedSeats.size()));
            return;
        }
        emit requestCheckout(selectedSeats);
    });

    mainLayout->addLayout(topBar);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(screenLabel);
    mainLayout->addSpacing(40);
    mainLayout->addLayout(seatMainLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(btnNext, 0, Qt::AlignHCenter);
}

void SeatSelectionView::loadHallData(Hall *hall) {
    currentHall = hall;
    selectedSeats.clear();
    
    // 切换影厅时，必须清空上一场的 BFS 缓存
    cachedBfsResults.clear();
    currentBfsIndex = -1;
    lastSearchedCount = -1;
    
    renderSeats();
}

void SeatSelectionView::renderSeats() {
    if (!currentHall) return;

    QLayoutItem *child;
    while ((child = seatMainLayout->takeAt(0)) != nullptr) {
        if (child->layout()) {
            QLayoutItem *subChild;
            while ((subChild = child->layout()->takeAt(0)) != nullptr) {
                delete subChild->widget();
                delete subChild;
            }
        }
        delete child;
    }

    for (int i = 0; i < currentHall->rows; ++i) {
        QHBoxLayout *rowLayout = new QHBoxLayout();
        rowLayout->setAlignment(Qt::AlignCenter); 
        rowLayout->setSpacing(8);

        for (int j = 0; j < currentHall->colsPerRow[i]; ++j) {
            QPushButton *seatBtn = new QPushButton(this);
            seatBtn->setFixedSize(35, 35);

            int status = currentHall->seatMatrix[i][j];
            QPoint pos(i, j);

            if (status == 1) {
                seatBtn->setStyleSheet("background-color: #E74C3C; border-radius: 6px;"); 
                seatBtn->setEnabled(false);
            } else if (selectedSeats.contains(pos)) {
                seatBtn->setStyleSheet("background-color: #2ECC71; border-radius: 6px; border: 2px solid #FFFFFF;"); 
            } else {
                seatBtn->setStyleSheet("background-color: #BDC3C7; border-radius: 6px;"); 
            }

            // ==========================================
            // 需求修复：严格的状态拦截与取消逻辑
            // ==========================================
            connect(seatBtn, &QPushButton::clicked, this, [=]() {
                int maxSeats = peopleCountSpin->value();
                
                if (selectedSeats.contains(pos)) {
                    // 如果已经是绿色，任何时候都允许取消（移除并变回灰色）
                    selectedSeats.removeAll(pos);
                } else {
                    // 如果点击的是灰色，必须先拦截容量限制
                    if (selectedSeats.size() >= maxSeats) {
                        QMessageBox::warning(this, "选座受限", 
                            "已达到购票人数上限！\n请先点击已选的绿色座位取消，然后再重新选座。");
                        return; // 强行中断，阻止变绿
                    }
                    selectedSeats.append(pos);
                }
                renderSeats(); 
            });

            rowLayout->addWidget(seatBtn);
        }
        seatMainLayout->addLayout(rowLayout);
    }
}

void SeatSelectionView::executeSmartSelection() {
    if (!currentHall) return;
    int n = peopleCountSpin->value();
    
    // 触发重新计算的条件：人数变更，或缓存耗尽
    if (n != lastSearchedCount || cachedBfsResults.isEmpty()) {
        cachedBfsResults.clear();
        currentBfsIndex = -1;
        lastSearchedCount = n;

        // 预估最大允许的跨排数（电影院通常不建议同批次跨越 4 排以上）
        int maxAllowedRows = std::min(n, 4); 

        // 全局扫描：以每一个空座位为绝对原点进行探测
        for (int startR = 0; startR < currentHall->rows; ++startR) {
            for (int startC = 0; startC < currentHall->colsPerRow[startR]; ++startC) {
                if (currentHall->seatMatrix[startR][startC] != 0) continue;

                // 核心重构：针对当前起点，动态施加不同宽度的物理包围盒
                // targetRows = 1 时，允许横向无限扩展（追求单排）
                // targetRows = 2 时，横向扩展被强制截断，迫使算法向上下排溢出，形成均等矩形
                for (int targetRows = 1; targetRows <= maxAllowedRows; ++targetRows) {
                    // 计算当前预期排数下的最大单侧列偏移量
                    int maxColOffset = (n / targetRows) / 2 + 1; 

                    QList<QPoint> currentSelection;
                    std::priority_queue<SeatNode> pq;
                    QSet<QString> visited;

                    pq.push({startR, startC, 0});
                    visited.insert(QString("%1,%2").arg(startR).arg(startC));

                    while (!pq.empty() && currentSelection.size() < n) {
                        SeatNode curr = pq.top();
                        pq.pop();
                        currentSelection.append(QPoint(curr.r, curr.c));

                        QPoint directions[] = {
                            QPoint(0, 1), QPoint(0, -1), QPoint(-1, 0), QPoint(1, 0)
                        };

                        for (const QPoint& dir : directions) {
                            int nr = curr.r + dir.x();
                            int nc = curr.c + dir.y();

                            // 1. 梯形边界越界校验
                            if (nr >= 0 && nr < currentHall->rows && nc >= 0 && nc < currentHall->colsPerRow[nr]) {
                                // 2. 包围盒水平截断校验（限制单排最大人数）
                                if (std::abs(nc - startC) <= maxColOffset) {
                                    if (currentHall->seatMatrix[nr][nc] == 0) {
                                        QString hash = QString("%1,%2").arg(nr).arg(nc);
                                        if (!visited.contains(hash)) {
                                            visited.insert(hash);
                                            // 代价函数：依然施加 100 的跨排惩罚，保证在包围盒内优先填满同行
                                            int cost = std::abs(nr - startR) * 100 + std::abs(nc - startC);
                                            pq.push({nr, nc, cost});
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // 如果成功收集到 N 个连续座位，压入缓存池
                    if (currentSelection.size() == n) {
                        std::sort(currentSelection.begin(), currentSelection.end(), [](const QPoint& a, const QPoint& b){
                            if (a.x() == b.x()) return a.y() < b.y();
                            return a.x() < b.x();
                        });

                        bool isDuplicate = false;
                        for (const auto& existingCombo : cachedBfsResults) {
                            bool match = true;
                            for (int i = 0; i < n; ++i) {
                                if (existingCombo[i] != currentSelection[i]) {
                                    match = false; break;
                                }
                            }
                            if (match) { isDuplicate = true; break; }
                        }
                        if (!isDuplicate) cachedBfsResults.append(currentSelection);
                    }
                }
            }
        }

        // ==========================================
        // 结果集多维图形学评分与排序
        // ==========================================
        std::sort(cachedBfsResults.begin(), cachedBfsResults.end(), [this](const QList<QPoint>& a, const QList<QPoint>& b) {
            // 内部 Lambda：计算集合占据的排数与人数分布的数学方差
            auto getShapeStats = [](const QList<QPoint>& list, int& rowCount, double& variance) {
                QMap<int, int> rowCounts;
                for (const auto& p : list) rowCounts[p.x()]++;
                rowCount = rowCounts.size();
                double mean = (double)list.size() / rowCount;
                variance = 0;
                for (int count : rowCounts.values()) {
                    variance += (count - mean) * (count - mean);
                }
            };

            int rowsA, rowsB;
            double varA, varB;
            getShapeStats(a, rowsA, varA);
            getShapeStats(b, rowsB, varB);

            // 规则 1：物理排数越少越好。如果存在能完整装下 20 人的单排，直接胜出。
            if (rowsA != rowsB) return rowsA < rowsB;

            // 规则 2：排数相同时，方差越小越好。
            // 例如 [18, 2] 的方差为 128，而 [10, 10] 的方差为 0。此规则彻底抹杀 18+2。
            if (std::abs(varA - varB) > 0.1) return varA < varB;

            // 规则 3：形状一样时，计算相对于影厅中轴线的偏移积分，优先推荐中央区座位。
            auto getCenterOffset = [this](const QList<QPoint>& list) {
                int totalOffset = 0;
                for (const auto& p : list) {
                    int centerCol = this->currentHall->colsPerRow[p.x()] / 2;
                    totalOffset += std::abs(p.y() - centerCol);
                }
                return totalOffset;
            };
            return getCenterOffset(a) < getCenterOffset(b);
        });
    }

    // 物理阻断与状态轮询
    if (cachedBfsResults.isEmpty()) {
        QMessageBox::critical(this, "智能推荐失败", 
            "当前剩余座位排布无法满足您的连续选座需求。\n请尝试减少人数，或手动挑选散座。");
        return;
    }

    currentBfsIndex = (currentBfsIndex + 1) % cachedBfsResults.size();
    selectedSeats = cachedBfsResults[currentBfsIndex];
    renderSeats();
}