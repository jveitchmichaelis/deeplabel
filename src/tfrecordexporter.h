#ifndef TFRECORDEXPORTER_H
#define TFRECORDEXPORTER_H

#include "baseexporter.h"

#include <proto/example.pb.h>
#include <proto/feature.pb.h>
#include <crc32.h>

#include <fstream>

class TFRecordExporter : public BaseExporter
{
public:
    explicit TFRecordExporter(LabelProject *project, QObject *parent = nullptr) : BaseExporter(project, parent){}
    void setNumberShards(int n);
    void generateLabelIds(const QString names_file);
    void process(void);
private:
    bool create_example(tensorflow::Example &example, QString image_path);
    void add_int64_feature(tensorflow::Features &features, std::string key, std::vector<int64> values);
    void add_bytes_feature(tensorflow::Features &features, std::string key, std::vector<void*> values, std::vector<size_t> size);
    void add_bytes_feature(tensorflow::Features &features, std::string key, std::vector<std::string> values);
    void add_float_feature(tensorflow::Features &features, std::string key, std::vector<float> values);
    QMap<int, float> weight_map;
    void create_weights();
    bool processImages(const QString folder, const QString record_prefix, const QList<QString> images, export_image_type split_type);
    int number_shards = 10;
};

#endif // TFRECORDEXPORTER_H
