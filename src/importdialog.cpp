#include "importdialog.h"
#include "ui_importdialog.h"

ImportDialog::ImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportDialog)
{
    ui->setupUi(this);
    toggleImporter();
    connect(ui->importSelectComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(toggleImporter()));
    connect(ui->importLabelledCheckbox, SIGNAL(clicked(bool)), this, SLOT(setImportUnlabelled(bool)));

    connect(ui->namesFileLineEdit, SIGNAL(textEdited(QString)), SLOT(setNamesFile(QString)));
    connect(ui->namesFilePushButton, SIGNAL(clicked()), this, SLOT(setNamesFile()));
    connect(ui->inputListLineEdit, SIGNAL(textEdited(QString)), SLOT(setInputFile(QString)));
    connect(ui->inputListPushButton, SIGNAL(clicked()), this, SLOT(setInputFile()));

    settings = new QSettings("DeepLabel", "DeepLabel");

    setImportUnlabelled(settings->value("import_unlabelled", false).toBool());
    ui->importLabelledCheckbox->setChecked(import_unlabelled);

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

void ImportDialog::setInputFile(QString path){

    if(path == ""){
        QString openDir;
        if(input_file == ""){
             openDir = QDir::homePath();
        }else{
             openDir = QDir(input_file).path();
        }

        path = QFileDialog::getOpenFileName(this, tr("Select input file"),
                                                        openDir);
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

bool ImportDialog::checkOK(){

    // If output folder exists
    if(!QDir(input_file).exists() || input_file == ""){
        qDebug() << "Import file/folder doesn't exist";
        return false;
    }

    if(ui->importSelectComboBox->currentText() == "Darknet"){
        // If we're using darknet, check the names
        // file exists and contains something
        if(!QFile::exists(names_file)){
            qDebug() << "Names file doesn't exist";
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
            qDebug() << "No classes?";
            return false;
        }
    }

    return true;
}


void ImportDialog::toggleImporter(){

    current_importer = ui->importSelectComboBox->currentText();

    ui->namesFileLineEdit->setEnabled(current_importer == "Darknet");

    checkOK();
}

