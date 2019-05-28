#include "detectoropencv.h"

DetectorOpenCV::DetectorOpenCV()
{

}

void DetectorOpenCV::setImageSize(int width, int height){
    if(width > 0 && height > 0){
        input_width = width;
        input_height = height;
    }
}

void DetectorOpenCV::readNamesFile(std::string class_file){

    // Load names of classes
    std::ifstream ifs(class_file.c_str());
    std::string line;
    while (std::getline(ifs, line)){
        class_names.push_back(line);
        std::cout << "Added detection class: " << class_names.back() << std::endl;
    }
}

void DetectorOpenCV::loadDarknet(std::string names_file, std::string cfg_file, std::string model_file){
    // Load the network
    readNamesFile(names_file);

    net = cv::dnn::readNetFromDarknet(cfg_file, model_file);
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    getOutputClassNames();
}

void DetectorOpenCV::getOutputClassNames()
{
    output_names.clear();

    //Get the indices of the output layers, i.e. the layers with unconnected outputs
    std::vector<int> outLayers = net.getUnconnectedOutLayers();

    //get the names of all the layers in the network
    std::vector<std::string> layersNames = net.getLayerNames();

    // Get the names of the output layers in names
    output_names.resize(outLayers.size());

    for (size_t i = 0; i < outLayers.size(); ++i){
        output_names.at(i) = layersNames.at(static_cast<size_t>(outLayers[i]) - 1);
    }

}

void DetectorOpenCV::postProcess(cv::Mat& frame, const std::vector<cv::Mat>& outputs, std::vector<BoundingBox> &filtered_boxes)
{
    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    // Debug: this should be three because there are three scales that Yolo searches over
    // std::cout << "Outputs: " << outputs.size() << std::endl;

    for (size_t i = 0; i < outputs.size(); ++i)
    {
        // Scan through all the bounding boxes output from the network and keep only the
        // ones with high confidence scores. Assign the box's class label as the class
        // with the highest score for the box.
        float* data = reinterpret_cast<float*>(outputs[i].data);
        for (int j = 0; j < outputs[i].rows; ++j, data += outputs[i].cols)
        {
            cv::Mat scores = outputs[i].row(j).colRange(5, outputs[i].cols);
            cv::Point classIdPoint;
            double confidence;
            // Get the value and location of the maximum score
            minMaxLoc(scores, nullptr, &confidence, nullptr, &classIdPoint);

            if (confidence > 0)
            {

                // Output is a percentage of the frame width/height
                // so it doesn't matter that we're transforming boxes
                // between the resized and full-size image.
                int centerX = static_cast<int>(data[0] * frame.cols);
                int centerY = static_cast<int>(data[1] * frame.rows);
                int width = static_cast<int>(data[2] * frame.cols);
                int height = static_cast<int>(data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                classIds.push_back(classIdPoint.x);
                confidences.push_back(static_cast<float>(confidence));
                boxes.push_back(cv::Rect(left, top, width, height));

            }else{
                if(confidence == 0.0)
                    continue;

                std::cout << "Detected "
                          << class_names.at(static_cast<size_t>(classIdPoint.x))
                          << " with low confidence: " << confidence << std::endl;

            }
        }
    }


    std::vector<int> indices;

    // Perform non maximum suppression to eliminate redundant overlapping boxes with
    // lower confidences

    // We set the confidence threshold to zero here, we'll filter the boxes out later.
    // This lets us provide some feedback to the user if their threshold is too high.
    cv::dnn::NMSBoxes(boxes, confidences, static_cast<float>(confThreshold), static_cast<float>(nmsThreshold), indices);

    for (size_t i = 0; i < indices.size(); ++i)
    {
        auto idx = static_cast<size_t>(indices.at(i));

        BoundingBox box;
        cv::Rect rect = boxes.at(idx);

        box.confidence = static_cast<double>(confidences.at(idx));
        box.classid = classIds.at(idx);
        box.classname = QString::fromStdString(class_names.at(static_cast<size_t>(box.classid)));

        // Darknet predicts box centres and half-width/height, so the
        // box can go outside the image.  Clamp to the image size:
        QPoint top_left = {std::max(0, rect.x), std::max(0, rect.y)};
        QPoint bottom_right = top_left + QPoint({rect.width, rect.height});

        bottom_right.setX(std::min(bottom_right.x(), frame.cols));
        bottom_right.setY(std::min(bottom_right.y(), frame.rows));

        box.rect.setBottomRight(bottom_right);
        box.rect.setTopLeft(top_left);

        std::cout << "Found " << box.classname.toStdString()
                  << " at" << " (" << box.rect.center().x() << ", " << box.rect.center().y()
                  << "), conf: " << box.confidence
                  << ", size (" << box.rect.width() << "x" << box.rect.height() << ")"
                  << std::endl;

        filtered_boxes.push_back(box);

    }

    return;
}

std::vector<BoundingBox> DetectorOpenCV::infer(cv::Mat image){

        std::vector<BoundingBox> results;

        auto mean = cv::Scalar(0,0,0);
        if(image.channels() == 1){
            mean = cv::Scalar(0);
        }

        // Check for 16-bit
        double scale_factor = 1/255.0;
        if(image.elemSize() == 2){
            scale_factor = 1/65535.0;
        }

        auto input_size = cv::Size(input_width, input_height);

        bool swap_rb = true; // BGR->RGB?
        bool crop = false; // Use the entire image
        auto blob = cv::dnn::blobFromImage(image, scale_factor, input_size, mean, swap_rb, crop);

        //Sets the input to the network
        net.setInput(blob);

        // Runs the forward pass to get output of the output layers
        std::vector<cv::Mat> outputs;
        net.forward(outputs, output_names);

        // Put efficiency information. The function getPerfProfile returns the
        // overall time for inference(t) and the timings for each of the layers(in layersTimes)
        std::vector<double> layersTimes;
        double freq = cv::getTickFrequency() / 1000;
        processing_time = net.getPerfProfile(layersTimes) / freq;

        std::cout << "Processed in: " << processing_time << std::endl;

        // Remove the bounding boxes with low confidence
        postProcess(image, outputs, results);

        return results;
}

void DetectorOpenCV::annotateImage(cv::Mat &frame, std::vector<BoundingBox> boxes, cv::Scalar colour, cv::Scalar font_colour){
    for(auto &box : boxes){
        //Draw a rectangle displaying the bounding box

        auto top_left = cv::Point(box.rect.left(), box.rect.top());

        cv::rectangle(frame, top_left,
                             cv::Point(box.rect.right(), box.rect.bottom()),
                             colour);

        //Get the label for the class name and its confidence
        std::string label = cv::format("%.2f", box.confidence);

        //Display the label at the top of the bounding box
        int baseLine;
        cv::Size labelSize = getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        cv::putText(frame, label, top_left, cv::FONT_HERSHEY_SIMPLEX, 0.5, font_colour);
    }
}
