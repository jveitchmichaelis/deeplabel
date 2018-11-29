#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QFileDialog>
#include <QTemporaryDir>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <imagelabel.h>
#include <labelproject.h>
#include <kittiexporter.h>

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
    std::map<std::string, cv::Ptr<cv::MultiTracker>> tracker_map;

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
    void exportData();
    void propagateTracking();
    void histogram(const cv::Mat &image, cv::Mat &hist);
    QImage convert16(const cv::Mat &source);
    cv::Ptr<cv::Tracker> createTrackerByName(trackerType type);
    cv::Rect2d qrect2cv(QRect rect);
    void setupTracking();
    void toggleAutoPropagate(bool state);
    void nextUnlabelled();
    QRect refineBoundingBox(cv::Mat image, QRect bbox);

signals:
    void selectedClass(QString);

};

#endif // MAINWINDOW_H
