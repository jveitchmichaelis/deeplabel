#include "imagelabel.h"

ImageLabel::ImageLabel(QWidget *parent) :
    QLabel(parent)
{
    setMinimumSize(1,1);
    setAlignment(Qt::AlignHCenter);
    setAlignment(Qt::AlignVCenter);
    setMouseTracking(true);
    setScaledContents(false);
    setFocusPolicy( Qt::StrongFocus );

    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

}

void ImageLabel::setPixmap ( const QPixmap & p)
{
    bboxes.clear();

    base_pixmap = p;
    pix = base_pixmap;
    QLabel::setPixmap(scaledPixmap());
}

int ImageLabel::heightForWidth( int width ) const
{
    return pix.isNull() ? height() : ((qreal)pix.height()*width)/pix.width();
}

QSize ImageLabel::sizeHint() const
{
    int w = width();
    return QSize( w, heightForWidth(w) );
}

QPixmap ImageLabel::scaledPixmap()
{

    auto rescaled = pix.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    scaled_width = (int) rescaled.width();
    scaled_height = (int) rescaled.height();

    scale_x = (float) scaled_height/pix.height();
    scale_y = (float) scaled_width/pix.width();

    return rescaled;
}

void ImageLabel::resizeEvent(QResizeEvent * e)
{
    if(!pix.isNull())
        QLabel::setPixmap(scaledPixmap());
}

void ImageLabel::mousePressEvent(QMouseEvent *ev){

    if(pix.isNull()) return;

    if(ev->button() == Qt::RightButton){

    }else if(ev->button() == Qt::LeftButton){
        if(bbox_state == WAIT_START){

            bbox_origin = ev->pos();

            rubberBand->setGeometry(QRect(bbox_origin, QSize()));
            rubberBand->show();

            bbox_state = DRAWING_BBOX;
        }else if(bbox_state == DRAWING_BBOX){
            bbox_final = ev->pos();
            bbox_state = WAIT_START;
        }
    }
}

void ImageLabel::mouseMoveEvent(QMouseEvent *ev){

    if(pix.isNull()) return;

    if(bbox_state == DRAWING_BBOX){

        QRect bbox = QRect(bbox_origin, ev->pos()).normalized();

        if(bbox.right() > scaled_width){
            bbox.setRight(scaled_width);
        }

        if(bbox.bottom() > scaled_height){
            bbox.setRight(scaled_height);
        }

        rubberBand->setGeometry(bbox);
    }else{
        BoundingBox bbox;
        foreach(bbox, bboxes){
            if(bbox.rect.contains(ev->pos())){

            }
        }
    }
}

void ImageLabel::drawBoundingBox(BoundingBox bbox){
    QPainter painter;
    painter.begin(&pix);
    QPen pen(Qt::red, 4);
    painter.setPen(pen);
    painter.drawRect(bbox.rect);

    if(bbox.classname != ""){
        painter.setFont(QFont("Helvetica", 40));
        painter.drawText(bbox.rect.bottomLeft(), bbox.classname);
    }

    painter.end();

}

void ImageLabel::setBoundingBoxes(QList<BoundingBox> input_bboxes){
    bboxes = input_bboxes;
    drawBoundingBoxes();
}

void ImageLabel::drawBoundingBoxes(){
    pix = base_pixmap;

    BoundingBox bbox;
    foreach(bbox, bboxes){
        drawBoundingBox(bbox);
    }

    QLabel::setPixmap(scaledPixmap());

}

void ImageLabel::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Space){
        if(rubberBand->width() > 0 and rubberBand->height() > 0){

            auto new_rect = QRect(bbox_origin, bbox_final).normalized();

            new_rect.setRight(new_rect.right() / scale_x);
            new_rect.setLeft(new_rect.left() / scale_x);
            new_rect.setTop(new_rect.top() / scale_y);
            new_rect.setBottom(new_rect.bottom() / scale_y);

            BoundingBox new_bbox;
            new_bbox.classname = current_classname;
            new_bbox.rect = new_rect;

            bboxes.append(new_bbox);
            emit newLabel(new_bbox);

            rubberBand->setGeometry(QRect(bbox_origin, QSize()));
            drawBoundingBoxes();
        }
    }else{
        event->ignore();
    }
}
