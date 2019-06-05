#ifndef DARKNETEXPORTER_H
#define DARKNETEXPORTER_H

#include <QObject>
#include <QImageReader>
#include <QFileDialog>
#include <boundingbox.h>
#include <labelproject.h>
#include <opencv2/opencv.hpp>
#include <random>

class DarknetExporter : public QObject
{
    Q_OBJECT
public:
    explicit DarknetExporter(LabelProject *project, QObject *parent = nullptr);

signals:
    void export_progress(int);

public slots:
    void splitData(float split=1, bool shuffle=false, int seed=42);
    bool setOutputFolder(QString folder);
    void setExportUnlabelled(bool res){export_unlabelled = res;}
    void generateLabelIds(const QString names_file);
    void process();


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

    bool export_unlabelled = false;

    std::map<QString, int> id_map;

    void writeLabels(const cv::Mat &image, const QString file, const QList<BoundingBox> labels);
    bool processImages(const QString folder, const QList<QString> images);
    bool saveImage(cv::Mat &image, const QString output, const double scale_x = -1.0, const double scale_y = -1.0);
};

#endif // DARKNETEXPORTER_H
