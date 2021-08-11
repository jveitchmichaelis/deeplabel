#include "multitracker.h"

cv::Rect2d qrect2cv(QRect rect){
    return cv::Rect2d(rect.x(), rect.y(), rect.width(),rect.height());
}


/* ---- OpenCV Trackers ---- */

cv::Ptr<cv::Tracker> MultiTrackerCV::createTrackerByName(OpenCVTrackerType type){
    using namespace cv;

    Ptr<Tracker> tracker;
    /*
    if (type == BOOSTING)
      tracker = TrackerBoosting::create();
    else if (type == MIL)
      tracker = TrackerMIL::create();
    else if (type == KCF)
      tracker = TrackerKCF::create();
    else if (type == TLD)
      tracker = TrackerTLD::create();
    else if (type == MEDIANFLOW)
      tracker = TrackerMedianFlow::create();
    else if (type == GOTURN)
      tracker = TrackerGOTURN::create();
    else if (type == MOSSE)
      tracker = TrackerMOSSE::create();
    else if (type == CSRT)
      tracker = TrackerCSRT::create();
    else {
      std::cerr << "Incorrect tracker specified";
    }*/

    tracker = TrackerCSRT::create();

    return tracker;
}

void MultiTrackerCV::init(const cv::Mat &image, QList<BoundingBox> bboxes){

    trackers.clear();
    QMutex mutex;

    // If we are tracking and we have some labelled boxes already
    QtConcurrent::blockingMap(bboxes.begin(), bboxes.end(), [&](BoundingBox &bbox)
    {
        if(bbox.rect.width()*bbox.rect.height() <= 0) return;

        auto tracker = createTrackerByName(type_);
        tracker->init(image, qrect2cv(bbox.rect));

        mutex.lock();
        trackers.push_back({tracker, bbox.classname});
        mutex.unlock();
    });
}

void MultiTrackerCV::update(const cv::Mat &image){
    QMutex mutex;
    bboxes.clear();

    QtConcurrent::blockingMap(trackers.begin(), trackers.end(), [&](auto&& tracker)
    {
        cv::Rect2d bbox;

        if( tracker.first->update(image, bbox)){

            QRect new_roi;
            new_roi.setX(static_cast<int>(bbox.x));
            new_roi.setY(static_cast<int>(bbox.y));
            new_roi.setWidth(static_cast<int>(bbox.width));
            new_roi.setHeight(static_cast<int>(bbox.height));

            BoundingBox new_bbox;
            new_bbox.rect = new_roi;
            new_bbox.classname = tracker.second;

            mutex.lock();
            bboxes.append(new_bbox);
            mutex.unlock();
        }
    });
}

/* ---- CamShift Tracker ---- */

void MultiTrackerCamshift::init(const cv::Mat &image, QList<BoundingBox> bboxes){

    trackers.clear();
    QMutex mutex;

    QtConcurrent::blockingMap(bboxes.begin(), bboxes.end(), [&](BoundingBox &bbox)
    {
        auto roi_hist = getRoiHist(image, bbox.rect);
        mutex.lock();
        trackers.push_back({roi_hist, bbox});
        mutex.unlock();
    });
}


void MultiTrackerCamshift::update(const cv::Mat &image){

    QMutex mutex;
    bboxes.clear();

    QtConcurrent::blockingMap(trackers.begin(), trackers.end(), [&](auto&& tracker)
    {
        // image, roi_hist, bbox
        cv::Rect2d bbox = updateCamshift(image, tracker.first, tracker.second.rect);

        QRect new_roi;
        new_roi.setX(static_cast<int>(bbox.x));
        new_roi.setY(static_cast<int>(bbox.y));
        new_roi.setWidth(static_cast<int>(bbox.width));
        new_roi.setHeight(static_cast<int>(bbox.height));

        BoundingBox new_bbox;
        new_bbox.rect = new_roi;
        new_bbox.classname = tracker.second.classname;

        mutex.lock();
        bboxes.append(new_bbox);
        mutex.unlock();


    });

}


cv::Mat MultiTrackerCamshift::getRoiHist(const cv::Mat &image, QRect bbox){
    auto roi = image(qrect2cv(bbox));
    cv::Mat mask;

    if(roi.channels() == 1){
        //cv::cvtColor(roi, roi, cv::COLOR_GRAY2BGR);
        //cv::cvtColor(roi, roi, cv::COLOR_BGR2HSV);
        auto lowScalar = cv::Scalar(30);
        auto highScalar = cv::Scalar(180);
        cv::inRange(roi, lowScalar, highScalar, mask);
    }else if(roi.channels() == 3){
        cv::cvtColor(roi, roi, cv::COLOR_BGR2HSV);
        auto lowScalar = cv::Scalar(30,30,30);
        auto highScalar = cv::Scalar(180,180,180);
        cv::inRange(roi, lowScalar, highScalar, mask);
    }else{
        // What the hell kind of 2 channel image is this?
        return cv::Mat();
    }

    // Generate histogram
    cv::Mat roiHist;

    int histSize = 256;
    float range[] = { 0, 180 }; //the upper boundary is exclusive
    const float* histRange = { range };

    bool uniform = true;
    bool accumulate = true;

    int n_images = 1;
    int *use_channels = nullptr;
    int n_dims = 1;

    cv::calcHist( &roi, n_images, use_channels, mask, roiHist, n_dims, &histSize, &histRange, uniform, accumulate);
    cv::normalize(roiHist, roiHist, 0, 255, cv::NORM_MINMAX);

    return roiHist;

}

cv::Rect2i MultiTrackerCamshift::updateCamshift(const cv::Mat &image, cv::Mat roiHist, QRect bbox){
    auto rect = qrect2cv(bbox);
    auto roi = image(rect);

    if(roi.channels() == 1){
        //cv::cvtColor(roi, roi, cv::COLOR_GRAY2BGR);
        cv::cvtColor(roi, roi, cv::COLOR_BGR2HSV);
    }else if(roi.channels() == 3){
        cv::cvtColor(roi, roi, cv::COLOR_BGR2HSV);
    }else{
        // What the hell kind of 2 channel image is this?
        return cv::Rect();
    }

    auto rectInt = cv::Rect2i(rect);

    cv::Mat backProjection;
    float range[] = { 0, 180 }; //the upper boundary is exclusive
    const float* histRange = { range };
    cv::calcBackProject(&image, 1, nullptr, roiHist, backProjection, &histRange);

    auto termcrit = cv::TermCriteria(cv::TermCriteria::EPS|cv::TermCriteria::COUNT, 10, 1);
    auto rotated_rect = cv::CamShift(backProjection, rectInt, termcrit);

    return rotated_rect.boundingRect();
}

void MultiTrackerCamshift::histogram(const cv::Mat &image, cv::Mat &hist){
    int histSize = 256;
    float range[] = { 0, 256 }; //the upper boundary is exclusive
    const float* histRange = { range };

    bool uniform = true;
    bool accumulate = true;

    calcHist( &image, 1, nullptr, cv::Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);
}

