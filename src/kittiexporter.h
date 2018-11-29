#ifndef KITTIEXPORTER_H
#define KITTIEXPORTER_H

#include <QObject>
#include <QImageReader>
#include <QFileDialog>
#include <boundingbox.h>
#include <labelproject.h>
#include <opencv2/opencv.hpp>
#include <random>

class KittiExporter : public QObject
{
    Q_OBJECT
public:
    explicit KittiExporter(LabelProject *project, QObject *parent = nullptr);

signals:

private:
    LabelProject *project;
    QList<QString> train_set;
    QList<QString> validation_set;
    QList<QString> images;

    QString train_folder;
    QString train_label_folder;
    QString train_image_folder;

    QString val_folder;
    QString val_label_folder;
    QString val_image_folder;

    QString output_folder;

public slots:
    void splitData(float split=0.9, bool shuffle=true);
    bool setOutputFolder(QString folder);
    void process();

private:
    void appendLabel(QString file, QList<BoundingBox> labels, double scale_x=1, double scale_y=1);
    int processSet(QString folder, QList<QString> images, int i);
    bool saveImage(QString input, QString output, double &scale_x, double &scale_y);
};

#endif // KITTIEXPORTER_H
