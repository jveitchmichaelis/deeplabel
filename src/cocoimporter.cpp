#include "cocoimporter.h"

void CocoImporter::import(QString annotation_file, QString image_folder){
    // Load Json
    qDebug() << "Loading COCO JSON";
    QFile loadFile(annotation_file);
    loadFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray json_data = loadFile.readAll();
    auto json = QJsonDocument::fromJson(json_data);

    // Load classes (categories)
    auto categories = json.object().value("categories");
    if(categories == QJsonValue::Undefined){
        qCritical() << "Categories not found";
        return;
    }

    QMap<int, QString> classes;
    QSet<QString> class_set;
    QJsonValue item;
    foreach(item, categories.toArray()){

        auto item_name = item.toObject().value("name");
        if(item_name == QJsonValue::Undefined || !item_name.isString())
            continue;

        auto item_id = item.toObject().value("id");
        if(item_id == QJsonValue::Undefined || !item_id.isDouble())
            continue;

        classes.insert(item_id.toInt(), item_name.toString());
        qDebug() << "added: " << item_id.toInt() << " " << item_name.toString();

        class_set.insert(item_name.toString());
    }

    for(auto &cls : class_set){
        project->addClass(cls);
    }

    // Load images
    auto images = json.object().value("images");
    if(images == QJsonValue::Undefined){
        qCritical() << "No images found";
        return;
    }

    QJsonValue image;
    QList<QString> image_list;
    QList<QList<BoundingBox>> label_list;
    QMap<int, int> image_index;
    QMap<int, QString> image_map;
    int i=0;
    foreach(image, images.toArray()){
        auto file_name = image.toObject().value("file_name");
        if(file_name == QJsonValue::Undefined || !file_name.isString())
            continue;

        auto file_id = image.toObject().value("id");
        if(file_id == QJsonValue::Undefined || !file_id.isDouble())
            continue;

        auto abs_file_name = QDir(image_folder).absoluteFilePath(file_name.toString());
        image_map.insert(file_id.toInt(), abs_file_name);

        image_list.append(abs_file_name);
        label_list.append(QList<BoundingBox> {});
        image_index.insert(file_id.toInt(), i++);
    }

    // Load labels
    auto annotations = json.object().value("annotations");
    if(annotations == QJsonValue::Undefined){
        qCritical() << "No annotations found";
        return;
    }

    QJsonValue annotation;

    foreach(annotation, annotations.toArray()){

        auto image_id = annotation.toObject().value("image_id");
        if(image_id == QJsonValue::Undefined || !image_id.isDouble())
            continue;

        auto category_id = annotation.toObject().value("category_id");
        if(category_id == QJsonValue::Undefined || !category_id.isDouble())
            continue;

        auto image_filename = image_map[image_id.toInt()];

        auto bbox = annotation.toObject().value("bbox");
        if(bbox == QJsonValue::Undefined || !bbox.isArray()){
            qWarning() << "No bounding box found for" << image_id;
            continue;
        }

        auto bbox_array = bbox.toArray();

        if(bbox_array.size() != 4)
            continue;

        int x = static_cast<int>(bbox_array.at(0).toDouble());
        int y = static_cast<int>(bbox_array.at(1).toDouble());
        int w = static_cast<int>(bbox_array.at(2).toDouble());
        int h = static_cast<int>(bbox_array.at(3).toDouble());

        if(w == 0){
            qWarning() << "Bounding box for" << image_filename << "has zero width";
            continue;
        }

        if(h == 0){
            qWarning() << "Bounding box for" << image_filename << "has zero height";
            continue;
        }

        BoundingBox new_bbox;
        new_bbox.rect.setX(x);
        new_bbox.rect.setY(y);
        new_bbox.rect.setWidth(w);
        new_bbox.rect.setHeight(h);
        int cat_id = static_cast<int>(category_id.toDouble());
        new_bbox.classname = classes[cat_id];
        new_bbox.classid = project->getClassId(new_bbox.classname);

        label_list[image_index[image_id.toInt()]].push_back(new_bbox);
    }

    project->addLabelledAssets(image_list, label_list);

    return;
}
