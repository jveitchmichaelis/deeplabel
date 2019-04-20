#include "darknetexporter.h"

DarknetExporter::DarknetExporter(LabelProject *project, QObject *parent) : QObject(parent)
{
    this->project = project;
    project->getImageList(images);
}

void DarknetExporter::splitData(float split, bool shuffle, int seed){

    if(split < 0 || split > 1){
        qDebug() << "Invalid split fraction, should be [0,1]";
    }

    if(shuffle){
        std::random_device rd;
        std::mt19937 generator(rd());
        generator.seed(static_cast<unsigned int>(seed));

        std::shuffle(images.begin(), images.end(), generator);
    }

    int pivot = static_cast<int>(images.size() * split);
    train_set = images.mid(0, pivot);
    validation_set = images.mid(pivot);

    qDebug() << train_set.size() << " images selected for train set.";
    qDebug() << validation_set.size() << " images selected for validation set.";

}

bool DarknetExporter::setOutputFolder(const QString folder){

    if(folder == "") return false;

    output_folder = folder;

    //Make output folder if it doesn't exist
    if (!QDir(output_folder).exists()){
        qDebug() << "Making output folder" << output_folder;
        QDir().mkpath(output_folder);
    }

    //Make the training and validation folders
    train_folder = QDir::cleanPath(output_folder+"/train");
    if (!QDir(train_folder).exists()){
        qDebug() << "Making training folder" << train_folder;
        QDir().mkpath(train_folder);
    }

    val_folder = QDir::cleanPath(output_folder+"/val");
    if (!QDir(val_folder).exists()){
        qDebug() << "Making validation folder" << val_folder;
        QDir().mkpath(val_folder);
    }

    train_label_folder = QDir::cleanPath(train_folder);
    train_image_folder = QDir::cleanPath(train_folder);
    val_label_folder = QDir::cleanPath(val_folder);
    val_image_folder = QDir::cleanPath(val_folder);

    return true;

}

void DarknetExporter::generateLabelIds(const QString names_file){
    id_map.clear();

    // Force sort list
    QStringList class_list;
    QFile fh(names_file);

    if (fh.open(QIODevice::ReadOnly)) {

        while (!fh.atEnd()) {
            // Darknet name file is just a newline delimited list of classes
            QByteArray line = fh.readLine();
            class_list.append(line);
        }
    }

    if(class_list.size() == 0){
        qDebug() << "No classes found in names file.";
        return;
    }

    int i = 0;
    for(auto &name : class_list){
        auto cleaned_name = name.simplified().toLower();
        id_map[cleaned_name] = i++;
        qDebug() << "Adding: " << cleaned_name << " (" << i << ")";
    }
}

void DarknetExporter::writeLabels(const cv::Mat &image, const QString label_filename, const QList<BoundingBox> labels){

    // Still make a label file even if there are no detections. This is important
    // for background class detection.

    QFile f(label_filename);

    // Delete existing files for simplicity.
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        BoundingBox label;
        foreach(label, labels){
            QString text;

            // Check if this label exists in the database
            if(id_map.find(label.classname.toLower()) == id_map.end()){
                qDebug() << "Couldn't find this label in the names file";
                continue;
            }

            double x = static_cast<double>(label.rect.center().x())/image.cols;
            double y = static_cast<double>(label.rect.center().y())/image.rows;
            double width = static_cast<double>(label.rect.width())/image.cols;
            double height = static_cast<double>(label.rect.height())/image.rows;

            text += QString("%1").arg(id_map[label.classname.toLower()]);
            text += QString(" %1").arg(x);
            text += QString(" %1").arg(y);
            text += QString(" %1").arg(width);
            text += QString(" %1").arg(height);
            text += "\n";

            f.write(text.toUtf8());

        }
    }
}

bool DarknetExporter::saveImage(cv::Mat &image, const QString output, const double scale_x, const double scale_y){

    if(image.rows == 0 || image.cols == 0){
        qDebug() << "Empty image ";
        return false;
    }

    if(scale_x > 0 && scale_y > 0)
        cv::resize(image, image, cv::Size(), scale_x, scale_y);

    std::vector<int> compression_params;

    // Png compression - maximum is super slow
    // TODO: add support to adjust this
    if(output.split(".").last().toLower() == "png"){
        compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(6);
    }

    return cv::imwrite(output.toStdString(), image, compression_params);
}

bool DarknetExporter::processImages(const QString folder, const QList<QString> images){

    QString image_path;
    QList<BoundingBox> labels;

    int i = 0;

    foreach(image_path, images){
        qDebug() << image_path;
        project->getLabels(image_path, labels);

        QString extension = QFileInfo(image_path).suffix();
        QString filename_noext = QFileInfo(image_path).baseName();
        QString image_filename = QString("%1/%2.%3").arg(folder).arg(filename_noext).arg(extension);

        cv::Mat image = cv::imread(image_path.toStdString());
        //saveImage(image, image_filename);

        QFile::copy(image_path, image_filename);

        QString label_filename = QString("%1/%2.txt").arg(folder).arg(filename_noext);
        writeLabels(image, label_filename, labels);

        emit export_progress((100 * i)/images.size());

    }

    return true;
}

void DarknetExporter::process(){
    processImages(train_folder, train_set);
    processImages(val_folder, validation_set);
}
