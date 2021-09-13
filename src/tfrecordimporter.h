#ifndef TFRECORDIMPORTER_H
#define TFRECORDIMPORTER_H

#include "baseimporter.h"

#include <proto/example.pb.h>
#include <proto/feature.pb.h>
#include <crc32.h>
#include <fstream>

class TFRecordImporter : public BaseImporter
{
public:
    using BaseImporter::import;

    explicit TFRecordImporter(LabelProject *project, QObject *parent = nullptr) : BaseImporter(parent){
        this->project = project;
    }

     void import_records(QString filename_mask);
     void import(QString filename);

private:
    void find_example_boundaries(QByteArray buffer,
                                 std::vector<size_t> &record_offsets,
                                 std::vector<size_t> &record_sizes);
    void process_example(tensorflow::Example &example);
};

#endif // TFRECORDIMPORTER_H
