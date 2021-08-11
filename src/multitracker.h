#ifndef TRACKER_H
#define TRACKER_H

#include <QObject>
#include <QtConcurrent/qtconcurrentmap.h>
#include <boundingbox.h>

#include <opencv2/opencv.hpp>

#if (CV_VERSION_MAJOR >= 4) && (CV_VERSION_MINOR >= 5) && (CV_VERSION_SUBMINOR >= 3)
#include <opencv2/tracking/tracking_legacy.hpp>
#else
#include <opencv2/tracking.hpp>
#endif

enum OpenCVTrackerType {BOOSTING, MIL, KCF, TLD, MEDIANFLOW, GOTURN, MOSSE, CSRT};
enum MultiTrackerType {OPENCV, CAMSHIFT};

cv::Rect2d qrect2cv(QRect rect);

class MultiTracker : public QObject
{
    Q_OBJECT
public:
    explicit MultiTracker(QObject *parent = nullptr) : QObject(parent){}

signals:

public slots:
    QList<BoundingBox> getBoxes(){ return bboxes;}

    virtual void init(const cv::Mat &image, QList<BoundingBox> bboxes) = 0;
    virtual void update(const cv::Mat &image) = 0;

protected:
    QList<BoundingBox> bboxes;
};

class MultiTrackerCV : public MultiTracker
{
    Q_OBJECT

public:
    explicit MultiTrackerCV(QObject *parent = nullptr) : MultiTracker(parent){}

public slots:
    void init(const cv::Mat &image, QList<BoundingBox> bboxes);
    void update(const cv::Mat &image);
    void setTrackerType(OpenCVTrackerType type){ type_ = type; }

private:
    std::vector<std::pair<cv::Ptr<cv::Tracker>, QString>> trackers;
    cv::Ptr<cv::Tracker> createTrackerByName(OpenCVTrackerType MultiTracker);
    OpenCVTrackerType type_;

};

class MultiTrackerCamshift : public MultiTracker
{
    Q_OBJECT
public:
    explicit MultiTrackerCamshift(QObject *parent = nullptr)  : MultiTracker(parent){}
    void init(const cv::Mat &image, QList<BoundingBox> bboxes);
    void update(const cv::Mat &image);


private:
    std::vector<std::pair<cv::Mat, BoundingBox>> trackers;
    cv::Rect2i updateCamshift(const cv::Mat &image, cv::Mat roiHist, QRect bbox);
    cv::Mat getRoiHist(const cv::Mat &image, QRect bbox);
    void histogram(const cv::Mat &image, cv::Mat &hist);

};

#endif // TRACKER_H
