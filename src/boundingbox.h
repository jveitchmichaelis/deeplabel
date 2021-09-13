#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include<QString>
#include<QRect>

struct BoundingBox{
    QRect rect = QRect(0,0,0,0);
    QString classname = "";
    int occluded = 0;
    bool truncated = false;
    int classid = 0;
    double confidence = 0;
};

inline QString printBoundingBox(BoundingBox box){
    return QString("%1, xy(%2, %3) w: %4 h: %5")
                .arg(box.classname)
                .arg(box.rect.left())
                .arg(box.rect.top())
                .arg(box.rect.width())
                .arg(box.rect.height());
}

#endif // BOUNDINGBOX_H
