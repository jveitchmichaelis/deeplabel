#ifndef DARKNETIMPORTER_H
#define DARKNETIMPORTER_H

#include<baseimporter.h>

class DarknetImporter : public BaseImporter
{
public:
    using BaseImporter::import;

    explicit DarknetImporter(LabelProject *project, QObject *parent = nullptr) : BaseImporter(parent){
        this->project = project;
    }
    void import(QString image_list, QString names_file);
private:
    void loadClasses(QString names_file);
    QList<BoundingBox> loadLabels(QString image);
};

#endif // DARKNETIMPORTER_H
