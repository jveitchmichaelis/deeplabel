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

void MainWindow::enableWrap(bool enable){
    wrap_index = enable;
}


void MainWindow::changeImage(){
    current_index = ui->imageNumberSpinbox->value();
    display();
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
            ui->imageGroupBox->setEnabled(true);
            ui->labelGroupBox->setEnabled(true);
            ui->navigationGroupBox->setEnabled(true);
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
    ui->imageProgressBar->setMaximum(number_images-1);
    ui->imageNumberSpinbox->setMaximum(number_images-1);
}

void MainWindow::updateClassList(){
    project->getClassList(classes);

    ui->classComboBox->clear();

    QString classname;
    foreach(classname, classes){
        ui->classComboBox->addItem(classname);
    }

    if(classes.size() > 0){
        current_class = ui->classComboBox->currentText();
        emit selectedClass(current_class);
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
        display();
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

    display();
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

    display();
}

void MainWindow::display(){

    current_imagepath = images.at(current_index);
    pixmap.load(current_imagepath);

    if(pixmap.isNull()){
        qDebug() << "Null pixmap?";
    }else{

        currentImage->setPixmap(pixmap);
        updateLabels();

        ui->imageProgressBar->setValue(current_index);
        ui->imageNumberSpinbox->setValue(current_index);
        ui->imageIndexLabel->setText(QString("%1/%2").arg(current_index).arg(number_images));

        auto image_info = QFileInfo(current_imagepath);
        ui->filenameLabel->setText(image_info.fileName());
        ui->filetypeLabel->setText(image_info.completeSuffix());
        ui->sizeLabel->setText(QString("%1 kB").arg(image_info.size() / 1000));
        ui->dimensionsLabel->setText(QString("(%1, %2) px").arg(pixmap.width()).arg(pixmap.height()));
    }


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
                                                    tr("Images (*.jpg, *.jpeg, *.png, *.bmp, *.tiff)"));

    if(image_filenames.size() != 0){
        QString path;
        foreach(path, image_filenames){
            project->addImage(path);
        }
    }

    updateImageList();

    if(number_images > 0){
        initDisplay();
    }

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
