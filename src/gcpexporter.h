#ifndef GCPEXPORTER_H
#define GCPEXPORTER_H

#include<baseexporter.h>

enum gcp_image_type{
    GCP_UNASSIGNED,
    GCP_TRAIN,
    GCP_VAL,
    GCP_TEST
};

class GCPExporter : public BaseExporter
{
    Q_OBJECT
public:
    explicit GCPExporter(LabelProject *project, QObject *parent = nullptr) : BaseExporter(project, parent){}

public slots:
    void process();
    void setBucket(QString uri);
    bool setOutputFolder(const QString folder);

private:
    bool processImages(const QString output_folder, QString filename, const QList<QString> images, gcp_image_type split_type);
    QString bucket_uri;
    QString image_folder;
};

#endif // GCPEXPORTER_H
