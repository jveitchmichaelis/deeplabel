#ifndef COCOIMPORTER_H
#define COCOIMPORTER_H

#include<baseimporter.h>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

class CocoImporter : public BaseImporter
{
public:
    using BaseImporter::import;

    explicit CocoImporter(LabelProject *project, QObject *parent = nullptr) : BaseImporter(parent){
        this->project = project;
    }

    void import(QString annotations_file, QString image_folder);
};

#endif // COCOIMPORTER_H
