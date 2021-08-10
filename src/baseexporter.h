#ifndef BASEEXPORTER_H
#define BASEEXPORTER_H

#include <QObject>
#include <QDateTime>
#include <QImageReader>
#include <QFileDialog>

#include <opencv2/opencv.hpp>
#include <random>

#include <labelproject.h>
#include <boundingbox.h>

enum export_image_type{
    EXPORT_UNASSIGNED,
    EXPORT_TRAIN,
    EXPORT_VAL,
    EXPORT_TEST
};

class BaseExporter : public QObject
{
    Q_OBJECT
public:
    explicit BaseExporter(LabelProject *project, QObject *parent = nullptr);

signals:
    void export_progress(int);

public slots:
    void splitData(float split=1, bool shuffle=false, int seed=42);
    bool setOutputFolder(QString folder);
    void setExportUnlabelled(bool res){export_unlabelled = res;}
    void setAppendLabels(bool res){append_labels = res;}
    void setFilenamePrefix(QString prefix);
    virtual void process() = 0;

protected:
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
    QString filename_prefix = "";

    std::map<QString, int> id_map;

    int image_id;
    int label_id;

    bool append_labels = false;
    bool validation_split = false;
    bool export_unlabelled = false;
    bool saveImage(cv::Mat &image, const QString output, const double scale_x = -1.0, const double scale_y = -1.0);
};

#endif // BASEEXPORTER_H
