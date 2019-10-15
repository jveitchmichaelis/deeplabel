#include "pascalvocexporter.h"

PascalVocExporter::PascalVocExporter(LabelProject *project, QObject *parent) : QObject(parent)
{
    this->project = project;
    project->getImageList(images);
}

void PascalVocExporter::splitData(float split, bool shuffle, int seed){

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

bool PascalVocExporter::setOutputFolder(const QString folder){

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

void PascalVocExporter::writeLabels(const cv::Mat &image, const QString image_filename, const QString label_filename, const QList<BoundingBox> labels){

    // Still make a label file even if there are no detections. This is important
    // for background class detection.

    QFile f(label_filename);

    // Delete existing files for simplicity.
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QXmlStreamWriter stream(&f);
        stream.setAutoFormatting(true);
        stream.writeStartDocument();

        stream.writeStartElement("annotation");

        auto finfo = QFileInfo(image_filename);

        stream.writeTextElement("folder", finfo.dir().dirName());
        stream.writeTextElement("filename", finfo.fileName());
        stream.writeTextElement("path", finfo.filePath());

        QString label_database = "";
        stream.writeStartElement("source");
            stream.writeTextElement("database", label_database);
        stream.writeEndElement(); // source

        // Image properties
        stream.writeStartElement("size");
            stream.writeTextElement("width", QString::number(image.cols));
            stream.writeTextElement("height", QString::number(image.rows));
            stream.writeTextElement("depth", QString::number(image.channels()));
        stream.writeEndElement(); // size

        // Not a segmentation mask
        stream.writeTextElement("segmented", "0");

        // Labels
        for(auto &label : labels){
            stream.writeStartElement("object");
                stream.writeTextElement("name", label.classname);
                stream.writeTextElement("pose", "Unspecified");
                stream.writeTextElement("truncated", "0");
                stream.writeTextElement("difficult", "0");

                stream.writeStartElement("bndbox");
                    stream.writeTextElement("xmin", QString::number(label.rect.left()));
                    stream.writeTextElement("ymin", QString::number(label.rect.top()));
                    stream.writeTextElement("xmax", QString::number(label.rect.right()));
                    stream.writeTextElement("ymax", QString::number(label.rect.bottom()));
                stream.writeEndElement(); //bndbox

            stream.writeEndElement(); //object
        }

        stream.writeEndElement(); // annotation
        stream.writeEndDocument();

        f.close();
    }
}

bool PascalVocExporter::saveImage(cv::Mat &image, const QString output, const double scale_x, const double scale_y){

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

bool PascalVocExporter::processImages(const QString folder, const QList<QString> images){

    QString image_path;
    QList<BoundingBox> labels;

    int i = 0;

    foreach(image_path, images){
        project->getLabels(image_path, labels);

        if(!export_unlabelled && labels.size() == 0) continue;


        QString extension = QFileInfo(image_path).suffix();
        QString filename_noext = QFileInfo(image_path).baseName();
        QString image_filename = QString("%1/%2.%3").arg(folder).arg(filename_noext).arg(extension);

        // Correct for duplicate file names in output
        int dupe_file = 1;
        while(QFile(image_filename).exists()){
            image_filename = QString("%1/%2_%3.%4").arg(folder).arg(filename_noext).arg(dupe_file++).arg(extension);
        }

        cv::Mat image = cv::imread(image_path.toStdString());

        // Copy the image to the new folder
        //saveImage(image, image_filename);
        QFile::copy(image_path, image_filename);

        QString label_filename = QString("%1/%2.xml").arg(folder).arg(filename_noext);
        writeLabels(image, image_filename, label_filename, labels);

        emit export_progress((100 * i)/images.size());
    }

    return true;
}

void PascalVocExporter::saveLabelMap(QString filename){

    QFile f(filename);

    // Delete existing files for simplicity.
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

        QList<QString> classes;
        project->getClassList(classes);

        for(auto classname : classes){
            f.write("item {\n");
            f.write(QString("  name: %1\n").arg(classname).toStdString().c_str());
            f.write(QString("  id: %1\n").arg(project->getClassId(classname)).toStdString().c_str());
            f.write(QString("  displayname: %1\n").arg(classname).toStdString().c_str());
            f.write("}\n");
        }

        f.close();
    }

}

void PascalVocExporter::process(bool export_map){
    processImages(train_folder, train_set);
    processImages(val_folder, validation_set);

    if(export_map){
        saveLabelMap(QString("%1/%2").arg(output_folder).arg("label_map.pbtxt"));
    }
}

