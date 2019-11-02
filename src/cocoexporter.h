#ifndef COCOEXPORTER_H
#define COCOEXPORTER_H

#include <baseexporter.h>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

class CocoExporter : public BaseExporter
{
    Q_OBJECT
public:
    explicit CocoExporter(LabelProject *project, QObject *parent = nullptr) : BaseExporter(project, parent){}

public slots:
    void process();

private:

    bool processImages(const QString folder, const QString filename, const QList<QString> images);
};

#endif // COCOEXPORTER_H
