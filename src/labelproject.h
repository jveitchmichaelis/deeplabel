#ifndef LABELPROJECT_H
#define LABELPROJECT_H

#include <QObject>
#include <QDir>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>

#include <QDebug>
#include <QMessageBox>

#include <boundingbox.h>

class LabelProject : public QObject
{
    Q_OBJECT
public:
    explicit LabelProject(QObject *parent = nullptr);
    ~LabelProject();
    bool loadDatabase(QString fileName);
    bool createDatabase(QString fileName);

    bool addClass(QString className);
    bool getClassList(QList<QString> &classes);
    bool removeClass(QString className);

    int addImageFolder(QString path);
    bool addImage(QString fileName);
    bool getImageList(QList<QString> &images);
    bool removeImage(QString fileName);

    bool addLabel(QString fileName, BoundingBox bbox);
    bool getLabels(QString fileName, QList<BoundingBox> &bboxes);
    bool removeLabel(QString fileName, BoundingBox bbox);

    bool setOccluded(QString fileName, BoundingBox bbox, int occluded);

    //QString getClassName(int classId);
    int getImageId(QString fileName);
    int getClassId(QString className);

signals:

public slots:


private:
    QSqlDatabase db;
    bool checkDatabase();

};

#endif // LABELPROJECT_H
