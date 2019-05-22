#ifndef LABELPROJECT_H
#define LABELPROJECT_H

#include <QObject>
#include <QDir>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QMutex>
#include <QMutexLocker>

#include <QDebug>
#include <QMessageBox>

#include <boundingbox.h>
#include <opencv2/opencv.hpp>

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
    bool classInDB(QString classname);

    int addImageFolder(QString path);
    bool addAsset(QString fileName);
    bool addVideo(QString fileName, QString outputFolder);
    bool getImageList(QList<QString> &images);
    bool removeImage(QString fileName);
    bool imageInDB(QString fileName);

    bool addLabel(QString fileName, BoundingBox bbox);
    bool getLabels(QString fileName, QList<BoundingBox> &bboxes);
    bool getLabels(int imageId, QList<BoundingBox> &bboxes);
    bool removeLabel(QString fileName, BoundingBox bbox);
    bool removeLabels(QString fileName);
    bool updateLabel(QString fileName, BoundingBox bbox);

    bool setOccluded(QString fileName, BoundingBox bbox, int occluded);

    int getNextUnlabelled(QString fileName);

    //QString getClassName(int classId);
    int getImageId(QString fileName);
    int getClassId(QString className);

signals:

public slots:


private:
    QSqlDatabase db;
    QMutex mutex;
    bool checkDatabase();

};

#endif // LABELPROJECT_H
