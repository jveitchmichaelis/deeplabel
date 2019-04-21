#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QFileDialog>
#include <QDialog>
#include <QSettings>
#include <QTemporaryDir>
#include <opencv2/opencv.hpp>
#include <imagelabel.h>
#include <labelproject.h>
#include <kittiexporter.h>
#include <darknetexporter.h>
#include <algorithm>
#include <exportdialog.h>
#include <multitracker.h>

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
    ExportDialog export_dialog;
    MultiTracker *multitracker;

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

    void updateDisplay(void);

    void openProject(void);
    void newProject(void);

    void addClass(void);
    void removeClass(void);

    void addImages(void);
    void addImageFolder(void);
    void nextImage(void);
    void previousImage(void);
    void removeImage(void);
    void removeImageLabels(void);
    void changeImage(void);

    void addLabel(BoundingBox bbox);
    void removeLabel(BoundingBox bbox);
    void removeLabelsFromImage();
    void updateLabel(BoundingBox old_bbox, BoundingBox new_bbox);
    void setDrawMode(void);
    void setSelectMode(void);

    void enableWrap(bool enable);
    void launchExportDialog();
    void handleExportDialog();

    QImage convert16(const cv::Mat &source);
    void detectObjects();

    // Tracking
    void initTrackers();
    void updateTrackers();
    void toggleAutoPropagate(bool state);
    void toggleRefineTracking(bool state);
    void nextUnlabelled();
    QRect refineBoundingBox(cv::Mat image, QRect bbox, int margin=5, bool debug_save=false);
    QRect refineBoundingBoxSimple(cv::Mat image, QRect bbox, int margin=5, bool debug_save=false);
    void refineBoxes();
    void initDetector();


signals:
    void selectedClass(QString);

};

#endif // MAINWINDOW_H
