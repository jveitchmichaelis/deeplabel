#ifndef IMAGELABEL_H
#define IMAGELABEL_H


#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QRubberBand>
#include <QResizeEvent>

#include<boundingbox.h>

enum drawState{
    WAIT_START,
    DRAWING_BBOX,
};

class ImageLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ImageLabel(QWidget *parent = 0);
    virtual int heightForWidth( int width ) const;
    virtual QSize sizeHint() const;
    QPixmap scaledPixmap();

signals:
    void newLabel(BoundingBox);

public slots:
    void setPixmap ( const QPixmap & );
    void setBoundingBoxes(QList<BoundingBox> input_bboxes);
    void setClassname(QString classname){current_classname = classname;}

    void resizeEvent(QResizeEvent *);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    QPixmap pix;
    QPixmap base_pixmap;
    QString current_classname;

    QList<BoundingBox> bboxes;
    void drawBoundingBox(BoundingBox);
    void drawBoundingBoxes();

    QPainter* painter;

    drawState bbox_state = WAIT_START;

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
