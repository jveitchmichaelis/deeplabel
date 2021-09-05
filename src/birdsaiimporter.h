#ifndef BIRDSAIIMPORTER_H
#define BIRDSAIIMPORTER_H

#include "motimporter.h"

class BirdsAIImporter : public MOTImporter
{
public:
    explicit BirdsAIImporter(LabelProject *project, QObject *parent = nullptr) : MOTImporter(project, parent){
        this->project = project;
    }
private:
    QList<BoundingBox> findBoxes(QVector<QStringList> labels, int id);
};

#endif // BIRDSAIIMPORTER_H
