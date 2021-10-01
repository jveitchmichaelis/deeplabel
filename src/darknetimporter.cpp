#include "darknetimporter.h"

void DarknetImporter::import(QString image_list, QString names_file, QString root_folder)
{
    loadClasses(names_file);

    // Get image filenames
    auto filenames = readLines(image_list);
    filenames.sort();

    QProgressDialog progress("...", "Abort", 0, filenames.size(), static_cast<QWidget*>(parent()));
    progress.setWindowModality(Qt::WindowModal);
    progress.setWindowTitle("Loading images and labels");
    int i = 0;

    QList<QList<BoundingBox>> bboxes;

    for(auto &image_path : filenames){

        if(progress.wasCanceled())
            break;

        if (root_folder != "") {
            image_path = QFileInfo(QDir(root_folder).absoluteFilePath(image_path))
                             .canonicalFilePath();
        }

        bboxes.append(loadLabels(image_path));

        progress.setValue(++i);
        progress.setLabelText(QString("%1/%2: %3").arg(i).arg(filenames.size()).arg(image_path));

    }

    if(progress.wasCanceled())
        project->addLabelledAssets(filenames.mid(0, bboxes.size()), bboxes);
    else
        project->addLabelledAssets(filenames, bboxes);
}

QList<BoundingBox> DarknetImporter::loadLabels(QString image_path){

    QList<BoundingBox> boxes = {};
    auto info = QFileInfo(image_path);
    QString filename_noext = QDir(info.absolutePath()).filePath(info.baseName());
    QString label_filename = QString("%1.txt").arg(filename_noext);

    auto image = cv::imread(image_path.toStdString());
    auto width = image.cols;
    auto height = image.rows;

    if(width <= 0 || height <= 0) return boxes;

    auto lines = readLines(label_filename);
    for(auto &line : lines){
        BoundingBox bbox;
        auto label = line.simplified().split(" ");

        if(label.size() != 5) continue;

        bbox.classid = label.at(0).toInt() + 1; // Since in the database they're 1-indexed
        bbox.classname = project->getClassName(bbox.classid);

        if(bbox.classname == ""){
            qWarning() << "Class" << bbox.classid << " not found in names file.";
        }

        int center_x = static_cast<int>(label.at(1).toFloat() * width);
        int center_y = static_cast<int>(label.at(2).toFloat() * height);
        int box_width = static_cast<int>(label.at(3).toFloat() * width);
        int box_height = static_cast<int>(label.at(4).toFloat() * height);

        auto top_left = QPoint(std::min(std::max(center_x - box_width/2, 0), width),
                               std::min(std::max(center_y - box_height/2, 0), height));

        auto bottom_right = QPoint(std::min(std::max(center_x + box_width/2, 0), width),
                                   std::min(std::max(center_y + box_height/2, 0), height));

        bbox.rect = QRect(top_left, bottom_right);

        boxes.append(bbox);
    }

    return boxes;
}

void DarknetImporter::loadClasses(QString names_file){
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
