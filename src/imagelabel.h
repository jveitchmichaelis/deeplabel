#ifndef IMAGELABEL_H
#define IMAGELABEL_H


#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QRubberBand>
#include <QResizeEvent>

#include <opencv2/opencv.hpp>
#include<boundingbox.h>

enum drawState{
    WAIT_START,
    DRAWING_BBOX,
};

enum interactionState{
    MODE_DRAW,
    MODE_DRAW_DRAG,
    MODE_SELECT,
};

class ImageLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ImageLabel(QWidget *parent = 0);
    virtual int heightForWidth( int width ) const;
    virtual QSize sizeHint() const;
    QPixmap scaledPixmap();
    QList<BoundingBox> getBoundingBoxes(){return bboxes;}
    cv::Mat getImage(){return image;}

signals:
    void newLabel(BoundingBox);
    void removeLabel(BoundingBox);
    void setOccluded(BoundingBox);

public slots:
    void setPixmap ( QPixmap & );
    void setImage(cv::Mat &image){this->image = image;}
    void setBoundingBoxes(QList<BoundingBox> input_bboxes);
    void setPotentialBoundingBoxes(QList<BoundingBox> input_bboxes);
    void setClassname(QString classname){current_classname = classname;}
    void addLabel(QRect rect, QString classname);

    void setDrawMode();
    void setDrawDragMode();
    void setSelectMode();

    void resizeEvent(QResizeEvent *);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    cv::Mat image;
    QPixmap pix;
    QPixmap base_pixmap;
    QPixmap scaled_pixmap;
    QString current_classname;

    QList<BoundingBox> bboxes;
    BoundingBox selected_bbox;
    void drawBoundingBox(BoundingBox bbox);
    void drawBoundingBox(BoundingBox bbox, QColor colour);
    void drawBoundingBoxes(QPoint location = QPoint());


    QRect clip(QRect bbox);

    QPainter* painter;

    drawState bbox_state = WAIT_START;
    interactionState current_mode = MODE_DRAW;

    QPoint bbox_origin, bbox_final;
    QRubberBand* rubberBand;
    int bbox_width = 0;
    int bbox_height = 0;

    float scale_x;
    float scale_y;

    int scaled_width;
    int scaled_height;

};

#endif // IMAGELABEL_H
