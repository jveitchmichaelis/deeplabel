#include "motimporter.h"

void MOTImporter::import(QString sequence_folder, QString annotation_folder, QString names_file){
    QDir seq_dir(sequence_folder);
    loadClasses(names_file);

    // Find all sequences
    QDirIterator it(sequence_folder, QDir::Dirs, QDirIterator::Subdirectories);
    QList<QString> subfolders;

    while (it.hasNext()) {
        auto subfolder = QDir(it.next()).canonicalPath();
        subfolders.append(subfolder);
        qInfo() << "Found: " << subfolder;
    }

    if(subfolders.size() == 0){
        qWarning() << "Couldn't find any sequences in " << sequence_folder;
        return;
    }

    subfolders.removeDuplicates();
    subfolders.sort();

    for(auto &subfolder : subfolders){

        qInfo() << "Checking: " << subfolder;

        auto sequence_name = QFileInfo(subfolder).baseName();

        qInfo() << "Adding images";
        project->addImageFolder(subfolder);

        auto annotation_dir = QDir(annotation_folder);
        QString annotation_file = annotation_dir
                                    .absoluteFilePath(QString("%1.csv")
                                    .arg(sequence_name));
        qDebug() << "Looking for: " << annotation_file;

        if(annotation_dir.exists(annotation_file)){
            // Find images:
            auto image_list = QDir(subfolder).entryList();
            auto labels = getLabels(annotation_file);

            qInfo() << "Adding annotations from: " << annotation_file;

            for(auto & image : image_list){
                qDebug() << "Adding labels for" << image;

                // Extract image ID, ignoring leading zeros
                auto split_file = QFileInfo(image).baseName().split("_");
                int image_id = split_file.back().toInt();

                // Get boxes for this ID and add to DB

                auto boxes = findBoxes(labels, image_id);

                for(auto &box : boxes){
                    bool res = project->addLabel(QDir(subfolder).absoluteFilePath(image), box);
                    if(!res){
                        qWarning() << "Failed to add labels for image: " << image;
                    }
                }

                if(!import_unlabelled && boxes.size() == 0){
                    project->removeImage(image);
                    qDebug() << "Removing unlabelled image: " << image;
                }

            }
        }else{
            qWarning() << "Failed to find annotation file: " << annotation_file;
        }
    }
}

QVector<QStringList> MOTImporter::getLabels(QString annotation_file){
    auto lines = readLines(annotation_file);
    QVector<QStringList> labels;

    for(auto &line : lines){
        auto label = line.simplified().split(",");

        if(label.size() != 10){
            qWarning() << "Label size is incorrect, found :" <<  label.size() << " elements, not 10.";
            continue;
        }else{
            labels.push_back(label);
        }
    }

    return labels;
}

QList<BoundingBox> MOTImporter::findBoxes(QVector<QStringList> labels, int id){

    QList<BoundingBox> boxes = {};

    // <frame>, <id>, <bb_left>, <bb_top>, <bb_width>, <bb_height>, <conf>, <x>, <y>, <z>
    for(auto &label : labels){

        if(label.at(0).toInt() != id){
            continue;
        }

        BoundingBox bbox;

        bbox.classid = label.at(1).toInt() + 1; // Since in the database they're 1-indexed
        bbox.classname = project->getClassName(bbox.classid);

        if(bbox.classname == ""){
            qWarning() << "Class" << bbox.classid << " not found in names file.";
        }

        auto top_left = QPoint(label.at(2).toDouble(), label.at(3).toDouble());
        auto bottom_right = top_left + QPoint(label.at(4).toDouble(), label.at(5).toDouble());

        bbox.rect = QRect(top_left, bottom_right);
        boxes.append(bbox);
    }

    return boxes;
}


void MOTImporter::loadClasses(QString names_file){
    QFile fh(names_file);

    if (fh.open(QIODevice::ReadOnly)) {

        while (!fh.atEnd()) {
            // Darknet name file is just a newline delimited list of classes
            QByteArray line = fh.readLine();

            if(QString(line) == "") continue;

            project->addClass(line.simplified());
            qInfo() << line.simplified();
        }
    }


}
