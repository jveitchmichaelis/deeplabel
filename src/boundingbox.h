#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include<QRect>

typedef struct{
    QRect rect = QRect(0,0,0,0);
    QString classname = "";
    QString imagepath = "";
    int occluded = 0;
    bool truncated = false;
} BoundingBox;

#endif // BOUNDINGBOX_H
