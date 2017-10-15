#ifndef LABELPROJECT_H
#define LABELPROJECT_H

#include <QObject>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>

#include <QDebug>
#include <QMessageBox>

class LabelProject : public QObject
{
    Q_OBJECT
public:
    explicit LabelProject(QObject *parent = nullptr);
    ~LabelProject();
    bool loadDatabase(QString fileName);
    bool createDatabase(QString fileName);

    //bool getImageList(std::vector<int> &images);
    //bool getClassList(std::vector<int> &classes);

    //int addImage(QString fileName);
    //int addClass(QString fileName);
    //int addLabel(int imageId, int classId, BoundingBox bbox);

    //QString getClassName(int classId);
    //QString getImagePath(int imageId);

signals:

public slots:


private:
    QSqlDatabase db;
    bool checkDatabase();

};

#endif // LABELPROJECT_H
