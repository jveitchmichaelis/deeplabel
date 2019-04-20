#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QFileDialog>
#include <QDialog>
#include <QSettings>
#include <QTemporaryDir>
#include <QtConcurrent/qtconcurrentmap.h>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <imagelabel.h>
#include <labelproject.h>
#include <kittiexporter.h>
#include <darknetexporter.h>
#include <algorithm>
#include <exportdialog.h>

#include <detection/detectoropencv.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    LabelProject *project;
    QPixmap pixmap;
    ImageLabel *currentImage;
    cv::Mat display_image;
    std::vector<std::pair<cv::Ptr<cv::Tracker>, QString>> trackers;
    std::vector<std::pair<cv::Mat, BoundingBox>> trackers_camshift;
    ExportDialog export_dialog;

    enum trackerType {BOOSTING, MIL, KCF, TLD, MEDIANFLOW, GOTURN, MOSSE, CSRT};

    // Enable tracking boxes in previous frames
    bool track_previous = false;
    bool refine_on_propagate = false;
    bool wrap_index;
    int current_index;

    void initDisplay();

    QList<QString> images;
    QList<QString> classes;
    QString current_imagepath;
    QString current_class;

    int number_images;
    void updateImageList();
    void updateClassList();
    void updateLabels();

    QList<BoundingBox> detected_objects;

    QSettings* settings;
    DetectorOpenCV detector;

private slots:
    void openProject(void);
    void newProject(void);
    void addClass(void);
    void addImages(void);
    void addImageFolder(void);
    void nextImage(void);
    void previousImage(void);
    void updateDisplay(void);
    void addLabel(BoundingBox bbox);
    void removeLabel(BoundingBox bbox);
    void removeClass(void);
    void removeLabelsFromImage();
    void removeImage(void);
    void setDrawMode(void);
    void setSelectMode(void);
    void changeImage(void);
    void enableWrap(bool enable);
    void launchExportDialog();
    void handleExportDialog();
    void histogram(const cv::Mat &image, cv::Mat &hist);
    QImage convert16(const cv::Mat &source);
    void detectObjects();


    // Tracking
    void propagateTracking();
    cv::Ptr<cv::Tracker> createTrackerByName(trackerType type);
    cv::Rect2d qrect2cv(QRect rect);
    void initTrackers();
    void toggleAutoPropagate(bool state);
    void toggleRefineTracking(bool state);
    void nextUnlabelled();
    QRect refineBoundingBox(cv::Mat image, QRect bbox, int margin=5, bool debug_save=false);
    QRect refineBoundingBoxSimple(cv::Mat image, QRect bbox, int margin=5, bool debug_save=false);
    cv::Rect2i updateCamShift(cv::Mat image, cv::Mat roiHist, QRect bbox);
    cv::Mat initCamShift(cv::Mat image, QRect bbox);
    void initTrackersCamShift();
    void propagateTrackingCamShift();
    void updateLabel(BoundingBox old_bbox, BoundingBox new_bbox);
    void refineBoxes();
    void initDetector();

signals:
    void selectedClass(QString);

};

#endif // MAINWINDOW_H
