#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionNew_Project, SIGNAL(triggered(bool)), this, SLOT(newProject()));
    connect(ui->actionOpen_Project, SIGNAL(triggered(bool)), this, SLOT(openProject()));

    connect(ui->actionAdd_image, SIGNAL(triggered(bool)), this, SLOT(addImages()));
    connect(ui->actionAdd_image_folder, SIGNAL(triggered(bool)), this, SLOT(addImageFolder()));

    connect(ui->actionNextImage, SIGNAL(triggered(bool)), this, SLOT(nextImage()));
    connect(ui->actionPreviousImage, SIGNAL(triggered(bool)), this, SLOT(previousImage()));

    connect(ui->addClassButton, SIGNAL(clicked(bool)), this, SLOT(addClass()));
    connect(ui->newClassText, SIGNAL(editingFinished()), this, SLOT(addClass()));

    connect(ui->actionInit_Tracking, SIGNAL(triggered(bool)), this, SLOT(initTrackers()));
    connect(ui->actionPropagate_Tracking, SIGNAL(triggered(bool)), this, SLOT(propagateTracking()));
    connect(ui->propagateCheckBox, SIGNAL(clicked(bool)), this, SLOT(toggleAutoPropagate(bool)));
    connect(ui->refineTrackingCheckbox, SIGNAL(clicked(bool)), this, SLOT(toggleRefineTracking(bool)));

    connect(ui->nextUnlabelledButton, SIGNAL(clicked(bool)), this, SLOT(nextUnlabelled()));

    currentImage = new ImageLabel(this);
    ui->scrollAreaWidgetContents->layout()->setAlignment(Qt::AlignHCenter);
    ui->scrollAreaWidgetContents->layout()->setAlignment(Qt::AlignVCenter);
    ui->scrollAreaWidgetContents->layout()->addWidget(currentImage);

    connect(this, SIGNAL(selectedClass(QString)), currentImage, SLOT(setClassname(QString)));
    connect(currentImage, SIGNAL(newLabel(BoundingBox)), this, SLOT(addLabel(BoundingBox)));
    connect(currentImage, SIGNAL(removeLabel(BoundingBox)), this, SLOT(removeLabel(BoundingBox)));
    connect(ui->

    connect(ui->removeClassButton, SIGNAL(clicked(bool)), this, SLOT(removeClass()));
    connect(ui->removeImageButton, SIGNAL(clicked(bool)), this, SLOT(removeImage()));

    ui->actionDraw_Tool->setChecked(true);
    connect(ui->actionDraw_Tool, SIGNAL(triggered(bool)), this, SLOT(setDrawMode()));
    connect(ui->actionSelect_Tool, SIGNAL(triggered(bool)), this, SLOT(setSelectMode()));

    connect(ui->classComboBox, SIGNAL(currentIndexChanged(QString)), currentImage, SLOT(setClassname(QString)));
    connect(ui->changeImageButton, SIGNAL(clicked(bool)), this, SLOT(changeImage()));
    connect(ui->imageNumberSpinbox, SIGNAL(editingFinished()), this, SLOT(changeImage()));

    connect(ui->actionWrap_images, SIGNAL(triggered(bool)), this, SLOT(enableWrap(bool)));
    connect(ui->actionExport, SIGNAL(triggered(bool)), this, SLOT(launchExportDialog()));

    connect(ui->actionRefine_boxes, SIGNAL(triggered(bool)), this, SLOT(refineBoxes()));

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
    project = new LabelProject(this);

    export_dialog.setModal(true);
    connect(&export_dialog, SIGNAL(accepted()), this, SLOT(handleExportDialog()));

    settings = new QSettings("DeepLabel", "DeepLabel");
    qDebug() << settings->value("project_folder").toString();

}

void MainWindow::toggleAutoPropagate(bool state){
    track_previous = state;
}

void MainWindow::toggleRefineTracking(bool state){
    refine_on_propagate = state;
}

cv::Ptr<cv::Tracker> MainWindow::createTrackerByName(trackerType type)
{
  using namespace cv;
  using namespace std;

  Ptr<Tracker> tracker;
  if (type == BOOSTING)
    tracker = TrackerBoosting::create();
  else if (type == MIL)
    tracker = TrackerMIL::create();
  else if (type == KCF)
    tracker = TrackerKCF::create();
  else if (type == TLD)
    tracker = TrackerTLD::create();
  else if (type == MEDIANFLOW)
    tracker = TrackerMedianFlow::create();
  else if (type == GOTURN)
    tracker = TrackerGOTURN::create();
  else if (type == MOSSE)
    tracker = TrackerMOSSE::create();
  else if (type == CSRT)
    tracker = TrackerCSRT::create();
  else {
    cout << "Incorrect tracker specified";
  }
  return tracker;
}

void MainWindow::enableWrap(bool enable){
    wrap_index = enable;
}


void MainWindow::changeImage(){
    current_index = ui->imageNumberSpinbox->value()-1;
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

    if(new_class != "" && !classes.contains(new_class)){
        project->addClass(new_class);
        ui->newClassText->clear();
        updateClassList();
    }
}

void MainWindow::addLabel(BoundingBox bbox){
    project->addLabel(current_imagepath, bbox);
}

void MainWindow::removeLabel(BoundingBox bbox){
    project->removeLabel(current_imagepath, bbox);
    updateLabels();
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

void MainWindow::removeLabelsFromImage(){
    project->removeLabels(current_imagepath);
    updateImageList();
    updateDisplay();
}

void MainWindow::removeClass(){
    if (QMessageBox::Yes == QMessageBox::question(this,
                                                  tr("Remove Image"),
                                                  tr("Really delete class and associated labels?"))){
        project->removeClass(current_class);
        updateClassList();
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

cv::Rect2d MainWindow::qrect2cv(QRect rect){
    return cv::Rect2d(rect.x(), rect.y(), rect.width(),rect.height());
}

void MainWindow::nextUnlabelled(){
    int n = project->getNextUnlabelled(current_imagepath);

    if(n != -1){
        ui->imageNumberSpinbox->setValue(n);
        changeImage();
    }
}

cv::Mat MainWindow::initCamShift(cv::Mat image, QRect bbox){
    auto roi = image(qrect2cv(bbox));

    if(roi.channels() == 1){
    cv::cvtColor(roi, roi, cv::COLOR_GRAY2BGR);
    cv::cvtColor(roi, roi, cv::COLOR_BGR2HSV);
    }else if(roi.channels() == 3){
        cv::cvtColor(roi, roi, cv::COLOR_BGR2HSV);
    }else{
        // What the hell kind of 2 channel image is this?
        return cv::Mat();
    }

    // Generate histogram
    auto lowScalar = cv::Scalar(30,30,0);
    auto highScalar = cv::Scalar(180,180,180);

    cv::Mat mask;;
    cv::inRange(roi, lowScalar, highScalar, mask);

    cv::Mat roiHist;

    int histSize = 256;
    float range[] = { 0, 180 }; //the upper boundary is exclusive
    const float* histRange = { range };

    bool uniform = true;
    bool accumulate = true;

    cv::calcHist( &roi, 1, nullptr, mask, roiHist, 1, &histSize, &histRange, uniform, accumulate);
    cv::normalize(roiHist, roiHist, 0, 255, cv::NORM_MINMAX);

    return roiHist;

}

cv::Rect2i MainWindow::updateCamShift(cv::Mat image, cv::Mat roiHist, QRect bbox){
    auto rect = qrect2cv(bbox);
    auto roi = image(rect);

    if(roi.channels() == 1){
    cv::cvtColor(roi, roi, cv::COLOR_GRAY2BGR);
    cv::cvtColor(roi, roi, cv::COLOR_BGR2HSV);
    }else if(roi.channels() == 3){
        cv::cvtColor(roi, roi, cv::COLOR_BGR2HSV);
    }else{
        // What the hell kind of 2 channel image is this?
        return cv::Rect();
    }

    auto rectInt = cv::Rect2i(rect);

    cv::Mat backProjection;
    float range[] = { 0, 180 }; //the upper boundary is exclusive
    const float* histRange = { range };
    cv::calcBackProject(&image, 1, nullptr, roiHist, backProjection, &histRange);

    auto termcrit = cv::TermCriteria(cv::TermCriteria::EPS|cv::TermCriteria::COUNT, 10, 1);
    auto rotated_rect = cv::CamShift(backProjection, rectInt, termcrit);

    return rotated_rect.boundingRect();
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

void MainWindow::initTrackersCamShift(){
    auto bboxes = currentImage->getBoundingBoxes();

    trackers_camshift.clear();
    auto image = currentImage->getImage();
    QMutex mutex;

    QtConcurrent::blockingMap(bboxes.begin(), bboxes.end(), [&](BoundingBox &bbox)
    {
        auto roi_hist = initCamShift(image, bbox.rect);
        mutex.lock();
        trackers_camshift.push_back({roi_hist, bbox});
        mutex.unlock();
    });

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

}

void MainWindow::propagateTrackingCamShift(){

    const auto image = currentImage->getImage();

    QtConcurrent::blockingMap(trackers_camshift.begin(), trackers_camshift.end(), [&](auto&& tracker)
    {
        cv::Rect2d bbox = updateCamShift(image, tracker.first, tracker.second.rect);

        QRect new_roi;
        new_roi.setX(static_cast<int>(bbox.x));
        new_roi.setY(static_cast<int>(bbox.y));
        new_roi.setWidth(static_cast<int>(bbox.width));
        new_roi.setHeight(static_cast<int>(bbox.height));

        BoundingBox new_bbox;
        new_bbox.rect = new_roi;
        new_bbox.classname = tracker.second.classname;

        project->addLabel(current_imagepath, new_bbox);

    });

    updateLabels();
}

void MainWindow::initTrackers(){
    auto bboxes = currentImage->getBoundingBoxes();

    trackers.clear();
    auto image = currentImage->getImage();
    QMutex mutex;

    // If we are tracking and we have some labelled boxes already
    QtConcurrent::blockingMap(bboxes.begin(), bboxes.end(), [&](BoundingBox &bbox)
    {
        auto tracker = createTrackerByName(CSRT);
        tracker->init(image, qrect2cv(bbox.rect));

        mutex.lock();
        trackers.push_back({tracker, bbox.classname});
        mutex.unlock();
    });
}

void MainWindow::propagateTracking(){

    // If there are no labels, and we're tracking the previous frame
    // propagate the bounding boxes. Otherwise we assume that the
    // current labels are the correct ones and should override.

    QtConcurrent::blockingMap(trackers.begin(), trackers.end(), [&](auto&& tracker)
    {
        cv::Rect2d bbox;
        auto image = currentImage->getImage();

        if( tracker.first->update(image, bbox)){

            QRect new_roi;
            new_roi.setX(static_cast<int>(bbox.x));
            new_roi.setY(static_cast<int>(bbox.y));
            new_roi.setWidth(static_cast<int>(bbox.width));
            new_roi.setHeight(static_cast<int>(bbox.height));

            BoundingBox new_bbox;
            new_bbox.rect = new_roi;
            new_bbox.classname = tracker.second;

            if(refine_on_propagate)
                refineBoundingBoxSimple(image, new_bbox.rect);

            project->addLabel(current_imagepath, new_bbox);
        }
    });

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

    updateDisplay();

    // Only auto-propagtae if we've enabled it and there are no boxes in the image already.
    if(track_previous && currentImage->getBoundingBoxes().size() == 0) propagateTracking();
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

QImage MainWindow::convert16(const cv::Mat &source){
    short* pSource = reinterpret_cast<short*>(source.data);

    QImage dest(source.cols, source.rows, QImage::Format_Grayscale8);
    int pixelCounts = dest.width() * dest.height();

    double minval, maxval;
    cv::minMaxIdx(source, &minval, &maxval);
    double range = maxval-minval;
    double scale_factor = 255.0/range;

    uchar* pDest = dest.bits();

    for (int i = 0; i < pixelCounts; i++)
    {
        uchar value = static_cast<uchar>((*(pSource) - minval)*scale_factor);
        *(pDest++) = value;
        pSource++;
   }

   return dest;
}

void MainWindow::updateDisplay(){

    if(images.size() == 0){
        QPixmap pixmap = QPixmap::fromImage(QImage());
        currentImage->setPixmap(pixmap);
        return;
    }

    current_imagepath = images.at(current_index);
    pixmap.load(current_imagepath);

    ui->imageProgressBar->setValue(current_index+1);
    ui->imageNumberSpinbox->setValue(current_index+1);
    ui->imageIndexLabel->setText(QString("%1/%2").arg(current_index+1).arg(number_images));

    auto image = cv::imread(current_imagepath.toStdString(), cv::IMREAD_UNCHANGED|cv::IMREAD_ANYDEPTH);

    if(image.empty()){
        qDebug() << "Failed to load image " << current_imagepath;
        return;
    }

    display_image = image.clone();

    if(image.elemSize() == 2){
        // Filthy hack because Qt sucks...
        QTemporaryDir dir;
        if (dir.isValid()) {
            convert16(display_image).save(dir.path()+"/temp.png");
            pixmap.load(dir.path()+"/temp.png");
        }
    }else{
        // Default to single channel 8-bit image
        QImage::Format format = QImage::Format_Grayscale8;

        if(display_image.channels() == 3){
            cv::cvtColor(display_image, display_image, cv::COLOR_BGR2RGB);
            format = QImage::Format_RGB888;
        }else if (display_image.channels() == 4){
            cv::cvtColor(display_image, display_image, cv::COLOR_BGRA2RGBA);
            format = QImage::Format_RGBA8888;
        }

        pixmap.fromImage(QImage(display_image.data, display_image.cols, display_image.rows, display_image.step, format));
    }

    if(pixmap.isNull()){
        qDebug() << "Null pixmap?";
    }else{

        currentImage->setImage(image);
        currentImage->setPixmap(pixmap);

        updateLabels();

        auto image_info = QFileInfo(current_imagepath);
        ui->imageBitDepthLabel->setText(QString("%1 bit").arg(image.elemSize() * 8));
        ui->filenameLabel->setText(image_info.fileName());
        ui->filetypeLabel->setText(image_info.completeSuffix());
        ui->sizeLabel->setText(QString("%1 kB").arg(image_info.size() / 1000));
        ui->dimensionsLabel->setText(QString("(%1, %2) px").arg(pixmap.width()).arg(pixmap.height()));
    }
}

void MainWindow::histogram(const cv::Mat &image, cv::Mat &hist){
    int histSize = 256;
    float range[] = { 0, 256 }; //the upper boundary is exclusive
    const float* histRange = { range };

    bool uniform = true;
    bool accumulate = true;

    calcHist( &image, 1, nullptr, cv::Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);
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

void MainWindow::addImages(void){
    QString openDir = QDir::homePath();
    QStringList image_filenames = QFileDialog::getOpenFileNames(this, tr("Select image(s)"),
                                                    openDir,
                                                    tr("JPEG (*.jpg, *.jpeg, *.JPG, *.JPEG);;PNG (*.png, *.PNG);;BMP (*.bmp, *.BMP);;TIFF (*.tif, *.tiff, *.TIF, *.TIFF);;All images (*.jpg, *.jpeg, *.png, *.bmp, *.tiff)"));

    if(image_filenames.size() != 0){
        QString path;

        QDialog image_load_progress(this);
        image_load_progress.setModal(true);
        image_load_progress.show();

        auto bar = new QProgressBar();
        bar->setMaximum(image_filenames.size());

        image_load_progress.setLayout(new QVBoxLayout());

        image_load_progress.layout()->addWidget(bar);
        int i=0;

        foreach(path, image_filenames){
            project->addImage(path);
            bar->setValue(i++);
        }

        image_load_progress.close();
    }

    updateImageList();
    initDisplay();

    return;
}

void MainWindow::addImageFolder(void){
    QString openDir = QDir::homePath();
    QString path = QFileDialog::getExistingDirectory(this, tr("Select image folder"),
                                                    openDir);

    if(path != ""){
        int number_added = project->addImageFolder(path);
        qDebug() << "Added: " << number_added << " images";
    }

    updateImageList();
    if(number_images != 0){
        current_index = 0;
        initDisplay();
    }

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
    }
}

void MainWindow::launchExportDialog(){
    export_dialog.open();
}

MainWindow::~MainWindow()
{
    delete ui;
}
