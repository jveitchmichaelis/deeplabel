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
    explicit ImageLabel(QWidget *parent = nullptr);
    virtual int heightForWidth( int width ) const;
    virtual QSize sizeHint() const;
    void setScaledContents(bool scale_factor);
    QList<BoundingBox> getBoundingBoxes(){return bboxes;}
    cv::Mat getImage(void){return image;}
    bool scaleContents(void);

signals:
    void newLabel(BoundingBox);
    void removeLabel(BoundingBox);
    void updateLabel(BoundingBox, BoundingBox);
    void setOccluded(BoundingBox);
    void setCurrentClass(QString);

public slots:
    void setPixmap ( QPixmap & );
    void setImage(cv::Mat &image){this->image = image;}
    void setBoundingBoxes(QList<BoundingBox> input_bboxes);
    void setClassname(QString classname);
    void addLabel(QRect rect, QString classname);
    void zoom(double factor);

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
    bool shouldScaleContents = false;

    QList<BoundingBox> bboxes;
    BoundingBox selected_bbox;
    void drawBoundingBox(BoundingBox bbox);
    void drawBoundingBox(BoundingBox bbox, QColor colour);
    void drawLabel(QPoint location = QPoint());
    QPoint getScaledImageLocation(QPoint location);
    QPixmap scaledPixmap(void);

    QRect clip(QRect bbox);

    QPainter* painter;

    drawState bbox_state = WAIT_START;
    interactionState current_mode = MODE_DRAW;

    QPoint bbox_origin, bbox_final;
    QRubberBand* rubberBand;
    int bbox_width = 0;
    int bbox_height = 0;

    double scale_factor = 1.0;
    double zoom_factor = 1.0;

    int scaled_width;
    int scaled_height;

    void drawBoundingBoxes(QPoint location);
};

#endif // IMAGELABEL_H
