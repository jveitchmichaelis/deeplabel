#include "importdialog.h"
#include "ui_importdialog.h"

ImportDialog::ImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportDialog)
{
    ui->setupUi(this);

    connect(ui->importSelectComboBox,
            SIGNAL(currentIndexChanged(QString)),
            this,
            SLOT(toggleImporter()));
    connect(ui->importLabelledCheckbox,
            SIGNAL(clicked(bool)),
            this,
            SLOT(setImportUnlabelled(bool)));
    connect(ui->relativeImageCheckbox, SIGNAL(clicked(bool)), this, SLOT(setRelativePaths(bool)));
    connect(ui->namesFileLineEdit, SIGNAL(textEdited(QString)), SLOT(setNamesFile(QString)));
    connect(ui->namesFilePushButton, SIGNAL(clicked()), this, SLOT(setNamesFile()));
    connect(ui->inputListLineEdit, SIGNAL(textEdited(QString)), SLOT(setInputFile(QString)));
    connect(ui->inputListPushButton, SIGNAL(clicked()), this, SLOT(setInputFile()));
    connect(ui->relativeImageRootLineEdit,
            SIGNAL(textEdited(QString)),
            SLOT(setRelativeImagePath(QString)));
    connect(ui->relativeImageRootPushButton, SIGNAL(clicked()), this, SLOT(setRelativeImagePath()));
    connect(ui->annotationLineEdit, SIGNAL(textEdited(QString)), SLOT(setAnnotationFile(QString)));
    connect(ui->annotationPushButton, SIGNAL(clicked()), this, SLOT(setAnnotationFile()));

    settings = new QSettings("DeepLabel", "DeepLabel");

    setImportUnlabelled(settings->value("import_unlabelled", false).toBool());
    ui->importLabelledCheckbox->setChecked(import_unlabelled);

    setRelativePaths(settings->value("relative_image_paths", false).toBool());
    ui->relativeImageCheckbox->setChecked(relative_image_paths);

    if(settings->contains("import_format")){
        auto format = settings->value("import_format").toString();
        if(format != ""){
            setImporter(format);
        }
    }else{
        setImporter("Darknet");
    }

    if(settings->contains("input_file")){
        auto path = settings->value("input_file").toString();
        if(path != ""){
            setInputFile(path);
        }
    }

    if(settings->contains("names_file")){
        auto path = settings->value("names_file").toString();
        if(path != ""){
            setNamesFile(path);
         }
    }

    if(settings->contains("annotation_file")){
        auto path = settings->value("annotation_file").toString();
        if(path != ""){
            setAnnotationFile(path);
         }
    }

    if (settings->contains("relative_root")) {
        auto path = settings->value("relative_root").toString();
        if (path != "") {
            setRelativeImagePath(path);
        }
    }
}

ImportDialog::~ImportDialog()
{
    delete ui;
}

bool ImportDialog::getImportUnlabelled(void){
    return import_unlabelled;
}

void ImportDialog::setImportUnlabelled(bool res){
    import_unlabelled = res;
    settings->setValue("import_unlabelled", import_unlabelled);
}

void ImportDialog::setRelativePaths(bool res)
{
    relative_image_paths = res;
    settings->setValue("relative_image_paths", relative_image_paths);
}

void ImportDialog::setRelativeImagePath(QString path)
{
    if (path == "") {
        QString openDir;
        if (relative_root == "") {
            openDir = QDir::homePath();
        } else {
            openDir = QDir(relative_root).path();
        }

        path = QFileDialog::getExistingDirectory(this, tr("Select relative root folder"), openDir);
    }

    if (path != "") {
        relative_root = path;
        ui->relativeImageRootLineEdit->setText(relative_root);
        settings->setValue("relative_root", relative_root);
    }

    checkOK();
}

void ImportDialog::setInputFile(QString path){

    if(path == ""){
        QString openDir;
        if(input_file == ""){
             openDir = QDir::homePath();
        }else{
             openDir = QDir(input_file).path();
        }

        if(current_importer == "MOT"
           || current_importer == "BirdsAI"){
            path = QFileDialog::getExistingDirectory(this, tr("Select sequence folder"),
                                                            openDir);
        }else if(current_importer == "Coco"
                 || current_importer == "PascalVOC"){
                  path = QFileDialog::getExistingDirectory(this, tr("Select image folder"),
                                                                  openDir);
        }else{
            path = QFileDialog::getOpenFileName(this, tr("Select input file"),
                                                            openDir);
        }
    }

    if(path != ""){
        input_file = path;
        ui->inputListLineEdit->setText(input_file);
        settings->setValue("input_file", input_file);
    }

    checkOK();
}

void ImportDialog::setNamesFile(QString path){

    if(path == ""){
        QString openDir;

        if(names_file == ""){
             openDir = QDir::homePath();
        }else{
             openDir = QFileInfo(names_file).absoluteDir().absolutePath();
        }

        path = QFileDialog::getOpenFileName(this, tr("Select darknet names file"),
                                                        openDir);
    }

    if(path != ""){
        names_file = path;
        ui->namesFileLineEdit->setText(names_file);
        settings->setValue("names_file", names_file);
    }

    checkOK();
}

void ImportDialog::setAnnotationFile(QString path){

    if(path == ""){
        QString openDir;

        if(annotation_file == ""){
             openDir = QDir::homePath();
        }else{
             openDir = QFileInfo(annotation_file).absoluteDir().absolutePath();
        }

        if(current_importer == "BirdsAI" || current_importer == "PascalVOC"){
            path = QFileDialog::getExistingDirectory(this, tr("Select annotation folder"),
                                                            openDir);
        }else{
            path = QFileDialog::getOpenFileName(this, tr("Select annotation file"),
                                                            openDir);
        }
    }

    if(path != ""){
        annotation_file = path;
        ui->annotationLineEdit->setText(annotation_file);
        settings->setValue("annotation_file", annotation_file);
    }

    checkOK();
}

bool ImportDialog::checkOK(){

    ui->namesFileLineEdit->setEnabled(current_importer != "Coco");

    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    if(current_importer == "Coco"){
        ui->namesFileLineEdit->setDisabled(true);
        ui->namesFilePushButton->setDisabled(true);
        ui->inputListLineEdit->setEnabled(true);
        ui->inputListPushButton->setEnabled(true);
        ui->relativeImageRootLineEdit->setEnabled(false);
        ui->relativeImageCheckbox->setEnabled(false);
        ui->relativeImageRootPushButton->setEnabled(false);
        ui->annotationLineEdit->setEnabled(true);
        ui->annotationPushButton->setEnabled(true);

        ui->ImagesLabel->setText("Image folder");

        if(!QFile::exists(annotation_file)){
            //qCritical() << "Annotation file doesn't exist";
            return false;
        }
    }

    if(current_importer == "PascalVOC"){
        ui->namesFileLineEdit->setDisabled(true);
        ui->namesFilePushButton->setDisabled(true);
        ui->inputListLineEdit->setEnabled(true);
        ui->inputListPushButton->setEnabled(true);
        ui->relativeImageRootLineEdit->setEnabled(false);
        ui->relativeImageCheckbox->setEnabled(false);
        ui->relativeImageRootPushButton->setEnabled(false);
        ui->annotationLineEdit->setEnabled(true);
        ui->annotationPushButton->setEnabled(true);

        ui->ImagesLabel->setText("Image folder");

        if(!QFile::exists(annotation_file)){
            //qCritical() << "Annotation file doesn't exist";
            return false;
        }
    }

    if(current_importer == "MOT" ||
        ui->importSelectComboBox->currentText() == "BirdsAI"){
        ui->namesFileLineEdit->setEnabled(true);
        ui->namesFilePushButton->setEnabled(true);
        ui->inputListLineEdit->setEnabled(true);
        ui->inputListPushButton->setEnabled(true);
        ui->relativeImageRootLineEdit->setEnabled(false);
        ui->relativeImageCheckbox->setEnabled(false);
        ui->relativeImageRootPushButton->setEnabled(false);

        bool is_mot = (current_importer == "MOT");
        ui->annotationLineEdit->setDisabled(is_mot);
        ui->annotationPushButton->setDisabled(is_mot);

        if(!QDir(annotation_file).exists() && !is_mot){
            //qCritical() << "Annotation folder doesn't exist";
            return false;
        }

        ui->ImagesLabel->setText("Image folder");

        if(!checkNamesFile(names_file))
            return false;

    }

    if(current_importer == "Darknet"){
        ui->namesFileLineEdit->setEnabled(true);
        ui->namesFilePushButton->setEnabled(true);
        ui->inputListLineEdit->setEnabled(true);
        ui->inputListPushButton->setEnabled(true);
        ui->relativeImageRootLineEdit->setEnabled(true);
        ui->relativeImageCheckbox->setEnabled(true);
        ui->relativeImageRootPushButton->setEnabled(true);
        ui->annotationLineEdit->setDisabled(true);
        ui->annotationPushButton->setDisabled(true);

        ui->ImagesLabel->setText("Image list");

        if(!checkNamesFile(names_file))
            return false;
    }

    // If input file exists
    if(!QFile(input_file).exists() || input_file == ""){
        //qCritical() << "Import file/folder doesn't exist";
        return false;
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    return true;
}

bool ImportDialog::checkNamesFile(QString names_file){
    // If we're using darknet, check the names
    // file exists and contains something
    if(!QFile::exists(names_file)){
        //qCritical() << "Names file doesn't exist";
        return false;
    }

    QStringList class_list;
    QFile fh(names_file);

    if (fh.open(QIODevice::ReadOnly)) {

        while (!fh.atEnd()) {
            // Darknet name file is just a newline delimited list of classes
            QByteArray line = fh.readLine();
            class_list.append(line);
        }
    }

    if(class_list.size() == 0){
        //qCritical() << "No classes found";
        return false;
    }

    return true;
}

void ImportDialog::setImporter(QString format){
    if(format == ""){
        current_importer = ui->importSelectComboBox->currentText();
        settings->setValue("import_format", current_importer);
    }else{
        ui->importSelectComboBox->setCurrentText(format);
        current_importer = format;
    }

    checkOK();
}

void ImportDialog::toggleImporter(QString format){
    setImporter(format);
}

