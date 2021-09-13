#include "tfrecordimporter.h"

void TFRecordImporter::find_example_boundaries(QByteArray buffer, std::vector<size_t> &record_offsets, std::vector<size_t> &record_sizes){

    size_t offset = 0;
    size_t next_offset = 0;
    size_t header_size = 12;
    size_t footer_size = 4;

    size_t buffer_size = static_cast<unsigned long long>(buffer.size());

    while(offset < buffer_size){
        QByteArray data_len_buf = buffer.mid(static_cast<int>(offset), 8);
        QByteArray data_len_crc_buf = buffer.mid(static_cast<int>(offset)+8, 4);

        size_t data_size = *reinterpret_cast<size_t*>(data_len_buf.data());
        size_t data_len_crc = *reinterpret_cast<uint32_t*>(data_len_crc_buf.data());

        uint32_t crc = tf_crc::Mask(tf_crc::Value(data_len_buf.data(), 8));

        if(data_len_crc == crc){
            // 4 to account for header and footer
            next_offset = offset + data_size + header_size + footer_size;
            record_offsets.push_back(offset + header_size);
            record_sizes.push_back(data_size + footer_size);
            qWarning() << "Found record at " << offset << " with size: " << data_size;
        }else{
            qWarning() << "Incorrect CRC, corrupt file?";
            break;
        }

        offset = next_offset;
    }
}

void TFRecordImporter::process_example(tensorflow::Example &example){
    // Image metadata
    auto map = example.features().feature();
    int image_height = map["image/height"].int64_list().value(0);
    int image_width = map["image/width"].int64_list().value(0);
    std::string image_filename = map["image/filename"].bytes_list().value(0);
    std::string source_id = map["image/source_id"].bytes_list().value(0);
    std::string ext = map["image/format"].bytes_list().value(0);
    std::string image_data = map["image/encoded"].bytes_list().value(0);

    cv::Mat cv_buffer = cv::Mat(1,
                                static_cast<int>(image_data.size()),
                                CV_8UC1,
                                const_cast<char*>(image_data.data()));

    qDebug() << "Performing image integrity check";
    cv::Mat image = cv::imdecode(cv_buffer, cv::IMREAD_UNCHANGED);
    if(image_height != image.rows){
        qWarning() << "Ignoring example: decoded image height does not match record height";
        return;
    }

    if(image_width != image.cols){
        qWarning() << "Ignoring record: decoded image height does not match record height";
        return;
    }

    int n_boxes = map["image/object/bbox/xmin"].float_list().value_size();
    qDebug() << "Image countains: " << n_boxes << " bounding boxes";

    if(n_boxes == 0 && !import_unlabelled){
        return;
    }

    project->addAsset(QString::fromStdString(image_filename));

    for(int i=0; i < n_boxes; ++i){

        auto xmin = map["image/object/bbox/xmin"].float_list().value(i);
        auto xmax = map["image/object/bbox/xmax"].float_list().value(i);
        auto ymin = map["image/object/bbox/ymin"].float_list().value(i);
        auto ymax = map["image/object/bbox/ymax"].float_list().value(i);
        auto class_name = map["image/object/class/text"].bytes_list().value(i);
        auto class_id = map["image/object/class/label"].int64_list().value(i);

        BoundingBox box;
        box.classid = class_id;
        box.classname = QString::fromStdString(class_name);
        box.rect = QRect(QPoint(xmin*image_width, ymin*image_height),
                         QPoint(xmax*image_width, ymax*image_height));

        qDebug() << printBoundingBox(box);
        project->addLabel(QString::fromStdString(image_filename), box);
    }


}

void TFRecordImporter::import_records(QString filename_mask){

    QFileInfo filemask(filename_mask);
    auto dir_name = filemask.absoluteDir().absolutePath();

    QDirIterator it(dir_name,
                    {filemask.baseName()},
                    QDir::Files);
    QStringList record_files;
    while (it.hasNext()) {
        auto record = QDir(it.next()).canonicalPath();
        record_files.append(record);
        qDebug() << "Found : " << record;
    }

    record_files.removeDuplicates();
    record_files.sort();

    for(auto &record : record_files){
        import(record);
    }
}

void TFRecordImporter::import(QString filename){

    qDebug() << "Processing record :" << filename;

    QFile f(filename);

    f.open(QIODevice::ReadOnly);
    auto buffer = f.readAll();

    qDebug() << "File size:" << buffer.size();

    std::vector<size_t> record_offsets;
    std::vector<size_t> record_sizes;
    find_example_boundaries(buffer, record_offsets, record_sizes);

    qDebug() << "Found : " << record_offsets.size() << " examples in file.";

    std::vector<tensorflow::Example> examples;
    for(size_t i=0; i < record_offsets.size(); i++){
        int curr_offset = static_cast<int>(record_offsets.at(i));
        int record_length = static_cast<int>(record_sizes.at(i));
        QByteArray example_buffer = buffer.mid(curr_offset, record_length);

        tensorflow::Example example;
        example.ParseFromArray(example_buffer.data(), example_buffer.size());
        process_example(example);
    }


}
