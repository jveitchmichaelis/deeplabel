#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionNew_Project, SIGNAL(triggered(bool)), this, SLOT(newProject()));
    connect(ui->actionOpen_Project, SIGNAL(triggered(bool)), this, SLOT(openProject()));
    connect(ui->actionMerge_Project, SIGNAL(triggered(bool)), this, SLOT(mergeProject()));

    connect(ui->actionAdd_video, SIGNAL(triggered(bool)), this, SLOT(addVideo()));
    connect(ui->actionAdd_image, SIGNAL(triggered(bool)), this, SLOT(addImages()));
    connect(ui->actionAdd_image_folder, SIGNAL(triggered(bool)), this, SLOT(addImageFolder()));
    connect(ui->actionAdd_image_folders, SIGNAL(triggered(bool)), this, SLOT(addImageFolders()));

    connect(ui->actionNextImage, SIGNAL(triggered(bool)), this, SLOT(nextImage()));
    connect(ui->actionPreviousImage, SIGNAL(triggered(bool)), this, SLOT(previousImage()));
    connect(ui->actionJump_forward, SIGNAL(triggered(bool)), this, SLOT(jumpForward()));
    connect(ui->actionJump_backward, SIGNAL(triggered(bool)), this, SLOT(jumpBackward()));

    connect(ui->addClassButton, SIGNAL(clicked(bool)), this, SLOT(addClass()));
    connect(ui->newClassText, SIGNAL(editingFinished()), this, SLOT(addClass()));

    connect(ui->actionInit_Tracking, SIGNAL(triggered(bool)), this, SLOT(initTrackers()));
    connect(ui->actionPropagate_Tracking, SIGNAL(triggered(bool)), this, SLOT(updateTrackers()));
    connect(ui->propagateCheckBox, SIGNAL(clicked(bool)), this, SLOT(toggleAutoPropagate(bool)));
    connect(ui->refineTrackingCheckbox, SIGNAL(clicked(bool)), this, SLOT(toggleRefineTracking(bool)));

    connect(ui->nextUnlabelledButton, SIGNAL(clicked(bool)), this, SLOT(nextUnlabelled()));
    connect(ui->nextInstanceButton, SIGNAL(clicked(bool)), this, SLOT(nextInstance()));

    display = new ImageDisplay;
    ui->imageDisplayLayout->addWidget(display);
    currentImage = display->getImageLabel();

    connect(this, SIGNAL(selectedClass(QString)), currentImage, SLOT(setClassname(QString)));
    connect(currentImage, SIGNAL(newLabel(BoundingBox)), this, SLOT(addLabel(BoundingBox)));
    connect(currentImage, SIGNAL(removeLabel(BoundingBox)), this, SLOT(removeLabel(BoundingBox)));
    connect(currentImage, SIGNAL(updateLabel(BoundingBox, BoundingBox)), this, SLOT(updateLabel(BoundingBox, BoundingBox)));
    connect(currentImage, SIGNAL(setCurrentClass(QString)), this, SLOT(setCurrentClass(QString)));

    connect(ui->actionDraw_Tool, SIGNAL(triggered(bool)), currentImage, SLOT(setDrawMode()));
    connect(ui->actionSelect_Tool, SIGNAL(triggered(bool)), currentImage, SLOT(setSelectMode()));
    connect(ui->classComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setCurrentClass(QString)));
    connect(display, SIGNAL(image_loaded()), this, SLOT(updateImageInfo()));

    connect(ui->removeClassButton, SIGNAL(clicked(bool)), this, SLOT(removeClass()));
    connect(ui->removeImageButton, SIGNAL(clicked(bool)), this, SLOT(removeImage()));
    connect(ui->removeImageLabelsButton, SIGNAL(clicked(bool)), this, SLOT(removeImageLabels()));
    connect(ui->actionRemove_labels_forwards, SIGNAL(triggered(bool)), this, SLOT(removeImageLabelsForward()));

    ui->actionDraw_Tool->setChecked(true);

    connect(ui->changeImageButton, SIGNAL(clicked(bool)), this, SLOT(updateDisplay()));
    connect(ui->imageNumberSpinbox, SIGNAL(editingFinished()), this, SLOT(updateDisplay()));

    connect(ui->colourMapCombo, SIGNAL(currentIndexChanged(QString)), display, SLOT(setColourMap(QString)));
    connect(ui->colourMapCheckbox, SIGNAL(clicked(bool)), display, SLOT(toggleColourMap(bool)));

    connect(ui->actionWrap_images, SIGNAL(triggered(bool)), this, SLOT(enableWrap(bool)));
    connect(ui->actionExport, SIGNAL(triggered(bool)), this, SLOT(launchExportDialog()));
    connect(ui->actionImport_Labels, SIGNAL(triggered(bool)), this, SLOT(launchImportDialog()));
    connect(ui->actionRefine_boxes, SIGNAL(triggered(bool)), this, SLOT(refineBoxes()));

    connect(ui->actionSetup_detector, SIGNAL(triggered(bool)), this, SLOT(setupDetector()));
    connect(ui->actionCalculate_histograms, SIGNAL(triggered(bool)), this, SLOT(computeStatistics()));

    auto prev_shortcut = ui->actionPreviousImage->shortcuts();
    prev_shortcut.append(QKeySequence("Left"));
    ui->actionPreviousImage->setShortcuts(prev_shortcut);

    auto next_shortcut = ui->actionNextImage->shortcuts();
    next_shortcut.append(QKeySequence("Right"));
    ui->actionNextImage->setShortcuts(next_shortcut);

    // Override progress bar animation on Windows
#ifdef WIN32
    ui->imageProgressBar->setStyleSheet("QProgressBar::chunk {background-color: #3add36; width: 1px;}");
#endif

    project = new LabelProject;

    settings = new QSettings("DeepLabel", "DeepLabel");

    multitracker = new MultiTrackerCV();
    reinterpret_cast<MultiTrackerCV *>(multitracker)->setTrackerType(CSRT);

    QtAwesome* awesome = new QtAwesome(qApp);
    awesome->initFontAwesome();

    QVariantMap options;
    options.insert( "color" , QColor(30,30,30) );
    options.insert( "scale-factor", 0.7 );

    ui->actionPreviousImage->setIcon(awesome->icon(fa::arrowleft, options));
    ui->actionNextImage->setIcon(awesome->icon(fa::arrowright, options));
    ui->actionSelect_Tool->setIcon(awesome->icon(fa::handpointero, options));
    ui->actionDraw_Tool->setIcon(awesome->icon(fa::pencilsquareo, options));
    ui->actionDetect_Objects->setIcon(awesome->icon(fa::magic, options));
    ui->actionDetect_Objects->setEnabled(false);
    connect(ui->actionDetect_Objects, SIGNAL(triggered(bool)), this, SLOT(detectCurrentImage()));
    connect(ui->actionSet_threshold, SIGNAL(triggered(bool)), this, SLOT(setConfidenceThreshold()));
    connect(ui->actionSet_NMS_threshold, SIGNAL(triggered(bool)), this, SLOT(setNMSThreshold()));
    detector.setConfidenceThreshold(settings->value("detector_confidence", 0.5).toDouble());
    detector.setNMSThreshold(settings->value("detector_nms_threshold", 0.4).toDouble());
    connect(ui->actionDetect_project, SIGNAL(triggered(bool)), this, SLOT(detectProject()));
    //ui->actionInit_Tracking->setIcon(awesome->icon(fa::objectungroup, options));

    refine_range_dialog = new RefineRangeDialog(this);
    connect(ui->actionRefine_image_range, SIGNAL(triggered(bool)), refine_range_dialog, SLOT(open()));
    connect(refine_range_dialog, SIGNAL(accepted()), this, SLOT(handleRefineRange()));

    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);

}

void MainWindow::mergeProject(QString filename){

    if(filename == ""){
        QString openDir = settings->value("project_folder", QDir::homePath()).toString();
        filename = QFileDialog::getOpenFileName(this, tr("Open Project"),
                                                        openDir,
                                                        tr("Label database (*.lbldb)"));
    }

    if(filename == "") return;

    LabelProject new_project;
    new_project.loadDatabase(filename);

    // Add new classes
    QList<QString> new_classes;
    new_project.getClassList(new_classes);

    qInfo() << "Found " << new_classes.size() << " classes.";

    for(auto &classname : new_classes){
        project->addClass(classname);
    }

    // Add new images
    QList<QString> new_images;
    new_project.getImageList(new_images);

    qInfo() << "Found " << new_images.size() << " images.";

    for(auto &image : new_images){
        // Add image
        auto res = project->addAsset(image);

        if(!res){
            qWarning() << "Problem adding: " << image;
        }else{
            qDebug() << "Added: " << image;
        }

        // Add labels for image
        QList<BoundingBox> bboxes;
        new_project.getLabels(image, bboxes);

        for(auto &bbox : bboxes){
            // Update the class ID
            bbox.classid = project->getClassId(bbox.classname);
            project->addLabel(image, bbox);
        }
    }

    updateImageList();
    updateClassList();
    updateDisplay();
}

void MainWindow::setCurrentClass(QString name){

    if(ui->classComboBox->currentText() != name){
        ui->classComboBox->setCurrentText(name);
    }

    current_class = name;
    emit selectedClass(current_class);
}

void MainWindow::setupDetector(void){

    ui->actionDetect_Objects->setEnabled(false);
    DetectorSetupDialog detection_dialog;
    detection_dialog.exec();

    if(detection_dialog.result() != QDialog::Accepted ) return;

    auto names_file = detection_dialog.getNames().toStdString();
    auto cfg_file = detection_dialog.getCfg().toStdString();
    auto weight_file = detection_dialog.getWeights().toStdString();

    detector.setChannels(detection_dialog.getChannels());
    detector.setTarget(detection_dialog.getTarget());
    detector.setFramework(detection_dialog.getFramework());
    detector.setConvertGrayscale(detection_dialog.getConvertGrayscale());
    detector.setConvertDepth(detection_dialog.getConvertDepth());
    detector.loadNetwork(names_file, cfg_file, weight_file);

    ui->actionDetect_Objects->setEnabled(true);
    ui->actionDetect_project->setEnabled(true);
}

void MainWindow::detectCurrentImage(){
    auto image = display->getOriginalImage();
    detectObjects(image, current_imagepath);

    updateClassList();
    updateLabels();
}

void MainWindow::detectObjects(cv::Mat &image, QString image_path){

    if(image.empty()) return;

    auto new_boxes = detector.infer(image);

    QList<BoundingBox> existing_boxes;
    project->getLabels(image_path, existing_boxes);

    for(auto &box : new_boxes){
        if(!project->classInDB(box.classname)){
            project->addClass(box.classname);
        }

        // Strip out boxes which are already in the image
        // assume detector is deterministic
        bool exists = false;
        for(auto &existing : existing_boxes){
            if(existing.rect == box.rect && existing.classname == box.classname){
                exists = true;
            }
        }

        if(!exists){
            qDebug() << "Adding label";
            project->addLabel(image_path, box);
        }

    }

}

void MainWindow::setConfidenceThreshold(void){
    QDialog  confidence_set_dialog(this);

    auto threshold_spinbox = new QDoubleSpinBox();
    threshold_spinbox->setMinimum(0);
    threshold_spinbox->setMaximum(1);
    threshold_spinbox->setValue(detector.getConfidenceThreshold());

    auto threshold_label = new QLabel("Detection Threshold: ");

    auto ok_button = new QPushButton("Ok");

    confidence_set_dialog.setWindowTitle("Confidence Threshold");
    confidence_set_dialog.setLayout(new QVBoxLayout());
    confidence_set_dialog.layout()->addWidget(threshold_label);
    confidence_set_dialog.layout()->addWidget(threshold_spinbox);
    confidence_set_dialog.layout()->addWidget(ok_button);
    confidence_set_dialog.layout()->setSizeConstraint(QLayout::SetFixedSize);

    connect(ok_button, SIGNAL(clicked(bool)), &confidence_set_dialog, SLOT(accept()));

    confidence_set_dialog.exec();

    if(confidence_set_dialog.result() == QDialog::Accepted){
        detector.setConfidenceThreshold(threshold_spinbox->value());
        settings->setValue("detector_confidence", detector.getConfidenceThreshold());
    }
}

void MainWindow::setNMSThreshold(void){
    QDialog  confidence_set_dialog(this);

    auto threshold_spinbox = new QDoubleSpinBox();
    threshold_spinbox->setMinimum(0);
    threshold_spinbox->setMaximum(1);
    threshold_spinbox->setValue(detector.getNMSThreshold());

    auto threshold_label = new QLabel("NMS Threshold: ");

    auto ok_button = new QPushButton("Ok");

    confidence_set_dialog.setWindowTitle("NMS Threshold");
    confidence_set_dialog.setLayout(new QVBoxLayout());
    confidence_set_dialog.layout()->addWidget(threshold_label);
    confidence_set_dialog.layout()->addWidget(threshold_spinbox);
    confidence_set_dialog.layout()->addWidget(ok_button);
    confidence_set_dialog.layout()->setSizeConstraint(QLayout::SetFixedSize);

    connect(ok_button, SIGNAL(clicked(bool)), &confidence_set_dialog, SLOT(accept()));

    confidence_set_dialog.exec();

    if(confidence_set_dialog.result() == QDialog::Accepted){
        detector.setNMSThreshold(threshold_spinbox->value());
        settings->setValue("detector_nms_threshold", detector.getNMSThreshold());
    }
}

void MainWindow::detectProject(void){

    QProgressDialog progress("Running detector", "Abort", 0, images.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setLabelText("...");

    int i = 0;
    for(auto& image_path : images){

        progress.setLabelText(image_path);

        if (progress.wasCanceled())
            break;

        auto image = cv::imread(image_path.toStdString(), cv::IMREAD_UNCHANGED|cv::IMREAD_ANYDEPTH);

        // Assume we have an alpha image if 4 channels
        if(image.channels() == 4){
            cv::cvtColor(image, image, cv::COLOR_BGRA2BGR);
        }

        detectObjects(image, image_path);

        progress.setValue(i++);

    }
    updateClassList();
    updateLabels();
}

void MainWindow::toggleAutoPropagate(bool state){
    track_previous = state;
}

void MainWindow::toggleRefineTracking(bool state){
    refine_on_propagate = state;
}

void MainWindow::enableWrap(bool enable){
    wrap_index = enable;
}

void MainWindow::jumpForward(int n){

    if(ui->imageNumberSpinbox->maximum() == 0) return;

    current_index = std::min(ui->imageNumberSpinbox->maximum()-1, ui->imageNumberSpinbox->value()+n);
    ui->imageNumberSpinbox->setValue(current_index);
    updateDisplay();
}

void MainWindow::jumpBackward(int n){

    current_index = std::max(1, ui->imageNumberSpinbox->value()-n);
    ui->imageNumberSpinbox->setValue(current_index);
    updateDisplay();
}

void MainWindow::setDrawMode(){
    ui->actionDraw_Tool->setChecked(true);
    ui->actionSelect_Tool->setChecked(false);
    currentImage->setDrawMode();
}

void MainWindow::setSelectMode(){
    ui->actionDraw_Tool->setChecked(false);
    ui->actionSelect_Tool->setChecked(true);
    currentImage->setSelectMode();
}

void MainWindow::openProject(QString fileName)
{

    if(fileName == ""){
        QString openDir = settings->value("project_folder", QDir::homePath()).toString();
        fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
                                                        openDir,
                                                        tr("Label database (*.lbldb)"));
    }

    if(fileName != ""){
        settings->setValue("project_folder", QFileInfo(fileName).absoluteDir().absolutePath());
        if(project->loadDatabase(fileName)){
            initDisplay();
            ui->menuImages->setEnabled(true);
            ui->menuDetection->setEnabled(true);
            ui->menuLabeling->setEnabled(true);
            ui->menuNavigation->setEnabled(true);
            ui->mainToolBar->setEnabled(true);
            ui->actionImport_Labels->setEnabled(true);
            ui->actionMerge_Project->setEnabled(true);
            setWindowTitle("DeepLabel - " + fileName);
        }else{
            QMessageBox::warning(this,tr("Project file error"), tr("Failed to open project."));
            setWindowTitle("DeepLabel");
        }
    }

    return;
}

void MainWindow::updateLabels(){
    QList<BoundingBox> bboxes;
    project->getLabels(current_imagepath, bboxes);

    ui->instanceCountLabel->setNum(static_cast<int>(bboxes.size()));
    currentImage->setBoundingBoxes(bboxes);
}

void MainWindow::addImageFolders(void){

    QDialog  image_folder_dialog(this);

    auto path_edit = new QLineEdit();
    auto ok_button = new QPushButton("Ok");
    auto path_label = new QLabel("Folder path (wildcards allowed)");

    image_folder_dialog.setWindowTitle("Add folders");
    image_folder_dialog.setLayout(new QVBoxLayout());
    image_folder_dialog.layout()->addWidget(path_label);
    image_folder_dialog.layout()->addWidget(path_edit);
    image_folder_dialog.layout()->addWidget(ok_button);

    connect(ok_button, SIGNAL(clicked(bool)), &image_folder_dialog, SLOT(accept()));

    image_folder_dialog.exec();

    if(image_folder_dialog.result() == QDialog::Accepted){
        project->addFolderRecursive(path_edit->text());
    }
}

void MainWindow::updateImageList(){
    project->getImageList(images);
    number_images = images.size();

    if(number_images == 0){
        ui->imageGroupBox->setDisabled(true);
        ui->labelGroupBox->setDisabled(true);
        ui->navigationGroupBox->setDisabled(true);
        ui->actionExport->setDisabled(true);
        ui->imageIndexLabel->setText(QString("-"));
    }else{
        ui->imageGroupBox->setEnabled(true);
        ui->labelGroupBox->setEnabled(true);
        ui->navigationGroupBox->setEnabled(true);
        ui->actionExport->setEnabled(true);
    }

    ui->imageProgressBar->setValue(0);
    ui->imageProgressBar->setMaximum(number_images);

    ui->imageNumberSpinbox->setMaximum(number_images);
    ui->imageNumberSpinbox->setValue(1);

    refine_range_dialog->setMaxImage(number_images);
}

void MainWindow::updateClassList(){
    project->getClassList(classes);

    ui->classComboBox->clear();

    QString classname;
    foreach(classname, classes){
        if(classname != "")
            ui->classComboBox->addItem(classname);
    }

    if(classes.size() > 0){
        ui->classComboBox->setEnabled(true);
        ui->removeClassButton->setEnabled(true);
        ui->classComboBox->setCurrentIndex(0);
    }else{
        ui->classComboBox->setDisabled(true);
        ui->removeClassButton->setDisabled(true);
    }
}

void MainWindow::addClass(){
    QString new_class = ui->newClassText->text();

    if(new_class.simplified() != "" && !classes.contains(new_class)){
        project->addClass(new_class.simplified());
        ui->newClassText->clear();
        updateClassList();
        setCurrentClass(new_class.simplified());
    }
}

void MainWindow::addLabel(BoundingBox bbox){
    project->addLabel(current_imagepath, bbox);
    updateLabels();
}

void MainWindow::removeLabel(BoundingBox bbox){
    project->removeLabel(current_imagepath, bbox);
    updateLabels();
}

void MainWindow::removeImageLabels(){
    if (QMessageBox::Yes == QMessageBox::question(this,
                                                  tr("Remove Labels"),
                                                  QString("Really delete all labels for this image?"))){;
        project->removeLabels(current_imagepath);
        updateLabels();
        updateDisplay();
    }
}

void MainWindow::removeImageLabelsForward(){
    if (QMessageBox::Yes == QMessageBox::question(this,
                                                  tr("Remove Labels"),
                                                  QString("Really delete all labels forwards?"))){;

        while(true){

            QList<BoundingBox> bboxes;
            project->getLabels(current_imagepath, bboxes);

            if(bboxes.size() == 0){
                return;
            }

            if(current_index == (number_images-1)){
                    return;
            }else{
                current_index++;
            }

            ui->imageNumberSpinbox->setValue(current_index+1);
            project->removeLabels(current_imagepath);

            // Show the new image
            updateLabels();
            updateDisplay();
        }

    }
}

void MainWindow::updateLabel(BoundingBox old_bbox, BoundingBox new_bbox){

    project->removeLabel(current_imagepath, old_bbox);
    project->addLabel(current_imagepath, new_bbox);
    updateLabels();
}

void MainWindow::removeImage(){
    if (QMessageBox::Yes == QMessageBox::question(this,
                                                  tr("Remove Image"),
                                                  tr("Really delete image and associated labels?"))){
        project->removeImage(current_imagepath);
        updateImageList();
        updateDisplay();
    }
}

void MainWindow::removeClass(){

    if (QMessageBox::Yes == QMessageBox::question(this,
                                                  tr("Remove Class"),
                                                  QString("Really delete all \"%1\" labels from your entire dataset?")
                                                    .arg(current_class))){;
        project->removeClass(current_class);
        updateClassList();
        updateDisplay();
    }
}

void MainWindow::initDisplay(){

    display->clearPixmap();

    updateImageList();
    updateClassList();

    current_index = 0;

    updateDisplay();
}

void MainWindow::nextUnlabelled(){
    int n = project->getNextUnlabelled(current_imagepath);

    if(n != -1){
        ui->imageNumberSpinbox->setValue(n);
        updateDisplay();
    }
}

void MainWindow::nextInstance(void){
    int n = project->getNextInstance(current_imagepath, current_class);

    if(n != -1){
        ui->imageNumberSpinbox->setValue(n+1);
        updateDisplay();
    }
}

QRect MainWindow::refineBoundingBoxSimple(cv::Mat image, QRect bbox, int margin, bool debug_save){
    QMargins margins(margin, margin, margin, margin);
    bbox += margins;

    // Clamp to within image - note zero-indexed so boundary is width-1 etc.

    bbox.setTop(std::max(bbox.top(), 0));
    bbox.setBottom(std::min(bbox.bottom(),image.rows-1));
    bbox.setLeft(std::max(bbox.left(), 0));
    bbox.setRight(std::min(bbox.right(), image.cols-1));

    auto roi = image(qrect2cv(bbox));

    // First we need to do foreground/background segmentation

    // Threshold input, 1 == good
    cv::Mat roi_thresh;

    // Convert colour images to grayscale for thresholding
    if(roi.channels() == 4){
        cv::cvtColor(roi, roi, cv::COLOR_BGRA2GRAY);
    }
    else if(roi.channels() == 3){
        cv::cvtColor(roi, roi, cv::COLOR_BGR2GRAY);
    }

    cv::threshold(roi, roi_thresh, 0, 255, cv::THRESH_OTSU|cv::THRESH_BINARY);

    if(debug_save) cv::imwrite("roi.png", roi);
    if(debug_save) cv::imwrite("roi_thresh.png", roi_thresh);

    std::vector<std::vector<cv::Point>> contours;
    cv::Mat hierarchy;
    cv::findContours(roi_thresh, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for(auto &contour : contours){

        if(static_cast<int>(cv::contourArea(contour)) == roi.rows*roi.cols){
            qDebug() << "Contour encloses all!";
        }

        //cv::drawContours(markers, {contour}, 0, {0,0,255});
        auto contour_bound = cv::boundingRect(contour);
        cv::rectangle(roi, contour_bound, {0,0,255});
    }

    if(debug_save) cv::imwrite("roi_contours.png", roi);

    QRect new_box;

    if(contours.size() > 0){
        auto contour_bound = cv::boundingRect(contours.at(0));
        new_box.setX(bbox.x()+contour_bound.x);
        new_box.setY(bbox.y()+contour_bound.y);
        new_box.setWidth(contour_bound.width);
        new_box.setHeight(contour_bound.height);
    }

    return new_box;
}

QRect MainWindow::refineBoundingBox(cv::Mat image, QRect bbox, int margin, bool debug_save){
    // Simple connected components refinement - debug only for now.

    QMargins margins(margin, margin, margin, margin);
    bbox += margins;

    bbox.setTop(std::max(0, std::min(image.rows, bbox.top())));
    bbox.setBottom(std::max(0, std::min(image.rows, bbox.top())));
    bbox.setLeft(std::max(0, std::min(image.cols, bbox.left())));
    bbox.setRight(std::max(0, std::min(image.cols, bbox.right())));

    auto roi = image(qrect2cv(bbox));

    // First we need to do foreground/background segmentation

    // Threshold input, 1 == good
    cv::Mat roi_thresh;
    cv::threshold(roi, roi_thresh, 0, 255, cv::THRESH_OTSU|cv::THRESH_BINARY);

    if(debug_save) cv::imwrite("roi.png", roi);
    if(debug_save) cv::imwrite("roi_thresh.png", roi_thresh);

    auto kernel_size = cv::Size(3,3);
    int iterations = 2;
    auto anchor = cv::Point(-1,-1);
    auto structure = cv::getStructuringElement(cv::MORPH_RECT, kernel_size);

    cv::Mat opening;
    cv::morphologyEx(roi_thresh, opening, cv::MORPH_OPEN, structure, anchor, iterations);

    // Background area
    iterations = 3;
    cv::Mat background;
    cv::dilate(opening, background, structure, anchor, iterations);

    if(debug_save) cv::imwrite("background.png", background);

    // Foreground area
    cv::Mat dist_transform;
    cv::Mat dist_labels;
    int mask_size = 5;
    cv::distanceTransform(opening, dist_transform, dist_labels, cv::DIST_L2, mask_size);

    if(debug_save) cv::imwrite("distance.png", dist_transform);

    cv::Mat foreground;
    double min_val, max_val;
    cv::minMaxIdx(dist_transform, &min_val, &max_val);
    int thresh = static_cast<int>(0.7*max_val);
    cv::threshold(dist_transform, foreground, thresh, 255,
                          cv::THRESH_BINARY);

    foreground.convertTo(foreground, CV_8UC1);
    if(debug_save) cv::imwrite("foreground.png", foreground);


    // Unknown region
    cv::Mat unknown;
    cv::subtract(background, foreground, unknown, cv::noArray(), CV_8UC1);

    if(debug_save) cv::imwrite("unknown.png", unknown);

    cv::Mat markers;
    cv::connectedComponents(foreground, markers, 8, CV_32SC1);
    markers += 1;

    int region_id = markers.at<int>(cv::Point(markers.cols/2, markers.rows/2));
    qDebug() << region_id;

    for(int i=0; i < static_cast<int>(markers.total()); i++){
        if(unknown.at<uchar>(i) == 255){
            markers.at<int>(i) = 0;
        }
    }

    if(debug_save) cv::imwrite("markers.png", markers);

    if(roi.channels() == 1) cv::cvtColor(roi, roi, cv::COLOR_GRAY2BGR);
    cv::watershed(roi, markers);

    markers.convertTo(markers, CV_8UC1);
    cv::threshold(markers, markers, 0, 255, cv::THRESH_OTSU);
    if(debug_save) cv::imwrite("watershed.png", markers);

    std::vector<std::vector<cv::Point>> contours;
    cv::Mat hierarchy;
    cv::findContours(markers, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

    cv::cvtColor(markers, markers, cv::COLOR_GRAY2BGR);

    cv::Rect contour_bound;
    for(auto &contour : contours){

        if(static_cast<int>(cv::contourArea(contour)) == roi.rows*roi.cols){
            qDebug() << "Contour encloses all!";
        }

        //cv::drawContours(markers, {contour}, 0, {0,0,255});
        contour_bound = cv::boundingRect(contour);
        cv::rectangle(markers, contour_bound, {0,0,255});
    }

    if(debug_save) cv::imwrite("contours.png", markers);
    QRect new_box;

    if(contours.size() == 1){
        new_box.setX(bbox.x()+contour_bound.x);
        new_box.setY(bbox.y()+contour_bound.y);
        new_box.setWidth(contour_bound.width);
        new_box.setHeight(contour_bound.height);
    }

    return new_box;
}

void MainWindow::refineBoxes(double min_new_area, double max_new_area){

    auto bboxes = currentImage->getBoundingBoxes();
    const auto image = currentImage->getImage();

    for(auto &bbox : bboxes){
        auto previous_area = bbox.rect.width()*bbox.rect.height();
        auto updated = refineBoundingBoxSimple(image, bbox.rect, 5, false);

        auto new_bbox = bbox;
        new_bbox.rect = updated;
        auto new_area = new_bbox.rect.width()*new_bbox.rect.height();

        // Make sure that new bbox isn't changed
        // too much
        if(!updated.size().isEmpty()
                && new_area >= min_new_area*previous_area
                && new_area <= max_new_area*previous_area){
            updateLabel(bbox, new_bbox);
        }
    }

    updateLabels();

}

void MainWindow::initTrackers(void){
    multitracker->init(currentImage->getImage(), currentImage->getBoundingBoxes());
}

void MainWindow::updateTrackers(void){

    // If there are no labels, and we're tracking the previous frame
    // propagate the bounding boxes. Otherwise we assume that the
    // current labels are the correct ones and should override.

    auto image = currentImage->getImage();
    if(image.empty()) return;

    multitracker->update(image);

    auto new_bboxes = multitracker->getBoxes();

    for(auto &new_bbox : new_bboxes){
        project->addLabel(current_imagepath, new_bbox);
    }

    updateLabels();
}

void MainWindow::nextImage(){

    if(images.empty()) return;

    if(current_index == (number_images-1)){
        if(wrap_index){
            current_index = 0;
        }else{
            return;
        }
    }else{
        current_index++;
    }

    ui->imageNumberSpinbox->setValue(current_index+1);

    // Show the new image
    updateDisplay();

    // Only auto-propagtae if we've enabled it and there are no boxes in the image already.
    if(track_previous && currentImage->getBoundingBoxes().size() == 0){
        updateTrackers();

        if(refine_on_propagate){
            refineBoxes();
        }
    }

    updateLabels();
}

void MainWindow::previousImage(){

    if(images.empty()) return;

    if(current_index == 0){
        if(wrap_index){
            current_index = number_images - 1;
        }else{
            return;
        }
    }else{
      current_index--;
    }

    ui->imageNumberSpinbox->setValue(current_index+1);
    updateDisplay();
}

void MainWindow::updateCurrentIndex(int index){
    current_index = index;
    ui->imageNumberSpinbox->setValue(current_index);
    updateDisplay();
}

void MainWindow::updateDisplay(){

    if(images.size() == 0){
        return;
    }else{
        current_index = ui->imageNumberSpinbox->value()-1;
        current_imagepath = images.at(current_index);
        display->setImagePath(current_imagepath);

        ui->statusBar->showMessage(current_imagepath);

        updateLabels();

        ui->imageProgressBar->setValue(current_index+1);
        ui->imageIndexLabel->setText(QString("%1/%2").arg(current_index+1).arg(number_images));
    }
}

void MainWindow::updateImageInfo(void){
    auto image_info = QFileInfo(current_imagepath);
    ui->imageBitDepthLabel->setText(QString("%1 bit").arg(display->getBitDepth()));
    ui->filenameLabel->setText(image_info.fileName());
    ui->filenameLabel->setToolTip(image_info.fileName());
    ui->filetypeLabel->setText(image_info.completeSuffix());
    ui->sizeLabel->setText(QString("%1 kB").arg(image_info.size() / 1000));
    ui->dimensionsLabel->setText(QString("(%1, %2) px").arg(currentImage->getImage().cols).arg(currentImage->getImage().rows));
}

void MainWindow::newProject()
{
    QString openDir = settings->value("project_folder", QDir::homePath()).toString();

    QFileDialog dialog(this);
    dialog.setDefaultSuffix(".lbldb");

    QString fileName = dialog.getSaveFileName(this, tr("New Project"),
                                                    openDir,
                                                    tr("Label database (*.lbldb)"));

    if(fileName != ""){
        free(project);
        project = new LabelProject;
        project->createDatabase(fileName);
        openProject(fileName);
    }

    return;
}

void MainWindow::addVideo(void){
    QString openDir = QDir::homePath();
    QString video_filename = QFileDialog::getOpenFileName(this, tr("Select video"),
                                                    openDir);

    QString output_folder = QFileDialog::getExistingDirectory(this, "Output folder", openDir);

    if(video_filename != ""){

        auto dialog_box = new QDialog(this);

        auto frame_skip_spin = new QSpinBox;
        auto frame_skip_label = new QLabel;
        auto layout = new QVBoxLayout;
        auto accept_button = new QPushButton;

        accept_button->setDefault(true);
        accept_button->setText("OK");
        connect(accept_button, SIGNAL(clicked()), dialog_box, SLOT(accept()));

        frame_skip_spin->setValue(1);
        frame_skip_spin->setMinimum(1);
        frame_skip_spin->setMaximum(1000);

        frame_skip_label->setText("Skip frames (1 to use all): ");

        dialog_box->setWindowModality(Qt::WindowModal);
        dialog_box->setLayout(layout);
        dialog_box->layout()->addWidget(frame_skip_label);
        dialog_box->layout()->addWidget(frame_skip_spin);
        dialog_box->layout()->addWidget(accept_button);

        if(dialog_box->exec()){
            qInfo() << "Frame skip: " << frame_skip_spin->value();
            project->addVideo(video_filename, output_folder, frame_skip_spin->value());
        }
    }

    updateImageList();
    initDisplay();

}

void MainWindow::addImages(void){
    QString openDir = settings->value("data_folder", QDir::homePath()).toString();
    QStringList image_filenames = QFileDialog::getOpenFileNames(this, tr("Select image(s)"),
                                                    openDir,
                                                    tr("JPEG (*.jpg *.jpeg *.JPG *.JPEG);;PNG (*.png *.PNG);;BMP (*.bmp *.BMP);;TIFF (*.tif *.tiff *.TIF *.TIFF);;All images (*.jpg *.jpeg *.png *.bmp *.tiff)"));

    if(image_filenames.size() != 0){
        QString path;

        QProgressDialog progress("Loading images", "Abort", 0, image_filenames.size(), this);
        progress.setWindowModality(Qt::WindowModal);
        int i=0;

        foreach(path, image_filenames){
            if(progress.wasCanceled()){
                break;
            }

            project->addAsset(path);
            progress.setValue(i++);
        }

        settings->setValue("data_folder", QDir(image_filenames.at(0)).dirName());
    }

    updateImageList();
    initDisplay();

    return;
}

void MainWindow::addImageFolder(void){
    QString openDir = settings->value("data_folder", QDir::homePath()).toString();
    QString path = QFileDialog::getExistingDirectory(this, tr("Select image folder"),
                                                    openDir);

    if(path != ""){
        int number_added = project->addImageFolder(path);
        settings->setValue("data_folder", path);
        qInfo() << "Added: " << number_added << " images";
    }

    updateImageList();
    initDisplay();

    return;
}

void MainWindow::handleExportDialog(){

    // If we hit OK and not cancel
    if(export_dialog->result() != QDialog::Accepted ) return;

    QThread* export_thread = new QThread;
    BaseExporter* exporter = nullptr;

    if(export_dialog->getExporter() == "Kitti"){
        exporter = new KittiExporter(project);
    }else if(export_dialog->getExporter() == "Darknet"){
        exporter = new DarknetExporter(project);
        static_cast<DarknetExporter*>(exporter)->generateLabelIds(export_dialog->getNamesFile());
    }else if(export_dialog->getExporter() == "Pascal VOC"){
        exporter = new PascalVocExporter(project);
        static_cast<PascalVocExporter*>(exporter)->setExportMap(export_dialog->getCreateLabelMap());
        exporter->process();
    }else if(export_dialog->getExporter().startsWith("COCO")){
        exporter = new CocoExporter(project);
    }else if(export_dialog->getExporter().startsWith("GCP")){
        exporter = new GCPExporter(project);
        static_cast<GCPExporter*>(exporter)->setBucket(export_dialog->getBucket());
    }else{
        qCritical() << "Invalid exporter type";
        return;
    }

    if(exporter != nullptr){
        exporter->moveToThread(export_thread);
        exporter->setExportUnlabelled(export_dialog->getExportUnlablled());
        exporter->setFilenamePrefix(export_dialog->getFilePrefix());
        exporter->setAppendLabels(export_dialog->getAppendLabels());

        if(export_dialog->getValidationSplitEnabled()){
            exporter->setValidationSplit(true);
            exporter->splitData(export_dialog->getValidationSplit(), export_dialog->getShuffle());
        }else{
            exporter->setValidationSplit(false);
            exporter->splitData(0, export_dialog->getShuffle());
        }


        exporter->setOutputFolder(export_dialog->getOutputFolder());
        exporter->process();
    }else{
        qCritical() << "Failed to instantiate exporter";
    }

}

void MainWindow::launchExportDialog(){

    export_dialog = new ExportDialog(this);

    export_dialog->setModal(true);
    connect(export_dialog, SIGNAL(accepted()), this, SLOT(handleExportDialog()));

    export_dialog->open();
}

void MainWindow::handleRefineRange(){

    if(refine_range_dialog->result() != QDialog::Accepted ){
        return;
    }

    refineRange(refine_range_dialog->getStart(), refine_range_dialog->getEnd());
}

void MainWindow::refineRange(int start, int end){

    if(start == -1 || end == -1) return;

    QList<QString> images;
    project->getImageList(images);

    QProgressDialog progress("Refining images", "Cancel", 0, end-start, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setLabelText("...");
    QApplication::processEvents(); // Otherwise stuff can happen a wee bit fast

    for(int i = start; i < end; ++i){
        if(progress.wasCanceled())
            break;

        progress.setLabelText(images[i]);

        updateCurrentIndex(i);
        refineBoxes();
        nextImage();

        progress.setValue(i);
        QApplication::processEvents();

    }
}

void MainWindow::computeStatistics(void){
    QList<QString> images;
    project->getImageList(images);
    std::vector<cv::Mat> histograms;
    histograms.resize(4);

    QProgressDialog progress("...", "Abort", 0, images.size(), this->parentWidget());
    progress.setWindowModality(Qt::WindowModal);
    progress.setWindowTitle("Calculating stats");
    progress.show();
    int i = 0;

    for(auto &image_path : images){
        if(progress.wasCanceled())
            break;

        auto image = cv::imread(image_path.toStdString(), cv::IMREAD_UNCHANGED);
        std::vector<cv::Mat> image_channels;
        cv::split( image, image_channels );
        int histSize = 65536;
        float range[] = { 0, 65536 }; //the upper boundary is exclusive
        const float* histRange = { range };
        bool uniform = true, accumulate = true;

        for(int c=0; c < static_cast<int>(image_channels.size()); ++c){
            cv::calcHist( &image_channels[c], 1, 0, cv::Mat(), histograms[c], 1, &histSize, &histRange, uniform, accumulate );
        }

        progress.setValue(++i);
        progress.setLabelText(image_path);

        QApplication::processEvents();
    }

    for(auto &histogram : histograms){

        double s = 0;
        double total_hist = 0;

        for(long long i=0; i < static_cast<long long>(histogram.total()); ++i){
            s += histogram.at<float>(i) * (i + 0.5); // bin centre
            total_hist += histogram.at<float>(i);
        }

        double mean = s / total_hist;

        double t = 0;
        for(long long i=0; i < static_cast<long long>(histogram.total()); ++i){
          double x = (i - mean);
          t += histogram.at<float>(i)*x*x;
        }
        double stdev = std::sqrt(t / total_hist);

        qInfo() << "Mean: " << mean;
        qInfo() << "Std: " << stdev;
    }
}

void MainWindow::handleImportDialog(){

    // If we hit OK and not cancel
    if (import_dialog->result() != QDialog::Accepted)
        return;

    QThread* import_thread = new QThread;

    if (import_dialog->getImporter() == "Darknet") {
        DarknetImporter importer(project);
        importer.moveToThread(import_thread);
        importer.setImportUnlabelled(import_dialog->getImportUnlabelled());

        if (import_dialog->getUseRelativePaths()) {
            importer.import(import_dialog->getInputFile(),
                            import_dialog->getNamesFile(),
                            import_dialog->getRelativePath());
        } else {
            importer.import(import_dialog->getInputFile(), import_dialog->getNamesFile());
        }

    } else if (import_dialog->getImporter() == "Coco") {
        CocoImporter importer(project);
        importer.moveToThread(import_thread);
        importer.setImportUnlabelled(import_dialog->getImportUnlabelled());
        importer.import(import_dialog->getAnnotationFile(), import_dialog->getInputFile());
    } else if (import_dialog->getImporter() == "MOT") {
        MOTImporter importer(project);
        importer.moveToThread(import_thread);
        importer.setImportUnlabelled(import_dialog->getImportUnlabelled());
        importer.loadClasses(import_dialog->getNamesFile());
        importer.import(import_dialog->getInputFile());
    } else if (import_dialog->getImporter() == "BirdsAI") {
        BirdsAIImporter importer(project);
        importer.moveToThread(import_thread);
        importer.setImportUnlabelled(import_dialog->getImportUnlabelled());
        importer.loadClasses(import_dialog->getNamesFile());
        importer.import(import_dialog->getInputFile(), import_dialog->getAnnotationFile());
    } else if (import_dialog->getImporter() == "PascalVOC") {
        PascalVOCImporter importer(project);
        importer.moveToThread(import_thread);
        importer.setImportUnlabelled(import_dialog->getImportUnlabelled());
        importer.import(import_dialog->getInputFile(), import_dialog->getAnnotationFile());
    }

    initDisplay();
}

void MainWindow::launchImportDialog(){

    import_dialog = new ImportDialog(this);

    import_dialog->setModal(true);
    connect(import_dialog, SIGNAL(accepted()), this, SLOT(handleImportDialog()));

    import_dialog->open();
}

MainWindow::~MainWindow()
{
    delete ui;
}
