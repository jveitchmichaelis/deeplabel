#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QFileDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QSettings>
#include <QTemporaryDir>
#include <QScrollArea>
#include <QScreen>
#include <QtConcurrent>
#include <QProgressDialog>

#include <opencv2/opencv.hpp>
#include <imagelabel.h>
#include <labelproject.h>

#include <kittiexporter.h>
#include <darknetexporter.h>
#include <pascalvocexporter.h>
#include <cocoexporter.h>

#include <detection/detectoropencv.h>
#include <detection/detectorsetupdialog.h>

#include <algorithm>
#include <exportdialog.h>
#include <multitracker.h>
#include <imagedisplay.h>

#include <QtAwesome.h>

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
    ImageLabel *currentImage;
    ExportDialog *export_dialog;
    MultiTracker *multitracker;
    QScrollArea *imageScrollArea;
    ImageDisplay *display;
    DetectorOpenCV detector;

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

    QSettings* settings;

private slots:

    void updateDisplay(void);

    void openProject(QString filename = "");
    void newProject(void);

    void addClass(void);
    void setCurrentClass(QString);
    void removeClass(void);

    void addVideo(void);
    void addImages(void);
    void addImageFolder(void);
    void addImageFolders();
    void nextImage(void);
    void previousImage(void);
    void removeImage(void);

    void addLabel(BoundingBox bbox);
    void removeLabel(BoundingBox bbox);
    void updateLabel(BoundingBox old_bbox, BoundingBox new_bbox);
    void removeImageLabels(void);

    void setDrawMode(void);
    void setSelectMode(void);

    void enableWrap(bool enable);
    void launchExportDialog();
    void handleExportDialog();

    // Tracking
    void initTrackers();
    void updateTrackers();
    void toggleAutoPropagate(bool state);
    void toggleRefineTracking(bool state);
    void nextUnlabelled();
    void nextInstance();

    QRect refineBoundingBox(cv::Mat image, QRect bbox, int margin=5, bool debug_save=false);
    QRect refineBoundingBoxSimple(cv::Mat image, QRect bbox, int margin=5, bool debug_save=false);
    void refineBoxes();

    void updateImageInfo();
    void jumpForward(int n = 10);
    void jumpBackward(int n = 10);

    void detectObjects(cv::Mat &image, QString image_path);
    void detectCurrentImage();
    void detectProject();
    void setupDetector();
    void setConfidenceThreshold();
    void setNMSThreshold();


    void updateCurrentIndex(int index);
signals:
    void selectedClass(QString);

};

#endif // MAINWINDOW_H
