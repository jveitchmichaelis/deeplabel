#include "baseimporter.h"

BaseImporter::BaseImporter(QObject *parent) : QObject(parent)
{

}

void BaseImporter::setImportUnlabelled(bool import){
    import_unlabelled = import;
}

void BaseImporter::setRootFolder(QString folder){
    label_root = folder;
}

void BaseImporter::addAsset(QString image_path, QList<BoundingBox> boxes){
    if(boxes.size() == 0 && !import_unlabelled)
        return;

    if(project->addAsset(image_path)){

        for(auto &box : boxes){
            project->addLabel(image_path, box);
        }
    }
}

QList<QString> BaseImporter::readLines(QString path){
    QFile file(path);
    QList<QString> lines = {};

    if (file.open(QFile::ReadOnly)) {
        while (!file.atEnd()){
            auto line = QString(file.readLine()).simplified();
            lines.append(line);
        }
    }

    return lines;
}
