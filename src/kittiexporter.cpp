#include "kittiexporter.h"

KittiExporter::KittiExporter(LabelProject *project, QObject *parent) : QObject(parent)
{
    this->project = project;
    project->getImageList(images);
}


void KittiExporter::splitData(float split, bool shuffle){

    if(shuffle){
        std::random_shuffle(images.begin(), images.end());
    }

    auto pivot = images.size() * split;
    train_set = images.mid(0, pivot);
    validation_set = images.mid(pivot);

}

bool KittiExporter::setOutputFolder(QString folder){

    if(folder == "") return false;

    output_folder = folder;

    train_folder = QDir::cleanPath(output_folder + "/train");
    val_folder = QDir::cleanPath(output_folder + "/val");

    train_label_folder = QDir::cleanPath(train_folder + "/labels");
    train_image_folder = QDir::cleanPath(train_folder + "/images");
    val_label_folder = QDir::cleanPath(val_folder + "/labels");
    val_image_folder = QDir::cleanPath(val_folder + "/images");

    QDir(output_folder).mkpath(train_label_folder);
    QDir(output_folder).mkpath(train_image_folder);
    QDir(output_folder).mkpath(val_label_folder);
    QDir(output_folder).mkpath(val_image_folder);

    if(!QDir(train_label_folder).exists()) return false;
    if(!QDir(train_image_folder).exists()) return false;
    if(!QDir(val_label_folder).exists()) return false;
    if(!QDir(val_image_folder).exists()) return false;

    return true;

}

void KittiExporter::appendLabel(QString label_filename, QList<BoundingBox> labels, double scale_x, double scale_y){

    QFile f(label_filename);

    if (f.open(QIODevice::WriteOnly | QIODevice::Append)) {
        BoundingBox label;
        foreach(label, labels){
            QString text;

            text += label.classname;
            text += " 0";
            text += " 0";
            text += " 0";
            text += QString(" %1").arg(label.rect.left() / scale_x);
            text += QString(" %2").arg(label.rect.right() / scale_x);
            text += QString(" %3").arg(label.rect.top() / scale_y);
            text += QString(" %4").arg(label.rect.bottom() / scale_y);
            text += " 0 0 0 0 0 0 0 0\n";

            f.write(text.toUtf8());

        }

    }
}

bool KittiExporter::saveImage(QString input, QString output, double &scale_x, double &scale_y){
    return true;
}

int KittiExporter::processSet(QString folder, QList<QString> images, int i){

    int base = 10;
    int pad = 5;

    QString image;
    QList<BoundingBox> labels;

    foreach(image, images){
        project->getLabels(image, labels);

        if(labels.size() > 0){

            QString extension = QFileInfo(image).suffix();
            QString image_filename = QString("%1/images/%2.%3").arg(folder).arg(i, pad, base, QChar('0')).arg(extension);
            saveImage(image, image_filename, scale_x, scale_y);

            QString label_filename = QString("%1/labels/%2.txt").arg(folder).arg(i, pad, base, QChar('0'));
            appendLabel(label_filename, labels, scale_x, scale_y);

            i++;
        }

    }

    return i;
}

void KittiExporter::process(){

    int i = 0;

    i = processSet(train_folder, train_set, i);

    processSet(val_folder, validation_set, i);

}

