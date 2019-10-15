#include "cocoexporter.h"

CocoExporter::CocoExporter(LabelProject *project, QObject *parent) : QObject(parent)
{

    this->project = project;
    project->getImageList(images);
}

void CocoExporter::splitData(float split, bool shuffle, int seed){

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

    qDebug() << "Split: " << split;
    qDebug() << train_set.size() << " images selected for train set.";
    qDebug() << validation_set.size() << " images selected for validation set.";

}

bool CocoExporter::setOutputFolder(const QString folder){

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

bool CocoExporter::processImages(const QString folder, const QString filename, const QList<QString> images){

    QString image_path;
    QList<BoundingBox> labels;

    auto now = QDateTime::currentDateTime();
    QJsonObject label_file;

    QJsonObject label_info;
    label_info["year"] = now.date().year();
    label_info["version"] = "1";
    label_info["description"] = "Description";
    label_info["contributor"] = "Contributor";
    label_info["date_created"] = now.date().toString(Qt::ISODate);
    label_info["url"] = "";

    label_file["info"] = label_info;

    QJsonArray annotations_array;
    QJsonArray licenses_array;
    QJsonArray image_array;
    QJsonArray category_array;

    int i = 0;

    QList<QString> classnames;
    project->getClassList(classnames);

    for(auto &classname : classnames){
        QJsonObject category;
        category["id"] = project->getClassId(classname);
        category["name"] = classname;
        category["supercategory"] = classname;

        category_array.append(category);
    }

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
        //saveImage(image, image_filename);

        if(image.empty()) continue;

        auto copied = QFile::copy(image_path, image_filename);
        if(!copied){
            qDebug() << "Failed to copy image" << image_filename;
        }

        QJsonObject image_info;
        image_info["id"] = image_id;
        image_info["width"] = image.cols;
        image_info["height"] = image.rows;
        image_info["file_name"] = image_filename;
        image_info["license"] = 0;
        image_info["flickr_url"] = "";
        image_info["coco_url"] = "";
        image_info["date_captured"] = now.date().toString(Qt::ISODate);

        image_array.append(image_info);

        for(auto &label : labels){
            QJsonObject annotation;
            annotation["id"] = label_id;
            annotation["image_id"] = image_id;
            annotation["category_id"] = label.classid;

            QJsonArray segmentation;
            segmentation.append(QString::number(label.rect.topLeft().x()));
            segmentation.append(QString::number(label.rect.topLeft().y()));
            segmentation.append(QString::number(label.rect.topRight().x()));
            segmentation.append(QString::number(label.rect.topRight().y()));
            segmentation.append(QString::number(label.rect.bottomRight().x()));
            segmentation.append(QString::number(label.rect.bottomRight().y()));
            segmentation.append(QString::number(label.rect.bottomLeft().x()));
            segmentation.append(QString::number(label.rect.bottomLeft().y()));

            QJsonArray segmentation_array;
            segmentation_array.append(segmentation);
            annotation["segmentation"] = segmentation_array;

            annotation["area"] = label.rect.width()*label.rect.height();

            QJsonArray bbox;
            bbox.append(QString::number(label.rect.x()));
            bbox.append(QString::number(label.rect.y()));
            bbox.append(QString::number(label.rect.width()));
            bbox.append(QString::number(label.rect.height()));

            annotation["bbox"] = bbox;
            annotation["iscrowd"] = 0;

            annotations_array.append(annotation);
            label_id++;
        }

        image_id++;
        emit export_progress((100 * i++)/images.size());
    }

    QJsonObject license;
    license["id"] = 0;
    license["name"] = "";
    license["url"] = "";

    licenses_array.append(license);

    label_file["images"] = image_array;
    label_file["annotations"] = annotations_array;
    label_file["license"] = licenses_array;
    label_file["categories"] = category_array;

    QString label_filename = QString("%1/%2.json").arg(output_folder).arg(filename);

    QJsonDocument json_output(label_file);
    QFile f(label_filename);

    f.open(QIODevice::WriteOnly | QIODevice::Truncate);

    if(f.isOpen()){
        f.write(json_output.toJson());
    }

    f.close();

    return true;
}

void CocoExporter::process(){
    image_id = 0;
    label_id = 0;

    processImages(train_folder, "train", train_set);
    processImages(val_folder, "val", validation_set);
}
