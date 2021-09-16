#include "birdsaiimporter.h"

void BirdsAIImporter::import(QString sequence_folder, QString annotation_folder){
    QDir seq_dir(sequence_folder);

    // Find all sequence folders
    QDirIterator it(sequence_folder, QDir::Dirs);
    QList<QString> subfolders;

    while (it.hasNext()) {
        auto subfolder = QDir(it.next()).canonicalPath();
        subfolders.append(subfolder);
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
            auto image_list = QDir(subfolder).entryList(QDir::Files);
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

QList<BoundingBox> BirdsAIImporter::findBoxes(QVector<QStringList> labels, int id){

    QList<BoundingBox> boxes = {};

    // <frame>, <id>, <bb_left>, <bb_top>, <bb_width>, <bb_height>, <class>, <species>, etc
    for(auto &label : labels){

        if(label.at(0).toInt() != id){
            continue;
        }

        BoundingBox bbox;

        /* https://sites.google.com/view/elizabethbondi/dataset
         * -1: unknown,
         * 0: human,
         * 1: elephant,
         * 2: lion,
         * 3: giraffe,
         * 4: dog,
         * 5: crocodile,
         * 6: hippo,
         * 7: zebra,
         * 8: rhino
         */
        int label_species = label.at(7).toInt();
        bbox.classid = label_species + 2; // Since in the database they're 1-indexed

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


