#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QFileDialog>
#include <QDialog>
#include <QTemporaryDir>
#include <QtConcurrent/qtconcurrentmap.h>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <imagelabel.h>
#include <labelproject.h>
#include <kittiexporter.h>
#include <darknetexporter.h>
#include <concurrent_vector.h>
#include <algorithm>
#include <exportdialog.h>

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
    Concurrency::concurrent_vector<std::pair<cv::Ptr<cv::Tracker>, QString>> trackers;
    Concurrency::concurrent_vector<std::pair<cv::Mat, BoundingBox>> trackers_camshift;
    ExportDialog export_dialog;

    enum trackerType {BOOSTING, MIL, KCF, TLD, MEDIANFLOW, GOTURN, MOSSE, CSRT};

    // Enable tracking boxes in previous frames
    bool track_previous = false;
    bool wrap_index;
    unsigned int current_index;

    void initDisplay();

    QList<QString> images;
    QList<QString> classes;
    QString current_imagepath;
    QString current_class;

    unsigned int number_images;
    void updateImageList();
    void updateClassList();
    void updateLabels();

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
    void removeImage(void);
    void setDrawMode(void);
    void setSelectMode(void);
    void changeImage(void);
    void enableWrap(bool enable);
    void launchExportDialog();
    void handleExportDialog();
    void histogram(const cv::Mat &image, cv::Mat &hist);
    QImage convert16(const cv::Mat &source);


    // Tracking
    void propagateTracking();
    cv::Ptr<cv::Tracker> createTrackerByName(trackerType type);
    cv::Rect2d qrect2cv(QRect rect);
    void initTrackers();
    void toggleAutoPropagate(bool state);
    void nextUnlabelled();
    QRect refineBoundingBox(cv::Mat image, QRect bbox);
    cv::Rect2i updateCamShift(cv::Mat image, cv::Mat roiHist, QRect bbox);
    cv::Mat initCamShift(cv::Mat image, QRect bbox);
    void initTrackersCamShift();
    void propagateTrackingCamShift();

signals:
    void selectedClass(QString);

};

#endif // MAINWINDOW_H
