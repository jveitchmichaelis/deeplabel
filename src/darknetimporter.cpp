#include "darknetimporter.h"

QList<QString> readLines(QString path){
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

void DarknetImporter::import(QString image_list, QString names_file){

    loadClasses(names_file);

    // Get image filenames
    auto filenames = readLines(image_list);

    QProgressDialog progress("...", "Abort", 0, filenames.size(), static_cast<QWidget*>(parent()));
    progress.setWindowModality(Qt::WindowModal);
    progress.setWindowTitle("Loading images");
    int i = 0;

    for(auto &image_path : filenames){

        if(progress.wasCanceled())
            break;

        qDebug() << "Importing " << image_path;

        auto boxes = loadLabels(image_path);

        qDebug() << "Found: " << boxes.size() << " labels.";

        addAsset(image_path, boxes);

        progress.setValue(++i);
        progress.setLabelText(image_path);

    }
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

        // bbox.classname = project->getClassName();
        bbox.classid = label.at(0).toInt();

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
        }
    }


}
