#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionNew_Project, SIGNAL(triggered(bool)), this, SLOT(newProject()));
    connect(ui->actionOpen_Project, SIGNAL(triggered(bool)), this, SLOT(openProject()));

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

    display = new ImageDisplay;
    ui->imageDisplayLayout->addWidget(display);
    currentImage = display->getImageLabel();


    connect(this, SIGNAL(selectedClass(QString)), currentImage, SLOT(setClassname(QString)));
    connect(currentImage, SIGNAL(newLabel(BoundingBox)), this, SLOT(addLabel(BoundingBox)));
    connect(currentImage, SIGNAL(removeLabel(BoundingBox)), this, SLOT(removeLabel(BoundingBox)));
    connect(ui->actionDraw_Tool, SIGNAL(triggered(bool)), currentImage, SLOT(setDrawMode()));
    connect(ui->actionSelect_Tool, SIGNAL(triggered(bool)), currentImage, SLOT(setSelectMode()));
    connect(ui->classComboBox, SIGNAL(currentIndexChanged(QString)), currentImage, SLOT(setClassname(QString)));
    connect(display, SIGNAL(image_loaded()), this, SLOT(updateImageInfo()));

    connect(ui->removeClassButton, SIGNAL(clicked(bool)), this, SLOT(removeClass()));
    connect(ui->removeImageButton, SIGNAL(clicked(bool)), this, SLOT(removeImage()));
    connect(ui->removeImageLabelsButton, SIGNAL(clicked(bool)), this, SLOT(removeImageLabels()));

    ui->actionDraw_Tool->setChecked(true);

    connect(ui->changeImageButton, SIGNAL(clicked(bool)), this, SLOT(changeImage()));
    connect(ui->imageNumberSpinbox, SIGNAL(editingFinished()), this, SLOT(changeImage()));

    connect(ui->actionWrap_images, SIGNAL(triggered(bool)), this, SLOT(enableWrap(bool)));
    connect(ui->actionExport, SIGNAL(triggered(bool)), this, SLOT(launchExportDialog()));
    connect(ui->actionRefine_boxes, SIGNAL(triggered(bool)), this, SLOT(refineBoxes()));

    connect(ui->actionSetup_detector, SIGNAL(triggered(bool)), this, SLOT(setupDetector()));

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

    export_dialog.setModal(true);
    connect(&export_dialog, SIGNAL(accepted()), this, SLOT(handleExportDialog()));

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
    connect(ui->actionDetect_Objects, SIGNAL(triggered(bool)), this, SLOT(detectObjects()));
    connect(ui->actionSet_threshold, SIGNAL(triggered(bool)), this, SLOT(setConfidenceThreshold()));
    detector.setConfidenceThreshold(settings->value("detector_confidence", 0.5).toDouble());
    connect(ui->actionDetect_project, SIGNAL(triggered(bool)), this, SLOT(detectProject()));
    //ui->actionInit_Tracking->setIcon(awesome->icon(fa::objectungroup, options));

    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);

}

void MainWindow::setupDetector(void){

    ui->actionDetect_Objects->setEnabled(false);
    DetectorSetupDialog detection_dialog;
    detection_dialog.exec();

    if(detection_dialog.result() != QDialog::Accepted ) return;

    auto names_file = detection_dialog.getNames().toStdString();
    auto cfg_file = detection_dialog.getCfg().toStdString();
    auto weight_file = detection_dialog.getWeights().toStdString();

    detector.loadDarknet(names_file, cfg_file, weight_file);
    ui->actionDetect_Objects->setEnabled(true);
}

void MainWindow::detectObjects(){

    auto image = cv::imread(current_imagepath.toStdString(), cv::IMREAD_UNCHANGED|cv::IMREAD_ANYDEPTH);

    if(image.empty()) return;

    auto new_boxes = detector.infer(image);
    auto existing_boxes = currentImage->getBoundingBoxes();

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
            project->addLabel(current_imagepath, box);
        }

    }

    updateClassList();
    updateLabels();

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
                project->addLabel(image_path, box);
            }

        }

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

void MainWindow::changeImage(){
    current_index = ui->imageNumberSpinbox->value()-1;
    updateDisplay();
}

void MainWindow::jumpForward(int n){
    current_index = std::min(ui->imageNumberSpinbox->maximum()-1, ui->imageNumberSpinbox->value()+n);
    updateDisplay();
}

void MainWindow::jumpBackward(int n){
    current_index = std::max(1, ui->imageNumberSpinbox->value()-n);
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

void MainWindow::openProject()
{
    QString openDir = settings->value("project_folder", QDir::homePath()).toString();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
                                                    openDir,
                                                    tr("Label database (*.lbldb)"));

    if(fileName != ""){
        settings->setValue("project_folder", QFileInfo(fileName).absoluteDir().absolutePath());
        if(project->loadDatabase(fileName)){
            initDisplay();
        }else{
            QMessageBox::warning(this,tr("Remove Image"), tr("Failed to open project."));
        }
    }

    return;
}

void MainWindow::updateLabels(){
    QList<BoundingBox> bboxes;

    project->getLabels(current_imagepath, bboxes);
    ui->instanceCountLabel->setNum(bboxes.size());
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
    ui->imageProgressBar->setMaximum(number_images);
    ui->imageNumberSpinbox->setMaximum(number_images);

    if(number_images == 0){
        ui->changeImageButton->setDisabled(true);
        ui->imageNumberSpinbox->setDisabled(true);
        ui->imageProgressBar->setDisabled(true);
        ui->propagateCheckBox->setDisabled(true);
    }else{
        ui->changeImageButton->setEnabled(true);
        ui->imageNumberSpinbox->setEnabled(true);
        ui->imageProgressBar->setEnabled(true);
        ui->propagateCheckBox->setEnabled(true);
    }
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
        current_class = ui->classComboBox->currentText();
        emit selectedClass(current_class);
    }else{
        ui->classComboBox->setDisabled(true);
        ui->removeClassButton->setDisabled(true);
    }
}

void MainWindow::addClass(){
    QString new_class = ui->newClassText->text();

    if(new_class.simplified() != "" && !classes.contains(new_class)){
        project->addClass(new_class);
        ui->newClassText->clear();
        updateClassList();
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
    current_class = ui->classComboBox->currentText();

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

    updateImageList();
    updateClassList();

    if(number_images != 0){
        current_index = 0;
        ui->imageGroupBox->setEnabled(true);
        ui->labelGroupBox->setEnabled(true);
        ui->navigationGroupBox->setEnabled(true);
        updateDisplay();
        ui->actionExport->setEnabled(true);
    }
}

void MainWindow::nextUnlabelled(){
    int n = project->getNextUnlabelled(current_imagepath);

    if(n != -1){
        ui->imageNumberSpinbox->setValue(n);
        changeImage();
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
    if(roi.channels() == 3){
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

void MainWindow::refineBoxes(){

    auto bboxes = currentImage->getBoundingBoxes();
    const auto image = currentImage->getImage();

    for(auto &bbox : bboxes){
        auto previous_area = bbox.rect.width()*bbox.rect.height();
        auto updated = refineBoundingBoxSimple(image, bbox.rect, 5, true);

        auto new_bbox = bbox;
        new_bbox.rect = updated;
        auto new_area = new_bbox.rect.width()*new_bbox.rect.height();

        if(!updated.size().isEmpty() && new_area >= 0.5*previous_area) updateLabel(bbox, new_bbox);
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

    multitracker->update(currentImage->getImage());

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

    updateDisplay();
}

void MainWindow::updateDisplay(){

    if(images.size() == 0){
        return;
    }

    current_imagepath = images.at(current_index);
    display->setImagePath(current_imagepath);

    updateLabels();

    ui->imageProgressBar->setValue(current_index+1);
    ui->imageNumberSpinbox->setValue(current_index+1);
    ui->imageIndexLabel->setText(QString("%1/%2").arg(current_index+1).arg(number_images));

}

void MainWindow::updateImageInfo(void){
    auto image_info = QFileInfo(current_imagepath);
    ui->imageBitDepthLabel->setText(QString("%1 bit").arg(currentImage->getImage().elemSize() * 8));
    ui->filenameLabel->setText(image_info.fileName());
    ui->filenameLabel->setToolTip(image_info.fileName());
    ui->filetypeLabel->setText(image_info.completeSuffix());
    ui->sizeLabel->setText(QString("%1 kB").arg(image_info.size() / 1000));
    ui->dimensionsLabel->setText(QString("(%1, %2) px").arg(currentImage->getImage().cols).arg(currentImage->getImage().rows));
}

void MainWindow::newProject()
{
    QString openDir = settings->value("project_folder", QDir::homePath()).toString();
    QString fileName = QFileDialog::getSaveFileName(this, tr("New Project"),
                                                    openDir,
                                                    tr("Label database (*.lbldb)"));

    if(fileName != ""){
        project->createDatabase(fileName);
    }

    updateDisplay();

    return;
}

void MainWindow::addVideo(void){
    QString openDir = QDir::homePath();
    QString video_filename = QFileDialog::getOpenFileName(this, tr("Select video"),
                                                    openDir);

    QString output_folder = QFileDialog::getExistingDirectory(this, "Output folder", openDir);

    if(video_filename != ""){
        project->addVideo(video_filename, output_folder);
    }

    updateImageList();
    initDisplay();

}

void MainWindow::addImages(void){
    QString openDir = settings->value("data_folder", QDir::homePath()).toString();
    QStringList image_filenames = QFileDialog::getOpenFileNames(this, tr("Select image(s)"),
                                                    openDir,
                                                    tr("JPEG (*.jpg, *.jpeg, *.JPG, *.JPEG);;PNG (*.png, *.PNG);;BMP (*.bmp, *.BMP);;TIFF (*.tif, *.tiff, *.TIF, *.TIFF);;All images (*.jpg, *.jpeg, *.png, *.bmp, *.tiff)"));

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
        qDebug() << "Added: " << number_added << " images";
    }

    updateImageList();
    initDisplay();

    return;
}

void MainWindow::handleExportDialog(){

    // If we hit OK and not cancel
    if(export_dialog.result() != QDialog::Accepted ) return;

    QThread* export_thread = new QThread;

    if(export_dialog.getExporter() == "Kitti"){
        KittiExporter exporter(project);
        exporter.moveToThread(export_thread);
        exporter.setOutputFolder(export_dialog.getOutputFolder());
        exporter.splitData(export_dialog.getValidationSplit(), export_dialog.getShuffle());
        exporter.process();
    }else if(export_dialog.getExporter() == "Darknet"){
        DarknetExporter exporter(project);
        exporter.moveToThread(export_thread);
        exporter.generateLabelIds(export_dialog.getNamesFile());
        exporter.setOutputFolder(export_dialog.getOutputFolder());
        exporter.splitData(export_dialog.getValidationSplit(), export_dialog.getShuffle());
        exporter.process();
    }else if(export_dialog.getExporter() == "Pascal VOC"){
        PascalVocExporter exporter(project);
        exporter.moveToThread(export_thread);
        exporter.setOutputFolder(export_dialog.getOutputFolder());
        exporter.splitData(export_dialog.getValidationSplit(), export_dialog.getShuffle());
        exporter.process(export_dialog.getCreateLabelMap());

    }else if(export_dialog.getExporter().startsWith("COCO")){
        CocoExporter exporter(project);
        exporter.moveToThread(export_thread);
        exporter.setOutputFolder(export_dialog.getOutputFolder());
        exporter.splitData(export_dialog.getValidationSplit(), export_dialog.getShuffle());
        exporter.process();
    }
}

void MainWindow::launchExportDialog(){
    export_dialog.open();
}

MainWindow::~MainWindow()
{
    delete ui;
}
