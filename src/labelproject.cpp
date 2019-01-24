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

    if(db.isOpen()){
        db.close();
    }

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(fileName);

    QFileInfo check_file(fileName);

    if (!db.open()) {
        QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
            QObject::tr("Unable to establish a database connection.\n"
                        "This example needs SQLite support. Please read "
                        "the Qt SQL driver documentation for information how "
                        "to build it.\n\n"
                        "Click Cancel to exit."), QMessageBox::Cancel);
        return false;
    }
    else if ( check_file.exists() && check_file.isFile() ){
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
    if(!image_record.contains("path")) return false;

    if(!label_record.contains("image_id")) return false;
    if(!label_record.contains("label_id")) return false;
    if(!label_record.contains("class_id")) return false;
    if(!label_record.contains("x")) return false;
    if(!label_record.contains("y")) return false;
    if(!label_record.contains("width")) return false;
    if(!label_record.contains("height")) return false;

    if(!class_record.contains("class_id")) return false;
    if(!class_record.contains("name")) return false;

    qDebug() << "Database structure looks good";

    return true;
}

bool LabelProject::createDatabase(QString fileName)
{
    /*!
     * Create a new labelling database at the given absolute location (\a fileName). Returns true if the database
     * was created successfully.
     */

    bool res;

    loadDatabase(fileName);

    QSqlQuery query(db);

    /*
     * The label database is a simple relational db - image locations, classes
     * and label bounding boxes.
     */

    res = query.exec("CREATE table images (image_id INTEGER PRIMARY KEY ASC, "
               "path varchar(256))");
    res &= query.exec("CREATE table classes (class_id INTEGER PRIMARY KEY ASC, "
               "name varchar(32))");
    res &= query.exec("CREATE table labels (label_id INTEGER PRIMARY KEY ASC, "
               "image_id int, class_id int, x int, y int, width int, height int)");

    if(!res){
        qDebug() << "Error: " << query.lastError();
    }

    return checkDatabase();
}

bool LabelProject::getImageList(QList<QString> &images)
{
    /*!
     * Get a list of all images in the database with absolute paths, which is cleared prior to retrieval. Returns false if the database query failed.
     */
    bool res = false;

    QSqlQuery query(db);
    res = query.exec("SELECT path FROM images");

    if(!res){
        qDebug() << "Error: " << query.lastError();
    }else{

        images.clear();

        while (query.next()) {
            QString path = query.value(0).toString();
            // Push absolute file path
            images.push_back(QDir::cleanPath(QDir(db.databaseName()).path() + QDir::separator() + path));
        }
    }

    return res;
}

bool LabelProject::getClassList(QList<QString> &classes)
{
    /*!
     * Get a list of all class names in the database and puts them in (\a classes), which is cleared prior to retrieval. Returns false if the database query failed.
     */
    bool res = false;

    QSqlQuery query(db);
    res = query.exec("SELECT name FROM classes");

    if(!res){
        qDebug() << "Error: " << query.lastError();
    }else{

        classes.clear();

        while (query.next()) {
            QString name = query.value(0).toString();
            classes.push_back(name);
        }
    }

    return res;
}

bool LabelProject::classInDB(QString className){
    bool res = false;
    QSqlQuery query(db);
    query.prepare("SELECT * FROM classes WHERe (name)"
                  "= (:name)");

    query.bindValue(":name", className);
    res = query.exec();

    if(!res){
        qDebug() << "Error: " << query.lastError();
        return false;
    }else{
        return query.next();
    }

    return false;
}

bool LabelProject::imageInDB(QString fileName){
    bool res = false;
    //IF NOT EXISTS(SELECT * FROM images where (path) = (:PATH))

    QSqlQuery query(db);
    query.prepare("SELECT * FROM images WHERe (path)"
                  "= (:path)");

    query.bindValue(":path", QDir(db.databaseName()).relativeFilePath(fileName));
    res = query.exec();

    if(!res){
        qDebug() << "Error: " << query.lastError();
    }else{
        return query.next();
    }

    return false;
}

bool LabelProject::addImage(QString fileName)
{
    /*!
     * Add an image to the database, given an absolute path to the image (\a fileName). Returns false if the file doesn't exist or if the query failed.
     */
    bool res = false;

    if(imageInDB(fileName)){
        qDebug() << "Image exists!";
        return true;
    }
    QFileInfo check_file(fileName);

    if (  check_file.exists() && check_file.isFile() ){
        QSqlQuery query(db);
        query.prepare("INSERT INTO images (path)"
                      "VALUES (:path)");

        query.bindValue(":path", QDir(db.databaseName()).relativeFilePath(fileName));
        res = query.exec();

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }
    }

    return res;
}

int LabelProject::getImageId(QString fileName){
    /*!
     * Get the image ID for an image at an absolute path (\a fileName), returns -1 if not found
     */
    int id = -1;

    QSqlQuery query(db);
    query.prepare("SELECT image_id FROM images WHERE path = ?");

    //Important - we need the relative image path again
    query.bindValue(0, QDir(db.databaseName()).relativeFilePath(fileName));
    bool res = query.exec();

    if(!res){
        qDebug() << "Error: " << query.lastError();
    }else{
        if(query.next()){
            id = query.value(0).toInt();
        }
    }

    return id;
}

int LabelProject::getClassId(QString className){
    /*!
     * Get the class ID for an class name, (\a className), returns -1 if not found
     */
    int id = -1;

    QSqlQuery query(db);
    query.prepare("SELECT class_id FROM classes WHERE name = ?");
    query.bindValue(0, className);
    bool res = query.exec();

    if(!res){
        qDebug() << "Error: " << query.lastError();
    }else{
        if(query.next()){
            id = query.value(0).toInt();
        }
    }

    return id;
}

bool LabelProject::getLabels(QString fileName, QList<BoundingBox> &bboxes){
    bboxes.clear();
    int image_id = getImageId(fileName);
    return getLabels(image_id, bboxes);
}

bool LabelProject::getLabels(int image_id, QList<BoundingBox> &bboxes){
    /*!
     * Get the labels for a given image  (at absolute path \a fileName) and puts them in the provided list: \a bboxes. Note \a bboxes will be emptied.
     * Returns false if the query failed.
     */
    bool res = false;

    if(image_id > 0){
        QSqlQuery query(db);

        query.prepare("SELECT name, x, y, width, height FROM labels"
                      " INNER JOIN classes ON labels.class_id = classes.class_id"
                      " WHERE image_id = ?");
        query.addBindValue(image_id);
        res = query.exec();

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }

        while(query.next()){
            BoundingBox new_bbox;
            auto rec = query.record();

            new_bbox.classname = rec.value(rec.indexOf("name")).toString();
            new_bbox.classid = getClassId(new_bbox.classname);

            new_bbox.rect.setX(rec.value(rec.indexOf("x")).toInt());
            new_bbox.rect.setY(rec.value(rec.indexOf("y")).toInt());
            new_bbox.rect.setWidth(rec.value(rec.indexOf("width")).toInt());
            new_bbox.rect.setHeight(rec.value(rec.indexOf("height")).toInt());

            bboxes.append(new_bbox);
        }
    }

    return res;
}

bool LabelProject::removeLabels(QString fileName){
    /*!
     * Remove all labels from an image given an absolute path to the image (\a fileName).
     */
    int image_id = getImageId(fileName);

    bool res = false;

    if(image_id > 0){
        QSqlQuery query(db);

        query.prepare("DELETE FROM labels WHERE (image_id = :image_id)");
        query.bindValue(":image_id", image_id);

        res = query.exec();

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }

    }

    return res;
}

bool LabelProject::removeLabel(QString fileName, BoundingBox bbox){
    /*!
     * Remove a label given an absolute path  (\a fileName) and the bounding box  (\a bbox). Returns false
     * if the query failed.
     */
    int image_id = getImageId(fileName);
    int class_id = getClassId(bbox.classname);

    bool res = false;

    if(image_id > 0 && class_id > 0){
        QSqlQuery query(db);

        query.prepare("DELETE FROM labels WHERE (image_id = :image_id AND class_id = :class_id"
                      " AND x = :x AND y = :y AND width = :width AND height = :height)");
        query.bindValue(":image_id", image_id);
        query.bindValue(":class_id", class_id);
        query.bindValue(":x", bbox.rect.x());
        query.bindValue(":y", bbox.rect.y());
        query.bindValue(":width", bbox.rect.width());
        query.bindValue(":height", bbox.rect.height());

        res = query.exec();

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }

    }

    return res;
}

bool LabelProject::setOccluded(QString fileName, BoundingBox bbox, int occluded){
    /*!
     * Set a label to be occluded. Returns false
     * if the query failed.
     */
    int image_id = getImageId(fileName);
    int class_id = getClassId(bbox.classname);

    bool res = false;

    if(image_id > 0 && class_id > 0){
        QSqlQuery query(db);

        query.prepare("UPDATE labels SET occluded = :occluded WHERE (image_id = :image_id AND class_id = :class_id"
                      " AND x = :x AND y = :y AND width = :width AND height = :height)");
        query.bindValue(":image_id", image_id);
        query.bindValue(":class_id", class_id);
        query.bindValue(":x", bbox.rect.x());
        query.bindValue(":y", bbox.rect.y());
        query.bindValue(":width", bbox.rect.width());
        query.bindValue(":height", bbox.rect.height());
        query.bindValue(":occluded", occluded);

        res = query.exec();

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }

    }

    return res;
}

bool LabelProject::removeClass(QString className){
    /*!
     * Remove a class given a class name (\a className). Also removes all labels with that class. Returns false
     * if the query failed.
     */
    int class_id = getClassId(className);

    bool res = false;

    if(class_id > 0){
        QSqlQuery query(db);

        // Delete all labels with this class
        query.prepare("DELETE FROM labels WHERE (class_id = :class_id)");
        query.bindValue(":class_id", class_id);

        res = query.exec();

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }

        // Delete the class itself
        query.prepare("DELETE FROM classes WHERE (class_id = :class_id)");
        query.bindValue(":class_id", class_id);

        res = query.exec();

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }

    }

    return res;
}

int LabelProject::getNextUnlabelled(QString fileName){
    int image_id = getImageId(fileName);

    // Lazy - optimise this later.
    QList<QString> images;
    getImageList(images);
    int num_images = images.size();

    QList<BoundingBox> bboxes;
    for(int i=image_id; i < num_images; i++){
        bboxes.clear();
        getLabels(i, bboxes);

        if(bboxes.size() == 0) return i;
    }

    return -1;
}

bool LabelProject::removeImage(QString fileName){

    /*!
     * Remove an image given an absolute path (\a fileName). Also removes all labels for that image. Returns false
     * if the query failed.
     */
    int image_id = getImageId(fileName);

    bool res = false;

    if(image_id > 0){
        QSqlQuery query(db);

        // Delete all labels for this image
        query.prepare("DELETE FROM labels WHERE (image_id = :image_id)");
        query.bindValue(":image_id", image_id);

        res = query.exec();

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }

        // Delete the image itself
        query.prepare("DELETE FROM images WHERE (image_id = :image_id)");
        query.bindValue(":image_id", image_id);

        res = query.exec();

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }

    }

    return res;
}

bool LabelProject::addLabel(QString fileName, BoundingBox bbox)
{
    /*!
     * Add a label given an absolute path (\a fileName) and the bounding box (\a bbox). Returns false
     * if the query failed.
     */

    QMutexLocker locker(&mutex);

    bool res = false;

    int image_id = getImageId(fileName);
    int class_id = getClassId(bbox.classname);

    if(image_id > 0 && class_id > 0){
        QSqlQuery query(db);

        query.prepare("INSERT INTO labels (image_id, class_id, x, y, width, height)"
                      "VALUES (:image_id, :class_id, :x, :y, :width, :height)");
        query.bindValue(":image_id", image_id);
        query.bindValue(":class_id", class_id);
        query.bindValue(":x", bbox.rect.x());
        query.bindValue(":y", bbox.rect.y());
        query.bindValue(":width", bbox.rect.width());
        query.bindValue(":height", bbox.rect.height());
        res = query.exec();

        if(!res){
            qDebug() << "Error: " << query.lastError();
        }

    }

    return res;
}


int LabelProject::addImageFolder(QString path){
    /*!
     * Add all images in a folder given an absolute \a path to the directory. Returns the number of images successfully added.
     */
    QDir dir(path);
    int number_added = 0;

    if(dir.exists()){
        QStringList filters;
        filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.tiff";
        auto image_list = dir.entryInfoList(filters, QDir::Files|QDir::NoDotAndDotDot);

        QFileInfo image_info;

        QSqlDatabase::database().transaction();
        foreach(image_info, image_list){

            QString image_path = image_info.absoluteFilePath();


            bool res = addImage(image_path);

            if(res){
                number_added++;
                qDebug() << "Added image: " << image_path;
            }else{
                qDebug() << "Failed to add image: " << image_path;
            }
        }
        QSqlDatabase::database().commit();
    }

    return number_added;

}

bool LabelProject::addClass(QString className)
{
    /*!
     * Add a class with name \a className. Returns false
     * if the query failed.
     */
    bool res = false;

    if(classInDB(className)){
        qDebug() << "Class exists!";
        return true;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO classes (name)"
                  "VALUES (:name)");
    query.bindValue(":name", className);
    res = query.exec();

    if(!res){
        qDebug() << "Error: " << query.lastError();
    }

    return res;
}

LabelProject::~LabelProject()
{
    /*!
     * Destructor, closes the database if it's open.
     */

    if(db.isOpen()){
        db.close();
    }
}
