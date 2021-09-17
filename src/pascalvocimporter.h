#ifndef PASCALVOCIMPORTER_H
#define PASCALVOCIMPORTER_H

#include "baseimporter.h"
#include <QDomDocument>

class PascalVOCImporter : public BaseImporter
{
public:
    using BaseImporter::import;

    explicit PascalVOCImporter(LabelProject *project, QObject *parent = nullptr) : BaseImporter(parent){
        this->project = project;
    }
    void import(QString image_folder, QString annotation_folder);

private:
    QList<BoundingBox> getLabels(QString annotation_file, QString &image_file);
};

#endif // PASCALVOCIMPORTER_H
