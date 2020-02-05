#ifndef BASEIMPORTER_H
#define BASEIMPORTER_H

#include <QObject>
#include <QFile>
#include <labelproject.h>

class BaseImporter : public QObject
{
    Q_OBJECT
public:
    explicit BaseImporter(QObject *parent = nullptr);
    virtual void import(QString input_file, QString label_file) = 0;

    void setRootFolder(QString folder);
    void setImportUnlabelled(bool import);
signals:

protected:
    bool import_unlabelled = false;
    QString label_root;
    LabelProject *project;

    void addAsset(QString image_path, QList<BoundingBox> boxes);
public slots:
};

#endif // BASEIMPORTER_H
