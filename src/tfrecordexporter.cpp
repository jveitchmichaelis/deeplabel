#include "tfrecordexporter.h"

void TFRecordExporter::generateLabelIds(const QString names_file){
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
        qWarning() << "No classes found in names file.";
        return;
    }

    int i = 1; // 0 is background
    for(auto &name : class_list){
        auto cleaned_name = name.simplified().toLower();
        id_map[cleaned_name] = i++;
        qDebug() << "Adding: " << cleaned_name << " (" << i << ")";
    }
}

void TFRecordExporter::add_int64_feature(tensorflow::Features &features,
                                         std::string key,
                                         std::vector<int64> values){

    auto map = features.mutable_feature();

    for(auto value : values){
        (*map)[key].mutable_int64_list()->add_value(value);
    }
}

void TFRecordExporter::add_bytes_feature(tensorflow::Features &features,
                                         std::string key,
                                         std::vector<void*> values,
                                         std::vector<size_t> sizes){

    auto map = features.mutable_feature();

    for(size_t i=0; i < values.size(); ++i){
        (*map)[key].mutable_bytes_list()->add_value(static_cast<char*>(values.at(i)),
                                                    sizes.at(i));
    }

}

void TFRecordExporter::add_bytes_feature(tensorflow::Features &features,
                                         std::string key,
                                         std::vector<std::string> values){

    auto map = features.mutable_feature();

    for(auto &value : values){
        (*map)[key].mutable_bytes_list()->add_value(value);
    }
}

void TFRecordExporter::add_float_feature(tensorflow::Features &features,
                                         std::string key,
                                         std::vector<float> values){
    auto map = features.mutable_feature();

    for(auto value : values){
        (*map)[key].mutable_float_list()->add_value(value);
    }
}

void TFRecordExporter::create_weights(){

    QMap<int, int> counts;
    project->getClassCounts(counts);
    int n_class = counts.size();

    int total_labels = 0;
    for(auto class_id : counts.keys()){
        total_labels += counts[class_id];
    }

    float total_weight = 0;
    for(auto class_id : counts.keys()){
        weight_map[class_id] = total_labels / counts[class_id];
        total_weight += weight_map[class_id];
    }

    for(auto &class_id : weight_map.keys()){
        weight_map[class_id] = weight_map[class_id]/total_weight * n_class;
        qInfo() << "Set weight for class " << class_id << "to: " << weight_map[class_id];
    }

}

bool TFRecordExporter::create_example(tensorflow::Example &example, QString image_path)
{

    QList<BoundingBox> labels;
    project->getLabels(image_path, labels);

    if(labels.size() == 0 && !export_unlabelled){
        return false;
    }else{
        qInfo() << "Processing " << image_path;
    }

    auto db_dir = project->getDbFolder();
    auto abs_path = QFileInfo(QDir::cleanPath(db_dir.filePath(image_path))).absoluteFilePath();

    QFile image(abs_path);
    image.open(QIODevice::ReadOnly);
    QByteArray buffer = image.readAll();

    auto im = cv::imdecode(cv::Mat(1, buffer.size(), CV_8UC1, buffer.data()), cv::IMREAD_UNCHANGED);
    auto image_ext = QFileInfo(image_path).suffix();

    tensorflow::Features features;

    // Image data
    add_int64_feature(features, "image/width", {im.cols});
    add_int64_feature(features, "image/height", {im.rows});
    add_bytes_feature(features, "image/filename", {image_path.toStdString()});
    add_bytes_feature(features, "image/source_id", {image_path.toStdString()});
    add_bytes_feature(features, "image/format", {image_ext.toStdString()});

    add_bytes_feature(features, "image/encoded", {buffer.data()},
                                                 {static_cast<size_t>(buffer.size())});
    image.close();

    std::vector<float> xmins;
    std::vector<float> xmaxs;
    std::vector<float> ymins;
    std::vector<float> ymaxs;
    std::vector<std::string> classes_text;
    std::vector<int64> classes;
    std::vector<float> class_weights;

    for(auto &label : labels){

        // Check if this label exists in the database
        if(id_map.find(label.classname.toLower()) == id_map.end()){
            qWarning() << "Couldn't find this label in the names file: " << label.classname.toLower();
            continue;
        }

        xmins.push_back(label.rect.left());
        xmaxs.push_back(label.rect.right());
        ymins.push_back(label.rect.bottom());
        ymaxs.push_back(label.rect.top());
        classes_text.push_back(label.classname.toStdString());
        classes.push_back(id_map[label.classname]);
        class_weights.push_back(weight_map[id_map[label.classname]]);

    }

    // Bounding boxes
    add_float_feature(features, "image/object/bbox/xmin", xmins);
    add_float_feature(features, "image/object/bbox/xmax", xmaxs );
    add_float_feature(features, "image/object/bbox/ymin", ymins );
    add_float_feature(features, "image/object/bbox/ymaxs", ymaxs );
    add_bytes_feature(features, "image/object/class/text", classes_text );
    add_int64_feature(features, "image/object/class/label", classes );
    //add_float_feature(features, "image/object/weight", class_weights );

    *example.mutable_features() = features;

    return true;
}

void TFRecordExporter::setNumberShards(int n){
    number_shards = n;
}

bool TFRecordExporter::processImages(const QString folder, const QString record_prefix, const QList<QString> images, export_image_type split_type){

    QString image_path;
    QList<BoundingBox> labels;

    QProgressDialog progress("...", "Abort", 0, images.size(), static_cast<QWidget*>(parent()));
    progress.setWindowModality(Qt::WindowModal);

    if(folder == ""){
        qCritical() << "Invalid folder specified.";
        return false;
    }

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

    if(!disable_progress){
        progress.hide();
    }

    int i = 0;
    int processed = 0;

    // Open shards
    QList<QFile *> shards;
    for(int s=0; s < number_shards; s++){
        qInfo() << "Creating output shard files";
        QString shard_name = QString("%1-%2-of-%3.tfrecord")
                                    .arg(record_prefix)
                                    .arg(s, 5, 10, static_cast<QChar>('0'))
                                    .arg(number_shards, 5, 10, static_cast<QChar>('0'));
        QString shard_path = QDir(output_folder).absoluteFilePath(shard_name);
        auto shard = new QFile(shard_path);

        if(shard->exists()){
            shard->remove();
            qWarning() << "Deleting and overwriting " << shard_path;
        }

        shard->open(QFile::Append);
        shards.push_back(shard);
    }

    foreach(image_path, images){

        if(progress.wasCanceled()){
            break;
        }

        project->getLabels(image_path, labels);

        tensorflow::Example example;
        bool res = create_example(example, image_path);

        if(res){
            qDebug() << "creating record for " << image_path;
            int shard_id = processed++ % number_shards;

            auto serialised_example = example.SerializeAsString();
            auto shard = shards.at(shard_id);

            if(shard->isOpen()){
                // Write header (data length, crc of data length)
                size_t data_len = example.ByteSizeLong();
                auto data_len_ptr = reinterpret_cast<const char*>(&data_len);
                int nbytes = shard->write(data_len_ptr, 8);

                uint32_t crc32 = tf_crc::Mask(tf_crc::Value(data_len_ptr, 8));
                shard->write(reinterpret_cast<const char*>(&crc32), 4);

                // Write record
                nbytes += shard->write(serialised_example.data(),
                                       serialised_example.size());

                // Write footer (CRC32 of data)
                crc32 = tf_crc::Mask(tf_crc::Value(serialised_example.data(),
                                                   serialised_example.size()));
                shard->write(reinterpret_cast<const char*>(&crc32), 4);

                if(nbytes <=0){
                    qCritical() << "Failed to write shard";
                }else{
                    qDebug() << "Wrote " << nbytes << "bytes";
                }

            }else{
                qCritical() << "Shard is not open for writing";
            }
        }

        if(!disable_progress){
            progress.setValue(i++);
            progress.setLabelText(image_path);
        }

    }

    for(auto &shard : shards){
        shard->close();
        delete shard;
    }

    return true;
}

void TFRecordExporter::process(void){
    create_weights();
    processImages(train_folder, "train", train_set, EXPORT_TRAIN);
    processImages(val_folder, "val", validation_set, EXPORT_VAL);
}
