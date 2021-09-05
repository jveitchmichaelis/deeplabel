#include "birdsaiimporter.h"

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


