#include "pascalvocimporter.h"

void PascalVOCImporter::import(QString image_folder, QString annotation_folder){

    QDir image_dir(image_folder);
    QDir annotation_dir(annotation_folder);

    QList<QList<BoundingBox>> label_list;
    QList<QString> image_list;

    auto annotations = QDir(annotation_folder).entryList(QDir::Files|QDir::NoDotAndDotDot);

    auto pbar = cliProgressBar();
    double progress = 0;
    int i = 0;

    for(auto & annotation : annotations){

        qDebug() << annotation;

        auto abs_annotation_path = annotation_dir.absoluteFilePath(annotation);
        QString image_filename;
        auto labels = getLabels(abs_annotation_path, image_filename);
        auto abs_image_path = image_dir.absoluteFilePath(image_filename);

        progress = 100*static_cast<double>(i++)/annotations.size();
        pbar.update(progress);
        pbar.print();

        if(!QFileInfo(abs_image_path).exists()){
            qWarning() << "Couldn't find image: " << abs_image_path;
            continue;
        }

        if(labels.empty() && !import_unlabelled){
            continue;
        }

        image_list.append(abs_image_path);
        label_list.append(labels);
    }

    qInfo() << "Importing images and labels";
    project->addLabelledAssets(image_list, label_list);

}

QList<BoundingBox> PascalVOCImporter::getLabels(QString annotation_file, QString &image_filename){
    QFile f(annotation_file);
    QDomDocument doc;
    QList<BoundingBox> labels;
    QList<QString> classes;
    project->getClassList(classes);

    if (!f.open(QIODevice::ReadOnly))
        return labels;
    if (!doc.setContent(&f)) {
        f.close();
        return labels;
    }

    auto objects = doc.elementsByTagName("object");

    image_filename = doc.elementsByTagName("filename").at(0).toElement().text();

    for(int i=0; i < objects.size(); i++){

        BoundingBox new_box;
        auto object = objects.at(i).toElement();
        auto name_node = object.elementsByTagName("name").at(0).toElement().text();

        new_box.classname = object.elementsByTagName("name").at(0).toElement().text();

        if(!classes.contains(new_box.classname)){
            project->addClass(new_box.classname);
            classes.clear();
            project->getClassList(classes);
        }

        new_box.classid = project->getClassId(new_box.classname);

        auto xmin = object.elementsByTagName("xmin").at(0).toElement().text().toInt();
        auto ymin = object.elementsByTagName("ymin").at(0).toElement().text().toInt();
        auto xmax = object.elementsByTagName("xmax").at(0).toElement().text().toInt();
        auto ymax = object.elementsByTagName("ymax").at(0).toElement().text().toInt();

        new_box.rect = QRect(QPoint(xmin, ymin), QPoint(xmax, ymax));

        labels.append(new_box);
    }

    return labels;
}

