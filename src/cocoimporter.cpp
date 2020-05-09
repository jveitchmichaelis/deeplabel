#include "cocoimporter.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <boundingbox.h>

void CocoImporter::import(QString annotation_file){
    // Load Json
    qDebug() << "Loading JSON";
    QFile loadFile(annotation_file);
    loadFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray json_data = loadFile.readAll();
    auto json = QJsonDocument::fromJson(json_data);

    // Load classes (categories)
    auto categories = json["categories"];
    if(categories == QJsonValue::Undefined){
        qDebug() << "Categories not found";
        return;
    }

    QMap<int, QString> classes;
    QJsonValue item;
    foreach(item, categories.toArray()){

        auto item_name = item["name"];
        if(item_name == QJsonValue::Undefined || !item_name.isString())
            continue;

        auto item_id = item["id"];
        if(item_id == QJsonValue::Undefined || !item_id.isDouble())
            continue;

        classes.insert(item_id.toInt(), item_name.toString());
        qDebug() << "added: " << item_id.toInt() << " " << item_name.toString();

        project->addClass(item_name.toString());

    }

    // Load images
    auto images = json["images"];
    if(images == QJsonValue::Undefined){
        qDebug() << "No images found";
        return;
    }

    QJsonValue image;
    QList<QString> image_list;
    QList<QList<BoundingBox>> label_list;
    QMap<int, int> image_index;
    QMap<int, QString> image_map;
    int i=0;
    foreach(image, images.toArray()){
        auto file_name = image["file_name"];
        if(file_name == QJsonValue::Undefined || !file_name.isString())
            continue;

        auto file_id = image["id"];
        if(file_id == QJsonValue::Undefined || !file_id.isDouble())
            continue;

        auto abs_file_name = QFileInfo(annotation_file).absoluteDir().filePath(file_name.toString());
        image_map.insert(file_id.toInt(), abs_file_name);

        image_list.append(abs_file_name);
        label_list.append(QList<BoundingBox> {});
        image_index.insert(file_id.toInt(), i++);
    }

    // Load labels
    auto annotations = json["annotations"];
    if(annotations == QJsonValue::Undefined){
        qDebug() << "No annotations found";
        return;
    }

    QJsonValue annotation;

    foreach(annotation, annotations.toArray()){

        auto image_id = annotation["image_id"];
        if(image_id == QJsonValue::Undefined || !image_id.isDouble())
            continue;

        auto category_id = annotation["category_id"];
        if(category_id == QJsonValue::Undefined || !category_id.isDouble())
            continue;

        auto image_filename = image_map[image_id.toInt()];

        auto bbox = annotation["bbox"];
        if(bbox == QJsonValue::Undefined || !bbox.isArray()){
            qDebug() << "No bounding box found";
            continue;
        }

        auto bbox_array = bbox.toArray();

        if(bbox_array.size() != 4)
            continue;

        int x = bbox_array[0].toInt();
        int y = bbox_array[1].toInt();
        int w = bbox_array[2].toInt();
        int h = bbox_array[3].toInt();

        BoundingBox new_bbox;
        new_bbox.rect.setX(x);
        new_bbox.rect.setY(y);
        new_bbox.rect.setWidth(w);
        new_bbox.rect.setHeight(h);
        new_bbox.classname = classes[category_id.toInt()];

        label_list[image_index[image_id.toInt()]].push_back(new_bbox);
    }

    project->addLabelledAssets(image_list, label_list);

    return;
}
