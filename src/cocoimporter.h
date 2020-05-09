#ifndef COCOIMPORTER_H
#define COCOIMPORTER_H

#include<baseimporter.h>

class CocoImporter : public BaseImporter
{
public:
    using BaseImporter::import;

    explicit CocoImporter(LabelProject *project, QObject *parent = nullptr) : BaseImporter(parent){
        this->project = project;
    }

    void import(QString annotations_file);
};

#endif // COCOIMPORTER_H
