#include "baseexporter.h"

BaseExporter::BaseExporter(LabelProject *project, QObject *parent) : QObject(parent)
{

    this->project = project;
    project->getImageList(images);
}

void BaseExporter::splitData(float split, bool shuffle, int seed){

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
    validation_set = images.mid(0, pivot);
    train_set = images.mid(pivot);

    qDebug() << "Split: " << split;
    qDebug() << train_set.size() << " images selected for train set.";
    qDebug() << validation_set.size() << " images selected for validation set.";

}

bool BaseExporter::setOutputFolder(const QString folder){

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

    if(validation_split){
        val_folder = QDir::cleanPath(output_folder+"/val");
        if (!QDir(val_folder).exists()){
            qDebug() << "Making validation folder" << val_folder;
            QDir().mkpath(val_folder);
        }

        val_label_folder = QDir::cleanPath(val_folder);
        val_image_folder = QDir::cleanPath(val_folder);
    }

    train_label_folder = QDir::cleanPath(train_folder);
    train_image_folder = QDir::cleanPath(train_folder);

    return true;

}

void BaseExporter::setFilenamePrefix(QString prefix){
    if(prefix != ""){
        filename_prefix = prefix;
    }
}

bool BaseExporter::saveImage(cv::Mat &image, const QString output, const double scale_x, const double scale_y){

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
