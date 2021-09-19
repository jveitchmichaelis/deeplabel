#include "videoexporter.h"

std::unordered_map<std::string ,int> VideoExporter::colour_hashmap{
    { "Cividis", cv::COLORMAP_CIVIDIS },
    { "Inferno", cv::COLORMAP_INFERNO },
    { "Magma", cv::COLORMAP_MAGMA },
    { "Hot", cv::COLORMAP_HOT },
    { "Bone", cv::COLORMAP_BONE },
    { "Plasma", cv::COLORMAP_PLASMA },
    { "Jet", cv::COLORMAP_JET },
    { "Rainbow", cv::COLORMAP_RAINBOW },
    { "Ocean", cv::COLORMAP_OCEAN },
    { "Viridis", cv::COLORMAP_VIRIDIS }
};

void VideoExporter::process()
{

    auto out_filename = QDir(output_folder).absoluteFilePath(filename);

    cv::VideoWriter writer(out_filename.toStdString(), fourcc, fps, frame_size);
    QList<QString> images;

    if(export_unlabelled)
        project->getImageList(images);
    else
        project->getLabelledImageList(images);

    if(!writer.isOpened()){
        qCritical() << "Failed to open video writer";
    }

    auto pbar = cliProgressBar();
    double progress = 0;
    int i = 0;

    qInfo() << "Writing video to file:" << out_filename;

    for(auto &abs_image_path : images){
        // Read
        auto image = cv::imread(abs_image_path.toStdString(), cv::IMREAD_UNCHANGED);

        int w, h;
        w = image.cols;
        h = image.rows;

        cv::Mat image_resized;
        cv::resize(image, image_resized, frame_size);

        if(image_resized.elemSize() == 2){
            convert16(image_resized);
        }

        if (image_resized.channels() == 4){
            cv::cvtColor(image_resized, image_resized, cv::COLOR_RGBA2RGB);
        }else if(image_resized.channels() == 1){
            cv::applyColorMap(image_resized, image_resized, colourmap);
        }

        if(this->display_boxes){
            QList<BoundingBox> labels;
            project->getLabels(abs_image_path, labels);

            for(auto &label : labels){
                double x_scale = static_cast<double>(w)/image_resized.cols;
                double y_scale = static_cast<double>(h)/image_resized.rows;
                drawBoundingBox(image_resized, label, x_scale, y_scale, this->box_thickness);

            }
        }

        writer.write(image_resized);

        progress = 100*static_cast<double>(i++)/images.size();
        pbar.update(progress);
        pbar.print();

    }

    writer.release();
}

void VideoExporter::drawBoundingBox(cv::Mat &source, BoundingBox box, double x_scale, double y_scale, int thickness){


    int top_x = box.rect.x()/x_scale;
    int top_y = box.rect.y()/y_scale;
    double font_scale = 0.8;

    auto rect = cv::Rect2i(top_x,
                           top_y,
                           box.rect.width()/x_scale,
                           box.rect.height()/y_scale);

    auto colour_list = QColor::colorNames();
    QColor colour = QColor(colour_list.at(std::max(0, box.classid) % colour_list.size()) );
    cv::Scalar color(colour.red(), colour.green(), colour.blue());

    cv::rectangle(source, rect, color, thickness);

    if(display_names){
        auto label_string = QString("%1").arg(box.classname).toStdString();

        int baseline;
        auto text_size = cv::getTextSize(label_string, cv::FONT_HERSHEY_DUPLEX, font_scale, thickness, &baseline);

        cv::Rect2i label_background(top_x,
                                    top_y,
                                    text_size.width,
                                    text_size.height);
        cv::rectangle(source, label_background, color, -1);

        auto text_colour = cv::Scalar(0,0,0);
        if(colour.red() == 0
            && colour.green() == 0
            && colour.blue() == 0){
            text_colour = {255,255,255};
        }

        cv::putText(source,
                    label_string,
                    {top_x, top_y + text_size.height},
                    cv::FONT_HERSHEY_DUPLEX,
                    this->font_scale,
                    text_colour,
                    thickness);
    }
}

void VideoExporter::convert16(cv::Mat &source, double minval, double maxval){

    if(minval < 0 || maxval < 0){
        cv::minMaxIdx(source, &minval, &maxval);
    }

    double range = maxval-minval;
    double scale_factor = 255.0/range;

    source.convertTo(source, CV_32FC1);
    source -= minval;
    source *= scale_factor;
    source.convertTo(source, CV_8UC1);

    return;
}

void VideoExporter::labelConfig(bool display_names, bool display_boxes, int box_thickness, double font_size){
    this->display_names = display_names;
    this->display_boxes = display_boxes;
    this->box_thickness = box_thickness;
    this->font_scale = font_size;
}

void VideoExporter::videoConfig(QString filename, QString fourcc_string, double fps, cv::Size frame_size, QString colourmap)
{
    this->filename = filename;
    this->fps = fps;
    this->fourcc = cv::VideoWriter::fourcc(fourcc_string.at(0).toLatin1(),
                                           fourcc_string.at(1).toLatin1(),
                                           fourcc_string.at(2).toLatin1(),
                                           fourcc_string.at(3).toLatin1());
    this->frame_size = frame_size;
    this->colourmap = colour_hashmap[colourmap.toStdString()];
}
