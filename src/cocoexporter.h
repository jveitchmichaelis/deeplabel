#ifndef COCOEXPORTER_H
#define COCOEXPORTER_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <labelproject.h>
#include <QImageReader>
#include <QFileDialog>
#include <boundingbox.h>
#include <opencv2/opencv.hpp>
#include <random>

class CocoExporter : public QObject
{
    Q_OBJECT
public:
    explicit CocoExporter(LabelProject *project, QObject *parent = nullptr);

signals:
    void export_progress(int);

public slots:
    void splitData(float split=1, bool shuffle=false, int seed=42);
    bool setOutputFolder(QString folder);
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

    std::map<QString, int> id_map;

    int image_id;
    int label_id;

    bool processImages(const QString folder, const QString filename, const QList<QString> images);
    bool saveImage(cv::Mat &image, const QString output, const double scale_x = -1.0, const double scale_y = -1.0);
};

#endif // COCOEXPORTER_H
