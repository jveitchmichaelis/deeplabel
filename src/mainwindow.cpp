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

    connect(ui->actionInit_Tracking, SIGNAL(triggered(bool)), this, SLOT(setupTracking()));
    connect(ui->actionPropagate_Tracking, SIGNAL(triggered(bool)), this, SLOT(propagateTracking()));
    connect(ui->propagateCheckBox, SIGNAL(clicked(bool)), this, SLOT(toggleAutoPropagate(bool)));

    currentImage = new ImageLabel(this);
    ui->scrollAreaWidgetContents->layout()->setAlignment(Qt::AlignHCenter);
    ui->scrollAreaWidgetContents->layout()->setAlignment(Qt::AlignVCenter);
    ui->scrollAreaWidgetContents->layout()->addWidget(currentImage);

    connect(this, SIGNAL(selectedClass(QString)), currentImage, SLOT(setClassname(QString)));
    connect(currentImage, SIGNAL(newLabel(BoundingBox)), this, SLOT(addLabel(BoundingBox)));
    connect(currentImage, SIGNAL(removeLabel(BoundingBox)), this, SLOT(removeLabel(BoundingBox)));

    connect(ui->removeClassButton, SIGNAL(clicked(bool)), this, SLOT(removeClass()));
    connect(ui->removeImageButton, SIGNAL(clicked(bool)), this, SLOT(removeImage()));

    ui->actionDraw_Tool->setChecked(true);
    connect(ui->actionDraw_Tool, SIGNAL(triggered(bool)), this, SLOT(setDrawMode()));
    connect(ui->actionSelect_Tool, SIGNAL(triggered(bool)), this, SLOT(setSelectMode()));

    connect(ui->classComboBox, SIGNAL(currentIndexChanged(QString)), currentImage, SLOT(setClassname(QString)));
    connect(ui->changeImageButton, SIGNAL(clicked(bool)), this, SLOT(changeImage()));
    connect(ui->imageNumberSpinbox, SIGNAL(editingFinished()), this, SLOT(changeImage()));

    connect(ui->actionWrap_images, SIGNAL(triggered(bool)), this, SLOT(enableWrap(bool)));
    connect(ui->actionExport, SIGNAL(triggered(bool)), this, SLOT(exportData()));

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
}

void MainWindow::toggleAutoPropagate(bool state){
    track_previous = state;
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
    QString openDir = QDir::homePath();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
                                                    openDir,
                                                    tr("Label database (*.lbldb)"));

    if(fileName != ""){
        if(project->loadDatabase(fileName)){
            initDisplay();
        }else{
            QMessageBox::warning(this,tr("Remove Image"), tr("Failed to open project."));
        }
    }

    return;
}

void MainWindow::propagateTracking(){

    // If there are no labels, and we're tracking the previous frame
    // propagate the bounding boxes. Otherwise we assume that the
    // current labels are the correct ones and should override.
    for(auto& bbox_class : classes){

        // Skip uninitialised tracker
        if(tracker_map.find(bbox_class.toStdString()) == tracker_map.end()) continue;

        qDebug() << "Updating tracker: " << bbox_class;

        auto tracker = tracker_map[bbox_class.toStdString()];
        bool res = tracker->update(currentImage->getImage());

        if(!res) continue;

        // Add the new object labels
        for(auto &bbox : tracker->getObjects()){

            cv::imwrite("bbox_post.png", currentImage->getImage()(bbox));

            QRect new_bbox;
            new_bbox.setX(bbox.x);
            new_bbox.setY(bbox.y);
            new_bbox.setWidth(bbox.width);
            new_bbox.setHeight(bbox.height);

            currentImage->addLabel(new_bbox, bbox_class);
        }
    }

    updateLabels();
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
    }
}

cv::Rect2d MainWindow::qrect2cv(QRect rect){
    return cv::Rect2d(rect.x(), rect.y(), rect.width(),rect.height());
}

void MainWindow::setupTracking(){
    auto bboxes = currentImage->getBoundingBoxes();

    // If we are tracking and we have some labelled boxes already
    if(bboxes.size() > 0){

        // Make a tracker per label class
        for(auto &label : classes){
            tracker_map[label.toStdString()] = cv::MultiTracker::create();
        }

        // Add objects to the multi-tracker
        for(auto &bbox : bboxes){
            auto tracker = createTrackerByName(CSRT);
            tracker_map[bbox.classname.toStdString()]->add(tracker, currentImage->getImage(), qrect2cv(bbox.rect));
            //cv::imwrite("bbox_pre.png", currentImage->getImage()(qrect2cv(bbox.rect)));
        }

    }
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

    current_imagepath = images.at(current_index);
    pixmap.load(current_imagepath);

    auto image = cv::imread(current_imagepath.toStdString(), cv::IMREAD_UNCHANGED|cv::IMREAD_ANYDEPTH);

    if(image.empty()) return;

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

        ui->imageProgressBar->setValue(current_index+1);
        ui->imageNumberSpinbox->setValue(current_index+1);
        ui->imageIndexLabel->setText(QString("%1/%2").arg(current_index+1).arg(number_images));

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
    QString openDir = QDir::homePath();
    QString fileName = QFileDialog::getSaveFileName(this, tr("New Project"),
                                                    openDir,
                                                    tr("Label database (*.lbldb)"));

    if(fileName != ""){
        project->createDatabase(fileName);
    }

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
    if(number_images == 0){
        initDisplay();
    }

    return;
}

void MainWindow::exportData(){

    QString openDir = QDir::homePath();
    QString path = QFileDialog::getExistingDirectory(this, tr("Select output folder"),
                                                    openDir);

    if(path != ""){
        KittiExporter exporter(project);
        QThread *export_thread = new QThread;

        exporter.moveToThread(export_thread);

        exporter.setOutputFolder(path);
        exporter.splitData();
        exporter.process();
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}
