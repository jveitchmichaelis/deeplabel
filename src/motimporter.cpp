#include "motimporter.h"

void MOTImporter::importSequence(QString folder){

    auto ini_filename = QDir(folder).absoluteFilePath("seqinfo.ini");
    auto sequence_name = QFileInfo(folder).baseName();

    // Load image folder path from ini file
    QSettings seq_ini(ini_filename, QSettings::IniFormat);
    seq_ini.setIniCodec(QTextCodec::codecForName("UTF-8"));
    seq_ini.beginGroup("Sequence");

    if(!seq_ini.contains("imDir")){
        qCritical() << ini_filename << " doesn't contain image directory";
        return;
    }

    auto image_foldername = seq_ini.value("imDir").toString();

    if(image_foldername == ""){
        qCritical() << "Image folder value in INI file is empty";
        return;
    }

    qDebug() << "Image folder:" << image_foldername;
    auto image_folder = QDir(folder).absoluteFilePath(image_foldername);
    qDebug() << "Adding images from " << image_folder;

    QList<QList<BoundingBox>> label_list;
    QList<QString> image_list;

    // Find and load gt.txt file:
    auto annotation_dir = QDir(QDir(folder).absoluteFilePath("gt"));

    QString annotation_file = annotation_dir
                                .absoluteFilePath(QString("gt.txt"));
    qDebug() << "Looking for: " << annotation_file;

    // Find images:
    qInfo() << "Loading annotations";
    auto labels = getLabels(annotation_file);

    qInfo() << "Checking images";

    auto pbar = cliProgressBar();
    double progress = 0;
    int i = 0;

    auto images = QDir(image_folder).entryList(QDir::Files);
    for(auto & image : images){
        qDebug() << "Adding labels for" << image;

        // Extract image ID, ignoring leading zeros
        auto split_file = QFileInfo(image).baseName().split("_");
        int image_id = split_file.back().toInt();

        // Get boxes for this ID and add to DB
        auto boxes = findBoxes(labels, image_id);

        progress = 100*static_cast<double>(i++)/images.size();
        pbar.update(progress);
        pbar.print();

        QString abs_image_path = QDir(image_folder).absoluteFilePath(image);
        if(boxes.empty() && !import_unlabelled){
            continue;
        }

        label_list.append(boxes);
        image_list.append(abs_image_path);

    }
    qInfo() << "\nInserting into database";
    project->addLabelledAssets(image_list, label_list);

}

void MOTImporter::import(QString sequence_folder){
    QDir seq_dir(sequence_folder);

    // Find all sequence folders
    QDirIterator it(sequence_folder, QDir::Dirs | QDir::NoDotAndDotDot);
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

        qInfo() << "Processing: " << subfolder;
        importSequence(subfolder);

    }
}

QVector<QStringList> MOTImporter::getLabels(QString annotation_file){
    auto lines = readLines(annotation_file);
    QVector<QStringList> labels;

    auto pbar = cliProgressBar();
    double progress = 0;
    int i = 0;

    for(auto &line : lines){
        auto label = line.simplified().split(",");

        // Note - MOT docs say 10, but MOTDet only contains 9 elements
        if(label.size() < 8){
            qWarning() << "Found :" <<  label.size() << " elements - need at least 9.";
            continue;
        }else{
            labels.push_back(label);
        }

        progress = 100*static_cast<double>(i++)/lines.size();
        pbar.update(progress);
        pbar.print();
    }

    qInfo() << "";

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

        bbox.classid = label.at(7).toInt();
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
    QList<QString> class_list;
    project->getClassList(class_list);

    if (fh.open(QIODevice::ReadOnly)) {

        while (!fh.atEnd()) {
            // Darknet name file is just a newline delimited list of classes
            QByteArray line = fh.readLine();

            if(QString(line) == "") continue;

            auto new_class = line.simplified();

            if(!class_list.contains(new_class)){
                project->addClass(new_class);
            }else{
                qDebug() << "Added: " << new_class;
            }
        }
    }


}
