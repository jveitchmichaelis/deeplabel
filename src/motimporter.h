#ifndef MOTIMPORTER_H
#define MOTIMPORTER_H

#include "baseimporter.h"
#include <QSettings>
#include <QTextCodec>

class MOTImporter : public BaseImporter
{
public:
    using BaseImporter::import;

    explicit MOTImporter(LabelProject *project, QObject *parent = nullptr) : BaseImporter(parent){
        this->project = project;
    }
    void import(QString sequence_folder);
    void importSequence(QString folder);
    void loadClasses(QString names_file);

protected:
    QVector<QStringList> getLabels(QString annotation_file);
    virtual QList<BoundingBox> findBoxes(QVector<QStringList> labels, int id);
    QString findImage(QList<QString> *images, QString sequence_name, int image_id);


};

#endif // MOTIMPORTER_H
