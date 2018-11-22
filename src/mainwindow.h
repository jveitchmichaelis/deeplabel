#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QFileDialog>
#include <QTemporaryDir>
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
    void display();
    void addLabel(BoundingBox bbox);
    void removeLabel(BoundingBox bbox);
    void removeClass(void);
    void removeImage(void);
    void setDrawMode(void);
    void setSelectMode(void);
    void changeImage(void);
    void enableWrap(bool enable);
    void exportData();
    void histogram(const cv::Mat &image, cv::Mat &hist);
    QImage convert16(const cv::Mat &source);

signals:
    void selectedClass(QString);

};

#endif // MAINWINDOW_H
