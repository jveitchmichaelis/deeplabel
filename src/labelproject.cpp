#include "labelproject.h"

LabelProject::LabelProject(QObject *parent) : QObject(parent)
{

}

bool LabelProject::loadDatabase(QString fileName)
{
    /*!
     * Load and check an existing database file. If the database is valid, returns true, otherwise
     * if an error occured returns false.
     */

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(fileName);

    if (!db.open()) {
        QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
            QObject::tr("Unable to establish a database connection.\n"
                        "This example needs SQLite support. Please read "
                        "the Qt SQL driver documentation for information how "
                        "to build it.\n\n"
                        "Click Cancel to exit."), QMessageBox::Cancel);
        return false;
    }
    else if ( QDir(fileName).exists() ){
        return checkDatabase();
    }else{
        return true;
    }

}

bool LabelProject::checkDatabase(){

    /*!
     * Check that the newly loaded database contains the correct tables/fields. Returns
     * true if the database is valid, false if not.
     */

    QSqlRecord image_record = db.record("images");
    QSqlRecord label_record = db.record("labels");
    QSqlRecord class_record = db.record("classes");

    if(image_record.isEmpty()) return false;
    if(label_record.isEmpty()) return false;
    if(class_record.isEmpty()) return false;

    if(!image_record.contains("image_id")) return false;
    if(!image_record.contains("input_image_path")) return false;

    if(!label_record.contains("image_id")) return false;
    if(!label_record.contains("label_id")) return false;
    if(!label_record.contains("class_id")) return false;
    if(!label_record.contains("x")) return false;
    if(!label_record.contains("y")) return false;
    if(!label_record.contains("width")) return false;
    if(!label_record.contains("height")) return false;

    if(!class_record.contains("class_id")) return false;
    if(!class_record.contains("class_name")) return false;

    return true;
}

bool LabelProject::createDatabase(QString fileName)
{
    /*!
     * Create a new labelling database at the given location. Returns true if the database
     * was created successfully.
     */

    bool res = loadDatabase(fileName);

    if(res){
        QSqlQuery query(db);

        /*
         * The label database is a simple relational db - image locations, classes
         * and label bounding boxes.
         */

        res = query.exec("create table images (image_id int primary key, "
                   "input_image_path varchar(256))");
        res &= query.exec("create table classes (class_id int primary key, "
                   "class_name varchar(32))");
        res &= query.exec("create table labels (label_id int primary key, "
                   "image_id int, class_id int, x int, y int, width int, height int)");

        if(!res){
            qDebug() << query.lastError();
        }

        return checkDatabase();
    }

    return false;
}

LabelProject::~LabelProject()
{
    /*!
     * Destructor, closes the database if it's open.
     */

    if(db.open()){
        db.close();
    }
}
