#ifndef DETECTOROPENCV_H
#define DETECTOROPENCV_H

#include<iostream>
#include<fstream>
#include<string>
#include<vector>

#include<opencv2/opencv.hpp>
#include <opencv2/core/ocl.hpp>
#include<opencv2/dnn.hpp>

#include<boundingbox.h>
#include<imagedisplay.h>

typedef enum {
    FRAMEWORK_TENSORFLOW,
    FRAMEWORK_DARKNET
} model_framework;

class DetectorOpenCV
{

public:
    DetectorOpenCV();

    void setImageSize(int width, int height);

    void loadNetwork(std::string names_file, std::string cfg_file, std::string model_file);

    void annotateImage(cv::Mat &image,
                       std::vector<BoundingBox> boxes,
                       cv::Scalar colour = cv::Scalar(0,0,255),
                       cv::Scalar font_colour = cv::Scalar(255,255,255));

    std::vector<BoundingBox> infer(cv::Mat image);
    std::vector<BoundingBox> inferDarknet(cv::Mat image);
    std::vector<BoundingBox> inferTensorflow(cv::Mat image);

    void setFramework(model_framework framework){this->framework = framework;}
    void setConfidenceThreshold(double thresh){confThreshold = std::max(0.0, thresh);}
    void setNMSThreshold(double thresh){nmsThreshold = std::max(0.0, thresh);}
    void setConvertGrayscale(bool convert){convert_grayscale = convert;}
    void setConvertDepth(bool convert){convert_depth = convert;}
    double getConfidenceThreshold(void){ return confThreshold;}
    double getNMSThreshold(void){ return nmsThreshold;}
    void setTarget(int target);
    void setChannels(int channels);
    int getChannels(void){return input_channels;}

private:

    void postProcess(cv::Mat& frame, const std::vector<cv::Mat>& outs, std::vector<BoundingBox> &filtered_outputs);
    void readNamesFile(std::string class_file = "coco.names");
    void getOutputClassNames(void);

    bool convert_grayscale = true;
    bool convert_depth = true;
    double processing_time;
    double confThreshold = 0.5; // Confidence threshold
    double nmsThreshold = 0.4;  // Non-maximum suppression threshold
    int input_width = 416;        // Width of network's input image
    int input_height = 416;       // Height of network's input image
    int input_channels = 3;
    int preferable_target = cv::dnn::DNN_TARGET_OPENCL;
    model_framework framework = FRAMEWORK_DARKNET;

    std::vector<std::string> class_names;
    std::vector<std::string> output_names;
    cv::dnn::Net net;
    void postProcessTensorflow(cv::Mat &frame, const std::vector<cv::Mat> &outputs, std::vector<BoundingBox> &filtered_boxes);
};

#endif // DETECTOROPENCV_H
