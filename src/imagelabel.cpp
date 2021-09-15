#include "imagelabel.h"

ImageLabel::ImageLabel(QWidget *parent) :
    QLabel(parent)
{
    setMinimumSize(1,1);
    setAlignment(Qt::AlignCenter);
    setMouseTracking(true);

    // This is important to preserve the aspect ratio
    setScaledContents(true);

    // Dark background
    setBackgroundRole(QPalette::Shadow);

    // Size policy
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    setFocusPolicy(Qt::StrongFocus);

    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

    setDrawMode();
}

void ImageLabel::setPixmap ( QPixmap & p)
{
    base_pixmap = p;

    drawLabel();

    QPixmap pixmap;
    resize(pixmap.size());
}

int ImageLabel::heightForWidth( int width ) const
{
    return base_pixmap.isNull() ? height() : static_cast<int>((static_cast<qreal>(base_pixmap.height())*width)/base_pixmap.width());
}

QSize ImageLabel::sizeHint() const
{
    int w = width();
    return QSize( w, heightForWidth(w) );
}

void ImageLabel::resizeEvent(QResizeEvent * e)
{
    if(!base_pixmap.isNull()){
        drawLabel();
    }else{
        e->ignore();
    }
}

void ImageLabel::zoom(double factor){
    zoom_factor = factor;
    scaledPixmap();

    if(zoom_factor == 1.0){
        resize(base_pixmap.size());
    }else{
        resize(scaled_pixmap.size());
    }

}

QPixmap ImageLabel::scaledPixmap(void)
{

    if(base_pixmap.isNull())
        return QPixmap();

    if(shouldScaleContents){
        scaled_pixmap = base_pixmap.scaled( size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }else if(zoom_factor != 1.0){
        scaled_pixmap = base_pixmap.scaled( zoom_factor*base_pixmap.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }else{
        scaled_pixmap = base_pixmap;
    }

    scaled_width = scaled_pixmap.width();
    scaled_height = scaled_pixmap.height();

    scale_factor = static_cast<float>(scaled_height)/base_pixmap.height();

    return scaled_pixmap;
}

void ImageLabel::setDrawMode(){
    current_mode = MODE_DRAW;
    setCursor(Qt::CrossCursor);
    rubberBand->setGeometry(QRect(bbox_origin, QSize()));
    rubberBand->show();
}

void ImageLabel::setDrawDragMode(){
    current_mode = MODE_DRAW_DRAG;
    rubberBand->setGeometry(QRect(bbox_origin, QSize()));
    rubberBand->show();
}

void ImageLabel::setSelectMode(){
    current_mode = MODE_SELECT;
    setCursor(Qt::ArrowCursor);
    rubberBand->hide();
}

QPoint ImageLabel::getScaledImageLocation(QPoint location){
    // If the image is fit to window

    QPoint scaled_location = location;

    if(scale_factor != 1.0){

        // Get the location on the image, accounting for padding
        scaled_location.setX(scaled_location.x() - (width() - scaled_width)/2);
        scaled_location.setY(scaled_location.y() - (height() - scaled_height)/2);

        scaled_location.setX(static_cast<int>(scaled_location.x() / scale_factor));
        scaled_location.setY(static_cast<int>(scaled_location.y() / scale_factor));
    }

    return scaled_location;
}

void ImageLabel::mousePressEvent(QMouseEvent *ev){

    if(base_pixmap.isNull()) return;

    QPoint image_location = ev->pos();

    if(current_mode == MODE_SELECT && ev->button() == Qt::LeftButton){

        drawLabel(getScaledImageLocation(image_location));

    }else if(current_mode == MODE_DRAW && ev->button() == Qt::LeftButton){
        if(bbox_state == WAIT_START){

            bbox_origin = image_location;

            rubberBand->setGeometry(QRect(bbox_origin, QSize()));
            rubberBand->show();

            bbox_state = DRAWING_BBOX;
        }else if(bbox_state == DRAWING_BBOX){

            bbox_final = image_location;

            QRect bbox(bbox_origin, bbox_final);

            bbox = clip(bbox.normalized());
            bbox_origin = bbox.topLeft();
            bbox_final = bbox.bottomRight();

            rubberBand->setGeometry(bbox);

            bbox_state = WAIT_START;
        }
    }
}

QRect ImageLabel::clip(QRect bbox){

    auto xpad = (width() - scaled_width)/2;

    bbox.setLeft(std::max(xpad, bbox.left()));
    bbox.setRight(std::min(width()-xpad, bbox.right()));

    auto ypad = (height() - scaled_height)/2;

    bbox.setTop(std::max(ypad, bbox.top()));
    bbox.setBottom(std::min(height()-ypad, bbox.bottom()));

    return bbox;
}

void ImageLabel::mouseReleaseEvent(QMouseEvent *ev){
    /*if(current_mode == MODE_DRAW_DRAG){
        bbox_final = ev->pos();

        QRect bbox(bbox_origin, bbox_final);
        rubberBand->setGeometry(clip(bbox.normalized()));

        bbox_state = WAIT_START;
    }else{
        ev->ignore();
    }*/
    ev->ignore();
}

void ImageLabel::mouseMoveEvent(QMouseEvent *ev){

    if(base_pixmap.isNull()) return;

    if(bbox_state == DRAWING_BBOX && current_mode == MODE_DRAW){
        QRect bbox = QRect(bbox_origin, ev->pos()).normalized();
        rubberBand->setGeometry(bbox);
    }
}

void ImageLabel::drawBoundingBox(BoundingBox bbox){
    auto colour_list = QColor::colorNames();
    QColor colour = QColor( colour_list.at(std::max(0, bbox.classid) % colour_list.size()) );
    drawBoundingBox(bbox, colour);
}

void ImageLabel::drawBoundingBox(BoundingBox bbox, QColor colour){

    if(scaled_pixmap.isNull()) return;

    QPainter painter;
    painter.begin(&scaled_pixmap);
    QPen pen(colour, 2);
    painter.setPen(pen);

    auto scaled_bbox = bbox.rect;

    scaled_bbox.setRight(static_cast<int>(scaled_bbox.right() * scale_factor));
    scaled_bbox.setLeft(static_cast<int>(scaled_bbox.left() * scale_factor));
    scaled_bbox.setTop(static_cast<int>(scaled_bbox.top() * scale_factor));
    scaled_bbox.setBottom(static_cast<int>(scaled_bbox.bottom() * scale_factor));

    if(bbox.classname != ""){

        //painter.fillRect(QRect(scaled_bbox.bottomLeft(), scaled_bbox.bottomRight()+QPoint(0,-10)).normalized(), QBrush(Qt::white));

        painter.setFont(QFont("Helvetica", 10));
        painter.drawText(scaled_bbox.bottomLeft(), bbox.classname);

    }

    painter.drawRect(scaled_bbox);

    painter.end();

}

void ImageLabel::setBoundingBoxes(QList<BoundingBox> input_bboxes){
    bboxes.clear();
    bboxes = input_bboxes;
    drawLabel();
}

void ImageLabel::setClassname(QString classname)
{
    current_classname = classname;

    if(selected_bbox.classname != classname){
        auto new_bbox = selected_bbox;
        new_bbox.classname = classname;

        emit updateLabel(selected_bbox, new_bbox);
    }

}

void ImageLabel::setScaledContents(bool should_scale){
    shouldScaleContents = should_scale;

    if(!shouldScaleContents){
        scale_factor = 1.0;
    }

}

bool ImageLabel::scaleContents(void){
    return shouldScaleContents;
}

void ImageLabel::drawLabel(QPoint location){

    scaledPixmap();

    if(scale_factor == 1.0){
        scaled_pixmap = base_pixmap;
    }

    BoundingBox bbox;
    selected_bbox = BoundingBox();
    foreach(bbox, bboxes){
        if(bbox.rect.contains(location)){
            drawBoundingBox(bbox, Qt::green);
            selected_bbox = bbox;
            emit setCurrentClass(selected_bbox.classname);
        }else{
            drawBoundingBox(bbox);
        }
    }

    QLabel::setPixmap(scaled_pixmap);
}

void ImageLabel::addLabel(QRect rect, QString classname){
    BoundingBox new_bbox;
    new_bbox.classname = classname;
    new_bbox.rect = rect;

    bboxes.append(new_bbox);
    emit newLabel(new_bbox);

    drawLabel();
}

void ImageLabel::keyPressEvent(QKeyEvent *event)
{

    if(event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete){
        emit removeLabel(selected_bbox);
    }else if(event->key() == Qt::Key_Escape){
            rubberBand->setGeometry(QRect(bbox_origin, QSize()));
            bbox_state = WAIT_START;
    }else if(event->key() == Qt::Key_Space && bbox_state == WAIT_START){
        if(rubberBand->width() > 0 && rubberBand->height() > 0){

            if(current_classname == ""){
                qWarning() << "No class selected!";
            }else{

                QRect bbox_rect;

                bbox_rect = QRect(bbox_origin, bbox_final);
                bbox_rect.setTopLeft(getScaledImageLocation(bbox_rect.topLeft()));
                bbox_rect.setBottomRight(getScaledImageLocation(bbox_rect.bottomRight()));

                addLabel(bbox_rect, current_classname);

                rubberBand->setGeometry(QRect(bbox_origin, QSize()));
            }
    }else if(event->key() == 'o'){
        emit setOccluded(selected_bbox);
        drawLabel();
    }
    }else{
        event->ignore();
    }
}
