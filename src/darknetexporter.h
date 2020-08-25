#ifndef DARKNETEXPORTER_H
#define DARKNETEXPORTER_H

#include <baseexporter.h>
#include <algorithm>

class DarknetExporter : public BaseExporter
{
    Q_OBJECT
public:
    explicit DarknetExporter(LabelProject *project, QObject *parent = nullptr) : BaseExporter(project, parent){}

signals:
    void export_progress(int);

public slots:
    void generateLabelIds(const QString names_file);
    void process();

protected:
    void writeLabels(const cv::Mat &image, const QString file, const QList<BoundingBox> labels);
    bool processImages(const QString folder, const QList<QString> images);
};

#endif // DARKNETEXPORTER_H
