#ifndef BASEIMPORTER_H
#define BASEIMPORTER_H

#include <QObject>
#include <QFile>
#include <labelproject.h>
#include <boundingbox.h>

class BaseImporter : public QObject
{
    Q_OBJECT

public:
    explicit BaseImporter(QObject *parent = nullptr);
    void import(){};

    void setRootFolder(QString folder);
    void setImportUnlabelled(bool import);
signals:

protected:
    bool import_unlabelled = false;
    QString label_root;
    LabelProject *project;

    void addAsset(QString image_path, QList<BoundingBox> boxes);
    QList<QString> readLines(QString path);

public slots:

};

#endif // BASEIMPORTER_H
