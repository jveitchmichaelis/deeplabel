#ifndef VIDEOEXPORTER_H
#define VIDEOEXPORTER_H

#include "baseexporter.h"
#include <unordered_map>

class VideoExporter : public BaseExporter
{
public:
    explicit VideoExporter(LabelProject *project, QObject *parent = nullptr) : BaseExporter(project, parent){}
    void videoConfig(QString filename, QString fourcc_string = "h264", double fps = 10, cv::Size frame_size = {1280,720}, QString colormap="Inferno");
    void process();
    void labelConfig(bool display_names=true, bool label_boxes=true, int box_thickness=1, double font_scale=1);
private:
    QString filename;
    int fourcc;
    int colourmap;
    double fps = 10;
    cv::Size frame_size;
    bool display_names = true;
    bool display_boxes = true;
    bool box_thickness = 1;
    double font_scale = 0.8;

    void normalise(cv::Mat &image_raw, cv::Mat &image_norm);
    static std::unordered_map<std::string ,int> colour_hashmap;
    void convert16(cv::Mat &source, double minval = -1, double maxval = -1);
    void drawBoundingBox(cv::Mat &source, BoundingBox box, double x_scale, double y_scale, int thickness=1);
};

#endif // VIDEOEXPORTER_H
