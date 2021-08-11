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
} ;

#endif // BOUNDINGBOX_H
