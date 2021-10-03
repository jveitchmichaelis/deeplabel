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

    drawLabels();

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
        drawLabels();
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

void ImageLabel::setDrawMode()
{
    current_mode = MODE_DRAW;
    setCursor(Qt::CrossCursor);
    rubberBand->setGeometry(QRect(bbox_origin, QSize()));
    rubberBand->show();
    setBoundingBoxes(bboxes);
}

void ImageLabel::setDrawDragMode()
{
    current_mode = MODE_DRAW_DRAG;
    rubberBand->setGeometry(QRect(bbox_origin, QSize()));
    rubberBand->show();
    setBoundingBoxes(bboxes);
}

void ImageLabel::setSelectMode(){
    current_mode = MODE_SELECT;
    setCursor(Qt::PointingHandCursor);
    rubberBand->hide();
    setBoundingBoxes(bboxes);
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

    QPoint click_location = ev->pos();

    if (current_mode == MODE_SELECT && ev->button() == Qt::LeftButton) {
        bool clicked = true;
        bool state_change = false;
        auto active_box = checkBoundingBoxes(click_location, state_change, clicked);

        if (state_change) {
            drawLabels();
        }

        if (active_box.label_id > 0) {
            edit_state = EDITING_BOX;
            editing_bbox = active_box;
            selected_bbox = active_box;
            initial_edit_centre = click_location;

        } else {
            selected_bbox = BoundingBox();
            edit_state = EDIT_WAIT;
        }
    } else if (current_mode == MODE_DRAW && ev->button() == Qt::LeftButton) {
        if (bbox_state == WAIT_START) {
            bbox_origin = click_location;

            rubberBand->setGeometry(QRect(bbox_origin, QSize()));
            rubberBand->show();

            bbox_state = DRAWING_BBOX;
        } else if (bbox_state == DRAWING_BBOX) {
            bbox_final = click_location;

            QRect bbox(bbox_origin, bbox_final);

            bbox = clip(bbox.normalized());
            bbox_origin = bbox.topLeft();
            bbox_final = bbox.bottomRight();

            rubberBand->setGeometry(bbox);

            bbox_state = WAIT_START;
        }
    }
}

void ImageLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    if (current_mode == MODE_SELECT && edit_state == EDITING_BOX) {
        auto temp = editing_bbox;
        editing_bbox = BoundingBox();
        updateLabel(selected_bbox, temp);
        edit_state = EDIT_WAIT;
        setBoundingBoxes(bboxes);

    } else {
        ev->ignore();
    }
}

void ImageLabel::mouseMoveEvent(QMouseEvent *ev)
{
    if (base_pixmap.isNull())
        return;

    if (bbox_state == DRAWING_BBOX && current_mode == MODE_DRAW) {
        QRect bbox = QRect(bbox_origin, ev->pos()).normalized();
        rubberBand->setGeometry(bbox);
    }

    if (current_mode == MODE_SELECT) {
        if (edit_state == EDIT_WAIT) {
            bool state_change;
            checkBoundingBoxes(ev->pos(), state_change);
            if (state_change) {
                drawLabels();
            }
        } else if (edit_state == EDITING_BOX) {
            editBoxCoords(editing_bbox, ev->pos());
            drawLabels();
        }
    }
}

void ImageLabel::editBoxCoords(BoundingBox &bbox, QPoint location)
{
    auto scaled_location = getScaledImageLocation(location);

    // Edge cases
    if (bbox.selected_edge == EDGE_TOP) {
        bbox.rect.setTop(scaled_location.y());
    } else if (bbox.selected_edge == EDGE_BOTTOM) {
        bbox.rect.setBottom(scaled_location.y());
    } else if (bbox.selected_edge == EDGE_LEFT) {
        bbox.rect.setLeft(scaled_location.x());
    } else if (bbox.selected_edge == EDGE_RIGHT) {
        bbox.rect.setRight(scaled_location.x());
    }

    // Corner cases
    if (bbox.selected_corner == CORNER_BOTTOMLEFT) {
        bbox.rect.setBottomLeft(scaled_location);
    } else if (bbox.selected_corner == CORNER_BOTTOMRIGHT) {
        bbox.rect.setBottomRight(scaled_location);
    } else if (bbox.selected_corner == CORNER_TOPLEFT) {
        bbox.rect.setTopLeft(scaled_location);
    } else if (bbox.selected_corner == CORNER_TOPRIGHT) {
        bbox.rect.setTopRight(scaled_location);
    }

    if (bbox.is_selected) {
        int width = bbox.rect.width();
        int height = bbox.rect.height();

        auto delta = scaled_location - getScaledImageLocation(initial_edit_centre);

        bbox.rect.setTopLeft(
            {bbox.rect.topLeft().x() + delta.x(), bbox.rect.topLeft().y() + delta.y()});
        bbox.rect.setWidth(width);
        bbox.rect.setHeight(height);

        initial_edit_centre = location;
    }

    bbox.rect = bbox.rect.normalized();
}

bool ImageLabel::isSelected(BoundingBox bbox, QPoint location, int padding)
{
    auto rect = QRect(QPoint(bbox.rect.topLeft().x() + padding, bbox.rect.topLeft().y() + padding),
                      QPoint(bbox.rect.bottomRight().x() - padding,
                             bbox.rect.bottomRight().y() - padding));

    return rect.contains(location);
}

BoundingBox ImageLabel::checkBoundingBoxes(QPoint location, bool &state_change, bool click)
{
    state_change = false;
    bool something_selected = false;
    BoundingBox active;

    for (auto &bbox : bboxes) {
        // Check for selected (centre hitbox)
        if (click) {
            auto prev_select = bbox.is_selected;
            bool selected = isSelected(bbox, getScaledImageLocation(location));

            if (selected && !something_selected) {
                bbox.is_selected = true;
                selected_bbox = bbox;
                active = bbox;
                something_selected = true;
                emit setCurrentClass(selected_bbox.classname);
            } else {
                bbox.is_selected = false;
            }

            state_change |= (prev_select == bbox.is_selected);
        }

        // Check for selected edge (edge hitboxes)
        auto prev_edge = bbox.selected_edge;
        bbox.selected_edge = getSelectedEdge(bbox, getScaledImageLocation(location));
        if (bbox.selected_edge != EDGE_NONE) {
            active = bbox;
        }
        state_change |= (prev_edge == bbox.selected_edge);

        // Check for selected corner (corner hitboxes)
        auto prev_corner = bbox.selected_corner;
        bbox.selected_corner = getSelectedCorner(bbox, getScaledImageLocation(location));
        if (bbox.selected_corner != CORNER_NONE) {
            active = bbox;
        }
        state_change |= (prev_corner == bbox.selected_corner);
    }

    return active;
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

void ImageLabel::drawBoundingBox(BoundingBox bbox)
{
    auto colour_list = QColor::colorNames();
    QColor colour = QColor(colour_list.at(std::max(0, bbox.classid) % colour_list.size()));
    drawBoundingBox(bbox, colour);
}

QRect ImageLabel::getPaddedRectangle(QPoint location, int pad)
{
    return QRect(location.x() - pad, location.y() - pad, 2 * pad, 2 * pad);
}

void ImageLabel::drawBoundingBox(BoundingBox bbox, QColor colour)
{
    if (scaled_pixmap.isNull())
        return;

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

    if (current_mode == MODE_SELECT) {
        QPen pen(Qt::white, 4);
        painter.setPen(pen);

        painter.drawRect(getPaddedRectangle(scaled_bbox.topLeft(), 5));
        painter.drawRect(getPaddedRectangle(scaled_bbox.topRight(), 5));
        painter.drawRect(getPaddedRectangle(scaled_bbox.bottomLeft(), 5));
        painter.drawRect(getPaddedRectangle(scaled_bbox.bottomRight(), 5));

        if (bbox.selected_edge != EDGE_NONE) {
            QPen pen(Qt::red, 4);
            painter.setPen(pen);

            if (bbox.selected_edge == EDGE_LEFT) {
                painter.drawLine(scaled_bbox.topLeft(), scaled_bbox.bottomLeft());
            } else if (bbox.selected_edge == EDGE_RIGHT) {
                painter.drawLine(scaled_bbox.topRight(), scaled_bbox.bottomRight());
            } else if (bbox.selected_edge == EDGE_BOTTOM) {
                painter.drawLine(scaled_bbox.bottomRight(), scaled_bbox.bottomLeft());
            } else {
                painter.drawLine(scaled_bbox.topLeft(), scaled_bbox.topRight());
            }
        }

        if (bbox.selected_corner != CORNER_NONE) {
            QPen pen(Qt::red, 4);
            painter.setPen(pen);

            if (bbox.selected_corner == CORNER_TOPLEFT) {
                painter.drawRect(getPaddedRectangle(scaled_bbox.topLeft(), 5));
            } else if (bbox.selected_corner == CORNER_TOPRIGHT) {
                painter.drawRect(getPaddedRectangle(scaled_bbox.topRight(), 5));
            } else if (bbox.selected_corner == CORNER_BOTTOMLEFT) {
                painter.drawRect(getPaddedRectangle(scaled_bbox.bottomLeft(), 5));
            } else {
                painter.drawRect(getPaddedRectangle(scaled_bbox.bottomRight(), 5));
            }
        }
    }

    painter.end();
}

void ImageLabel::setBoundingBoxes(QList<BoundingBox> input_bboxes){
    bboxes.clear();
    bboxes = input_bboxes;
    drawLabels();
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

SelectedEdge ImageLabel::getSelectedEdge(BoundingBox bbox, QPoint location)
{
    auto edge = EDGE_NONE;
    auto rect = bbox.rect;

    auto left_hitbox = QRect(QPoint(rect.topLeft().x() - point_threshold,
                                    rect.topLeft().y() + point_threshold),
                             QPoint(rect.bottomLeft().x() + point_threshold,
                                    rect.bottomLeft().y() - point_threshold));

    auto top_hitbox = QRect(QPoint(rect.topLeft().x() + point_threshold,
                                   rect.topLeft().y() - point_threshold),
                            QPoint(rect.topRight().x() - point_threshold,
                                   rect.topRight().y() + point_threshold));

    auto right_hitbox = QRect(QPoint(rect.topRight().x() - point_threshold,
                                     rect.topRight().y() + point_threshold),
                              QPoint(rect.bottomRight().x() + point_threshold,
                                     rect.bottomRight().y() - point_threshold));

    auto bottom_hitbox = QRect(QPoint(rect.bottomLeft().x() + point_threshold,
                                      rect.bottomLeft().y() - point_threshold),
                               QPoint(rect.bottomRight().x() - point_threshold,
                                      rect.bottomRight().y() + point_threshold));

    bool proper = true;
    if (left_hitbox.contains(location, proper)) {
        edge = EDGE_LEFT;
    } else if (right_hitbox.contains(location, proper)) {
        edge = EDGE_RIGHT;
    } else if (top_hitbox.contains(location, proper)) {
        edge = EDGE_TOP;
    } else if (bottom_hitbox.contains(location, proper)) {
        edge = EDGE_BOTTOM;
    }

    return edge;
}

double ImageLabel::pointDistance(QPoint p1, QPoint p2)
{
    auto l = QLine(p1, p2);
    return std::sqrt(std::pow(l.dx(), 2) + std::pow(l.dy(), 2));
}

SelectedCorner ImageLabel::getSelectedCorner(BoundingBox bbox, QPoint location)
{
    auto corner = CORNER_NONE;
    auto rect = bbox.rect;

    auto top_left_dist = pointDistance(rect.topLeft(), location);
    auto top_right_dist = pointDistance(rect.topRight(), location);
    auto bottom_left_dist = pointDistance(rect.bottomLeft(), location);
    auto bottom_right_dist = pointDistance(rect.bottomRight(), location);

    if (top_left_dist < point_threshold) {
        corner = CORNER_TOPLEFT;
    } else if (top_right_dist < point_threshold) {
        corner = CORNER_TOPRIGHT;
    } else if (bottom_left_dist < point_threshold) {
        corner = CORNER_BOTTOMLEFT;
    } else if (bottom_right_dist < point_threshold) {
        corner = CORNER_BOTTOMRIGHT;
    }

    return corner;
}

void ImageLabel::drawLabels(QPoint cursor_location)
{
    scaledPixmap();

    if (scale_factor == 1.0) {
        scaled_pixmap = base_pixmap;
    }

    BoundingBox bbox;
    auto colour_list = QColor::colorNames();

    foreach (bbox, bboxes) {
        if (bbox.label_id == editing_bbox.label_id) {
            drawBoundingBox(editing_bbox, Qt::red);
        } else {
            if (bbox.label_id == selected_bbox.label_id) {
                drawBoundingBox(bbox, Qt::green);
            } else {
                drawBoundingBox(bbox);
            }
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

    drawLabels();
}

void ImageLabel::keyPressEvent(QKeyEvent *event)
{

    if(event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete){
        emit removeLabel(selected_bbox);
    } else if (event->key() == Qt::Key_Escape) {
        rubberBand->setGeometry(QRect(bbox_origin, QSize()));
        bbox_state = WAIT_START;
        selected_bbox = BoundingBox();
        bool state;
        checkBoundingBoxes({-1, -1}, state, true);
        drawLabels();
    } else if (event->key() == Qt::Key_Space && bbox_state == WAIT_START) {
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
        drawLabels();
    }
    } else {
        event->ignore();
    }
}
