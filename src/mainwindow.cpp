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

    currentImage = new ImageLabel(this);
    ui->scrollAreaWidgetContents->layout()->setAlignment(Qt::AlignHCenter);
    ui->scrollAreaWidgetContents->layout()->setAlignment(Qt::AlignVCenter);
    ui->scrollAreaWidgetContents->layout()->addWidget(currentImage);
    connect(this, SIGNAL(selectedClass(QString)), currentImage, SLOT(setClassname(QString)));
    connect(currentImage, SIGNAL(newLabel(BoundingBox)), this, SLOT(addLabel(BoundingBox)));
    connect(currentImage, SIGNAL(removeLabel(BoundingBox)), this, SLOT(removeLabel(BoundingBox)));

    connect(ui->actionDraw_Tool, SIGNAL(triggered(bool)), currentImage, SLOT(setDrawMode()));
    connect(ui->actionSelect_Tool, SIGNAL(triggered(bool)), currentImage, SLOT(setSelectMode()));
    connect(ui->actionDraw_Tool, SIGNAL(triggered(bool)), this, SLOT(setDrawMode()));
    connect(ui->actionSelect_Tool, SIGNAL(triggered(bool)), this, SLOT(setSelectMode()));

    project = new LabelProject(this);
}

void MainWindow::setDrawMode(){
    ui->actionDraw_Tool->setChecked(true);
    ui->actionSelect_Tool->setChecked(false);
}

void MainWindow::setSelectMode(){
    ui->actionDraw_Tool->setChecked(false);
    ui->actionSelect_Tool->setChecked(true);
}

void MainWindow::openProject()
{
    QString openDir = QDir::homePath();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
                                                    openDir,
                                                    tr("Label database (*.lbldb)"));

    if(fileName != ""){
        project->loadDatabase(fileName);
        initDisplay();
    }

    return;
}

void MainWindow::updateImageList(){
    project->getImageList(images);
    number_images = images.size();
}

void MainWindow::updateClassList(){
    project->getClassList(classes);

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
        updateClassList();
    }
}

void MainWindow::addLabel(BoundingBox bbox){
    project->addLabel(current_imagepath, bbox);
}

void MainWindow::removeLabel(BoundingBox bbox){
    project->removeLabel(current_imagepath, bbox);

    QList<BoundingBox> bboxes;
    project->getLabels(current_imagepath, bboxes);
    currentImage->setBoundingBoxes(bboxes);
}

void MainWindow::initDisplay(){

    updateImageList();
    updateClassList();

    if(number_images != 0){
        current_index = 0;
        current_imagepath = images.at(current_index);
        display(current_imagepath);
    }
}

void MainWindow::nextImage(){

    if(images.empty()) return;

    if(current_index == number_images){
        if(wrap_index){
            current_index = 0;
        }else{
            return;
        }
    }else{
        current_index++;
    }

    current_imagepath = images.at(current_index);
    display(images.at(current_index));
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

    current_imagepath = images.at(current_index);
    display(images.at(current_index));
}

void MainWindow::display(QString fileName){

    pixmap.load(fileName);

    if(pixmap.isNull()){
        qDebug() << "Null pixmap?";
    }else{

        currentImage->setPixmap(pixmap);
        QList<BoundingBox> bboxes;
        project->getLabels(fileName, bboxes);
        currentImage->setBoundingBoxes(bboxes);

        auto image_info = QFileInfo(fileName);
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
    if(number_images == 0){
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

MainWindow::~MainWindow()
{
    delete ui;
}
