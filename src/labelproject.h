#ifndef LABELPROJECT_H
#define LABELPROJECT_H

#include <QObject>
#include <QApplication>
#include <QDir>
#include <QDirIterator>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QRegExp>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QProgressDialog>

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
    bool getClassCounts(QMap<int, int> &counts);
    bool removeClass(QString className);
    bool classInDB(QString classname);
    bool addAsset(QString fileName);
    bool addLabelledAssets(QList<QString> images, QList<QList<BoundingBox>> bboxes);
    void addVideo(QString fileName, QString outputFolder, int frame_skip=1);
    bool getImageList(QList<QString> &images, bool relative=false);
    bool removeImage(QString fileName);
    bool imageInDB(QString fileName);

    bool addLabel(QString fileName, BoundingBox bbox);
    bool addLabel(QString fileName, QList<BoundingBox> bbox);
    bool getLabels(QString fileName, QList<BoundingBox> &bboxes);
    bool getLabels(int imageId, QList<BoundingBox> &bboxes);
    bool removeLabel(QString fileName, BoundingBox bbox);
    bool removeLabels(QString fileName);
    bool updateLabel(QString fileName, BoundingBox bbox);

    bool setOccluded(QString fileName, BoundingBox bbox, int occluded);

    int getNextUnlabelled(QString fileName);
    int getNextInstance(QString fileName, QString className);

    QDir getDbFolder(){
            auto db = getDatabase();
            return QDir(db.databaseName());
    }

    int getImageId(QString fileName);
    int getClassId(QString className);
    QString getClassName(int classID);
    void assignThread(QThread* thread);

signals:
    void finished();

    void video_split_finished(QString);
    void load_finished();
    void load_progress(int);

public slots:
    int addImageFolder(QString path);
    void cancelLoad();
    void addFolderRecursive(QString path_filter);
private:
    QMutex mutex;
    bool checkDatabase();
    bool should_cancel;
    QString connection_name;
    QSqlDatabase getDatabase();
};

#endif // LABELPROJECT_H
