#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <QRect>
#include <QString>

enum SelectedEdge {
    EDGE_LEFT,
    EDGE_TOP,
    EDGE_RIGHT,
    EDGE_BOTTOM,
    EDGE_NONE,
};

enum SelectedCorner {
    CORNER_TOPLEFT,
    CORNER_TOPRIGHT,
    CORNER_BOTTOMLEFT,
    CORNER_BOTTOMRIGHT,
    CORNER_NONE,
};

struct BoundingBox{
    QRect rect = QRect(0,0,0,0);
    QString classname = "";
    int occluded = 0;
    bool truncated = false;
    int classid = 0;
    double confidence = 0;

    // Drawing helpers
    bool is_selected = false;
    SelectedEdge selected_edge = EDGE_NONE;
    SelectedCorner selected_corner = CORNER_NONE;

    // ID
    int label_id = -1;
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
