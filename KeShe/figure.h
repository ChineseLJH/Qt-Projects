#ifndef FIGURE_H
#define FIGURE_H

#include <QString>

// 手办数据结构体 (模型的一部分)
// 这是一个纯数据类，不需要继承QObject
struct Figure {
    QString id;         // 唯一ID，用于内部识别
    QString name;       // 商品名称
    double  price;      // 价格
    int     quantity;   // 库存数量
    QString imagePath;  // 图片文件在资源系统中的路径 (例如 ":/images/saber.png")
};

#endif // FIGURE_H

