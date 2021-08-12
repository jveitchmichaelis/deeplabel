#include "kittiexporter.h"

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
            text += " 0.0";
            text += " 0";
            text += " 0.0";
            text += QString(" %1").arg(label.rect.left() * scale_x);
            text += QString(" %2").arg(label.rect.top() * scale_x);
            text += QString(" %3").arg(label.rect.right() * scale_y);
            text += QString(" %4").arg(label.rect.bottom() * scale_y);
            text += " 0.0 0.0 0.0 0.0 0.0 0.0 0.0\n";

            f.write(text.toUtf8());

        }

    }
}

int KittiExporter::processSet(QString folder, QList<QString> images, int n_images){

    int base = 10;
    int pad = 5;

    QString image;
    QList<BoundingBox> labels;
    double scale_x=1, scale_y=1;

    QProgressDialog progress("...", "Abort", 0, images.size(), static_cast<QWidget*>(parent()));
    progress.setWindowModality(Qt::WindowModal);
    int progress_count = 0;

    foreach(image, images){

        if(progress.wasCanceled()){
            break;
        }

        project->getLabels(image, labels);

        if(!export_unlabelled && labels.size() == 0){
            progress.setValue(progress_count);
            progress.setLabelText(QString("%1 is unlabelled").arg(image));
            progress.repaint();
            QApplication::processEvents();
            continue;
        }

        QString extension = QFileInfo(image).suffix();
        QString filename_noext = QFileInfo(image).completeBaseName();

        QString image_filename = QString("%1/%2.%3").arg(folder).arg(filename_noext).arg(extension);

        // Correct for duplicate file names in output
        int dupe_file = 1;
        while(QFile(image_filename).exists()){
            image_filename = QString("%1/%2_%3.%4").arg(folder).arg(filename_noext).arg(dupe_file++).arg(extension);
        }

        QString label_filename = QString("%1/labels/%2.txt").arg(folder).arg(n_images, pad, base, QChar('0'));
        appendLabel(label_filename, labels, scale_x, scale_y);

        progress.setValue(progress_count++);
        progress.setLabelText(image_filename);
        QApplication::processEvents();

    }

    return n_images;
}

void KittiExporter::process(){

    int n_images = 0;

    n_images = processSet(train_folder, train_set, 0);
    processSet(val_folder, validation_set, n_images);

}

