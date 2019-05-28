#ifndef DETECTOROPENCV_H
#define DETECTOROPENCV_H

#include<iostream>
#include<fstream>
#include<string>
#include<vector>

#include<opencv2/opencv.hpp>
#include<opencv2/dnn.hpp>

#include<boundingbox.h>

class DetectorOpenCV
{

public:
    DetectorOpenCV();

    void setImageSize(int width, int height);
    void loadDarknet(std::string names_file, std::string cfg_file, std::string model_file);
    void annotateImage(cv::Mat &image,
                       std::vector<BoundingBox> boxes,
                       cv::Scalar colour = cv::Scalar(0,0,255),
                       cv::Scalar font_colour = cv::Scalar(255,255,255));
    std::vector<BoundingBox> infer(cv::Mat image);
    void setConfidenceThreshold(double thresh){confThreshold = std::max(0.0, thresh);}
    double getConfidenceThreshold(void){ return confThreshold; }
private:

    void postProcess(cv::Mat& frame, const std::vector<cv::Mat>& outs, std::vector<BoundingBox> &filtered_outputs);
    void readNamesFile(std::string class_file = "coco.names");
    void getOutputClassNames(void);


    double processing_time;
    double confThreshold = 0.5; // Confidence threshold
    double nmsThreshold = 0.4;  // Non-maximum suppression threshold
    int input_width = 416;        // Width of network's input image
    int input_height = 416;       // Height of network's input image

    std::vector<std::string> class_names;
    std::vector<std::string> output_names;
    cv::dnn::Net net;
};

#endif // DETECTOROPENCV_H
