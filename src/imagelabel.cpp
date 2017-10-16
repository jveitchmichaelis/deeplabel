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

void ImageLabel::setDrawMode(){
    current_mode = MODE_DRAW;
    rubberBand->setGeometry(QRect(bbox_origin, QSize()));
    rubberBand->show();
}

void ImageLabel::setSelectMode(){
    current_mode = MODE_SELECT;
    rubberBand->hide();
}

void ImageLabel::setPixmap ( const QPixmap & p)
{
    bboxes.clear();
    base_pixmap = p;

    drawBoundingBoxes();
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
    scaled_pixmap = pix.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    scaled_width = (int) scaled_pixmap.width();
    scaled_height = (int) scaled_pixmap.height();

    scale_x = (float) scaled_height/pix.height();
    scale_y = (float) scaled_width/pix.width();

    return scaled_pixmap;
}

void ImageLabel::resizeEvent(QResizeEvent * e)
{
    if(!pix.isNull()){
        drawBoundingBoxes();
    }else{
        e->ignore();
    }
}

void ImageLabel::mousePressEvent(QMouseEvent *ev){

    if(pix.isNull()) return;

    if(current_mode == MODE_SELECT && ev->button() == Qt::LeftButton){

        QPoint image_location = ev->pos();

        image_location.setX(image_location.x() / scale_x);
        image_location.setY(image_location.y() / scale_y);

        drawBoundingBoxes(image_location);
    }

    if(current_mode == MODE_DRAW && ev->button() == Qt::LeftButton){
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

    if(bbox_state == DRAWING_BBOX && current_mode == MODE_DRAW){

        QRect bbox = QRect(bbox_origin, ev->pos()).normalized();

        if(bbox.right() > scaled_width){
            bbox.setRight(scaled_width);
        }

        if(bbox.bottom() > scaled_height){
            bbox.setRight(scaled_height);
        }

        rubberBand->setGeometry(bbox);
    }
}

void ImageLabel::drawBoundingBox(BoundingBox bbox, QColor colour){
    QPainter painter;
    painter.begin(&scaled_pixmap);
    QPen pen(colour, 2);
    painter.setPen(pen);

    auto scaled_bbox = bbox.rect;

    scaled_bbox.setRight(scaled_bbox.right() * scale_x);
    scaled_bbox.setLeft(scaled_bbox.left() * scale_x);
    scaled_bbox.setTop(scaled_bbox.top() * scale_y);
    scaled_bbox.setBottom(scaled_bbox.bottom() * scale_y);

    painter.drawRect(scaled_bbox);

    if(bbox.classname != ""){
        painter.setFont(QFont("Helvetica", 10));
        painter.drawText(scaled_bbox.bottomLeft(), bbox.classname);
    }

    painter.end();

}

void ImageLabel::setBoundingBoxes(QList<BoundingBox> input_bboxes){
    bboxes = input_bboxes;
    drawBoundingBoxes();
}

void ImageLabel::drawBoundingBoxes(QPoint location){
    pix = base_pixmap;
    scaledPixmap();

    BoundingBox bbox;
    foreach(bbox, bboxes){
        if(bbox.rect.contains(location)){
            drawBoundingBox(bbox, Qt::green);
            selected_bbox = bbox;
        }else{
            drawBoundingBox(bbox, Qt::red);
        }
    }

    QLabel::setPixmap(scaled_pixmap);

}

void ImageLabel::keyPressEvent(QKeyEvent *event)
{

    if(event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete){
        emit removeLabel(selected_bbox);
    }else if(event->key() == Qt::Key_Escape){
        rubberBand->setGeometry(QRect(bbox_origin, QSize()));
        bbox_state = WAIT_START;

    }else if(event->key() == Qt::Key_Space){
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
