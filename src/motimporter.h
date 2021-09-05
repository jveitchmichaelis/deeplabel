#ifndef MOTIMPORTER_H
#define MOTIMPORTER_H

#include "baseimporter.h"

class MOTImporter : public BaseImporter
{
public:
    using BaseImporter::import;

    explicit MOTImporter(LabelProject *project, QObject *parent = nullptr) : BaseImporter(parent){
        this->project = project;
    }
    void import(QString sequence_folder, QString annotation_folder, QString names_file);
private:
    virtual QList<BoundingBox> findBoxes(QVector<QStringList> labels, int id);
    QString findImage(QList<QString> *images, QString sequence_name, int image_id);
    QVector<QStringList> getLabels(QString annotation_file);
    void loadClasses(QString names_file);
};

#endif // MOTIMPORTER_H
