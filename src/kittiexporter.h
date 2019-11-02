#ifndef KITTIEXPORTER_H
#define KITTIEXPORTER_H

#include <baseexporter.h>

class KittiExporter : public BaseExporter
{
    Q_OBJECT
public:
    explicit KittiExporter(LabelProject *project, QObject *parent = nullptr) : BaseExporter(project, parent){}

public slots:
    void process();
    bool setOutputFolder(QString folder);

private:
    void appendLabel(QString file, QList<BoundingBox> labels, double scale_x=1, double scale_y=1);
    int processSet(QString folder, QList<QString> images, int i);
};

#endif // KITTIEXPORTER_H
