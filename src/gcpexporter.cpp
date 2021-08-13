#include "gcpexporter.h"

bool GCPExporter::processImages(const QString folder,
                                const QString filename,
                                const QList<QString> images,
                                export_image_type split_type){
    QString image_path;
    QList<BoundingBox> labels;
    QString label_filename = QString("%1/%2.txt").arg(folder).arg(filename);

    QFile f(label_filename);
    if (!f.open(QIODevice::Append | QIODevice::Truncate)) {
        return false;
    }

    QProgressDialog progress("...", "Abort", 0, images.size(), static_cast<QWidget*>(parent()));
    progress.setWindowModality(Qt::WindowModal);


    if(!disable_progress){
        QString split_text = "";
        if(split_type == EXPORT_VAL){
            split_text = "VAL";
            progress.setWindowTitle("Exporting validation images");
        }else if(split_type == EXPORT_TRAIN){
            split_text = "TRAIN";
            progress.setWindowTitle("Exporting train images");
        }else if(split_type == EXPORT_TEST){
            split_text = "TEST";
            progress.setWindowTitle("Exporting test images");
        }else{
            split_text = "UNASSIGNED";
        }
    }else{
        progress.hide();
    }

    int i=0;

    foreach(image_path, images){

        if(progress.wasCanceled()){
            break;
        }

        // Check image
        cv::Mat image = cv::imread(image_path.toStdString());
        if(image.empty()) continue;

        double width = static_cast<double>(image.cols);
        double height = static_cast<double>(image.rows);

        project->getLabels(image_path, labels);

        // Check labels
        if(!export_unlabelled && labels.size() == 0){
            if(!disable_progress){
                progress.setValue(i);
                progress.setLabelText(QString("%1 is unlabelled").arg(image_path));
                progress.repaint();
                QApplication::processEvents();
            }
            continue;
        }

        // Deal with duplicate filenames, etc
        QString extension = QFileInfo(image_path).suffix();
        QString filename_noext = QFileInfo(image_path).completeBaseName();

        QString image_filename = QString("%1/%2.%3").arg(folder).arg(filename_noext).arg(extension);

        // Correct for duplicate file names in output
        int dupe_file = 1;
        while(QFile(image_filename).exists()){
            image_filename = QString("%1/%2_%3.%4").arg(folder).arg(filename_noext).arg(dupe_file++).arg(extension);
        }

        filename_noext = QFileInfo(image_filename).completeBaseName();

        auto copied = QFile::copy(image_path, image_filename);
        if(!copied){
            qDebug() << "Failed to copy image" << image_filename;
        }

        auto bucket_path = QString("%1/%2.%3").arg(bucket_uri).arg(filename_noext).arg(extension);

        for(auto &label : labels){
            auto gcp_label = QString("%1,%2,%3,%4,%5,,,%6,%7,,\n")
                    .arg(split_type)
                    .arg(bucket_path)
                    .arg(label.classname)
                    .arg(QString::number(static_cast<double>(label.rect.topLeft().x())/width))
                    .arg(QString::number(static_cast<double>(label.rect.topLeft().y())/height))
                    .arg(QString::number(static_cast<double>(label.rect.bottomRight().x())/width))
                    .arg(QString::number(static_cast<double>(label.rect.bottomRight().x())/height));

           qDebug() << gcp_label;

           f.write(gcp_label.toUtf8());

        }

        if(!disable_progress){
            progress.setValue(i++);
            progress.setLabelText(image_filename);
            QApplication::processEvents();
        }
    }

    return true;
}

bool GCPExporter::setOutputFolder(const QString folder){

    if(folder == "") return false;

    output_folder = folder;

    //Make output folder if it doesn't exist
    if (!QDir(output_folder).exists()){
        qDebug() << "Making output folder" << output_folder;
        QDir().mkpath(output_folder);
    }

    image_folder = QDir::cleanPath(output_folder+"/images");
    if (!QDir(image_folder).exists()){
        qDebug() << "Making validation folder" << image_folder;
        QDir().mkpath(image_folder);
    }

    image_folder = QDir::cleanPath(image_folder);

    return true;

}

void GCPExporter::setBucket(QString uri){

    if(!uri.startsWith("gs://")){
        bucket_uri = QString("gs://%1").arg(uri);
    }else{
        bucket_uri = uri;
    }
}

void GCPExporter::process(){
    processImages(image_folder, "labels", train_set, EXPORT_TRAIN);
    processImages(image_folder, "labels", validation_set, EXPORT_VAL);
}
