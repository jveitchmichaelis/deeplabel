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

    class_names.clear();

    // Load names of classes
    std::ifstream ifs(class_file.c_str());
    std::string line;
    int i=0;
    while (std::getline(ifs, line)){
        QString cleaned = QString::fromStdString(line).simplified().toLower();
        class_names.push_back(cleaned.toStdString());
        std::cout << "Added detection class: " << i++ << " " << class_names.back() << std::endl;
    }
}

void DetectorOpenCV::setChannels(int channels){
    input_channels = channels;
}

void DetectorOpenCV::setTarget(int target){
    preferable_target = target;

    if(preferable_target == cv::dnn::DNN_TARGET_OPENCL){
        // Check for GPU
        cv::ocl::Context context;

        if(!cv::ocl::haveOpenCL()){
            std::cout << "OpenCL is not available. Falling back to CPU" << std::endl;
            preferable_target = cv::dnn::DNN_TARGET_CPU;
        }

        // Attempt to use a GPU
        if(context.create(cv::ocl::Device::TYPE_GPU)){
            std::cout << "Found OpenCL capable GPU - we're going to use it!" << std::endl;
            cv::ocl::Device(context.device(1));
        }
    }
#ifdef WITH_CUDA
    else if(preferable_target == cv::dnn::DNN_TARGET_CUDA || preferable_target == cv::dnn::DNN_TARGET_CUDA_FP16){
        // Check for GPU
        auto devinfo = cv::cuda::DeviceInfo();
        if (!devinfo.isCompatible()){
            qDebug() << "OpenCL is not available. Falling back to CPU";
            preferable_target = cv::dnn::DNN_TARGET_CPU;
        }else{
            qDebug() << "NVIDIA GPU detected.";
        }
    }
#endif
}

void DetectorOpenCV::loadNetwork(std::string names_file, std::string cfg_file, std::string model_file){
    // Load the names
    readNamesFile(names_file);

    // Infer network type automatically
    net = cv::dnn::readNet(model_file, cfg_file);

    // Should default to DNN_BACKEND_OPENCV (otherwise Intel inference engine)
    if(preferable_target == cv::dnn::DNN_TARGET_CUDA || preferable_target == cv::dnn::DNN_TARGET_CUDA_FP16){
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        qDebug() << "Set preferable backend and target to CUDA";
    }else{
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    }

    net.setPreferableTarget(preferable_target);

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

void DetectorOpenCV::postProcessTensorflow(cv::Mat& frame, const std::vector<cv::Mat>& outputs, std::vector<BoundingBox> &filtered_boxes){
    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    auto detections = outputs.at(0);
    const int numDetections = detections.size[2];

    std::cout << "Outputs_size: " << detections.total() << std::endl;
    std::cout << "Number of detections: " << numDetections << std::endl;

    // batch id, class id, confidence, bbox (x4)
    detections = detections.reshape(1, static_cast<int>(detections.total()) / 7);

    // There are top-k (= 100 typical) detections, most of which should have
    // more or less zero confidence.
    for (int i = 0; i < numDetections; ++i)
    {
        float confidence = detections.at<float>(i, 2);

        if (confidence > confThreshold)
        {

            // Extract the bounding box
            int classId = static_cast<int>(detections.at<float>(i, 1));
            int left = static_cast<int>(frame.cols * detections.at<float>(i, 3));
            int top = static_cast<int>(frame.rows * detections.at<float>(i, 4));
            int right = static_cast<int>(frame.cols * detections.at<float>(i, 5));
            int bottom = static_cast<int>(frame.rows * detections.at<float>(i, 6));

            BoundingBox bbox;
            bbox.rect.setLeft(std::max(0, std::min(left, frame.cols - 1)));
            bbox.rect.setTop(std::max(0, std::min(top, frame.rows - 1)));
            bbox.rect.setRight(std::max(0, std::min(right, frame.cols - 1)));
            bbox.rect.setBottom(std::max(0, std::min(bottom, frame.rows - 1)));
            bbox.confidence = static_cast<double>(confidence);
            bbox.classid = classId;
            bbox.classname = QString::fromStdString(class_names.at(static_cast<size_t>(bbox.classid)));

            std::cout << "Found (" << bbox.classid << ") " << bbox.classname.toStdString()
                      << " at" << " (" << bbox.rect.center().x() << ", " << bbox.rect.center().y()
                      << "), conf: " << bbox.confidence
                      << ", size (" << bbox.rect.width() << "x" << bbox.rect.height() << ")"
                      << std::endl;

            filtered_boxes.push_back(bbox);
        }else{
            //
        }
    }
}

void DetectorOpenCV::postProcess(cv::Mat& frame, const std::vector<cv::Mat>& outputs, std::vector<BoundingBox> &filtered_boxes)
{
    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    // Debug: this should be three because there are three scales that Yolo searches over
    //std::cout << "Outputs: " << outputs.size() << std::endl;

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

    std::vector<BoundingBox> detections;

    // Assume we have an alpha image if 4 channels
    if(image.channels() == 4){
        cv::cvtColor(image, image, cv::COLOR_BGRA2BGR);
    }

    if(convert_depth && image.elemSize() == 2){
        double minval, maxval;
        cv::minMaxIdx(image, &minval, &maxval);

        double range = maxval-minval;
        double scale_factor = 255.0/range;

        image.convertTo(image, CV_32FC1);
        image -= minval;
        image *= scale_factor;
        image.convertTo(image, CV_8UC1);
    }

    if(convert_grayscale && image.channels() == 1){
        cv::cvtColor(image, image, cv::COLOR_GRAY2RGB);
    }

    if(image.channels() != input_channels){
        std::cout << "Input channel mismatch. Expecting "
                 << input_channels
                 << " channels but image has " << image.channels()
                 << " channels.";

        return detections;
    }

    if(framework == FRAMEWORK_TENSORFLOW){
        detections = inferTensorflow(image);
    }else if(framework == FRAMEWORK_DARKNET){
        detections =  inferDarknet(image);
    }

    return detections;

}

std::vector<BoundingBox> DetectorOpenCV::inferTensorflow(cv::Mat image){

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

        auto input_size = cv::Size(image.cols, image.rows);

        bool swap_rb = false; // BGR->RGB?
        bool crop = false; // Use the entire image

        // No normalising! The model will handle it.
        auto blob = cv::dnn::blobFromImage(image, 1.0, input_size, mean, swap_rb, crop);

        //Sets the input to the network
        net.setInput(blob);

        // Runs the forward pass to get output of the output layers
        std::vector<cv::Mat> outputs;
        net.forward(outputs, output_names);

        postProcessTensorflow(image, outputs, results);

        std::vector<double> layersTimes;
        double freq = cv::getTickFrequency() / 1000;
        processing_time = net.getPerfProfile(layersTimes) / freq;

        std::cout << "Processed in: " << processing_time << std::endl;

        return results;
}

std::vector<BoundingBox> DetectorOpenCV::inferDarknet(cv::Mat image){

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
