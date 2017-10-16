#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <imagelabel.h>
#include <labelproject.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    LabelProject *project;
    QPixmap pixmap;
    ImageLabel *currentImage;

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

private slots:
    void openProject(void);
    void newProject(void);
    void addClass(void);
    void addImages(void);
    void addImageFolder(void);
    void nextImage(void);
    void previousImage(void);
    void display(QString fileName);
    void addLabel(BoundingBox bbox);
    void removeLabel(BoundingBox bbox);
    void setDrawMode(void);
    void setSelectMode(void);

signals:
    void selectedClass(QString);

};

#endif // MAINWINDOW_H
