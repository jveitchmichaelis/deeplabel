#ifndef GCPEXPORTER_H
#define GCPEXPORTER_H

#include<baseexporter.h>

class GCPExporter : public BaseExporter
{
    Q_OBJECT
public:
    explicit GCPExporter(LabelProject *project, QObject *parent = nullptr) : BaseExporter(project, parent){}

public slots:
    void process();
    void setBucket(QString uri, bool local=false);
    bool setOutputFolder(const QString folder);

private:
    bool processImages(const QString output_folder, QString filename, const QList<QString> images, export_image_type split_type=EXPORT_TRAIN);
    QString bucket_uri;
    QString image_folder;
};

#endif // GCPEXPORTER_H
