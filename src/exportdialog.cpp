#include "exportdialog.h"
#include "ui_exportdialog.h"

ExportDialog::ExportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportDialog)
{
    ui->setupUi(this);
    toggleExporter();
    do_shuffle = ui->randomSplitCheckbox->isChecked();
    validation_split_pc = ui->validationSplitSpinbox->value();

    connect(ui->exportSelectComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(toggleExporter()));
    connect(ui->randomSplitCheckbox, SIGNAL(clicked(bool)), this, SLOT(toggleShuffle(bool)));
    connect(ui->validationSplitSpinbox, SIGNAL(valueChanged(int)), this, SLOT(setValidationSplit(int)));
    connect(ui->exportLabelledCheckbox, SIGNAL(clicked(bool)), this, SLOT(setExportUnlabelled(bool)));
    connect(ui->gcpBucketLineEdit, SIGNAL(textEdited(QString)), SLOT(setBucketUri(QString)));

    connect(ui->namesFileLineEdit, SIGNAL(textEdited(QString)), SLOT(setNamesFile(QString)));
    connect(ui->namesFilePushButton, SIGNAL(clicked()), this, SLOT(setNamesFile()));
    connect(ui->outputFolderLineEdit, SIGNAL(textEdited(QString)), SLOT(setOutputFolder(QString)));
    connect(ui->outputFolderPushButton, SIGNAL(clicked()), this, SLOT(setOutputFolder()));

    settings = new QSettings("DeepLabel", "DeepLabel");

    ui->validationSplitSpinbox->setValue(settings->value("validation_split_pc", 80).toInt());
    toggleShuffle(settings->value("do_shuffle", false).toBool());
    setExportUnlabelled(settings->value("export_unlabelled", false).toBool());
    ui->exportLabelledCheckbox->setChecked(export_unlabelled);

    if(settings->contains("output_folder")){
        auto path = settings->value("output_folder").toString();
        if(path != ""){
            setOutputFolder(path);
        }
    }

    if(settings->contains("names_file")){
        auto path = settings->value("names_file").toString();
        if(path != ""){
            setNamesFile(path);
         }
    }
}

ExportDialog::~ExportDialog()
{
    delete settings;
    delete ui;
}

bool ExportDialog::getExportUnlablled(void){
    return export_unlabelled;
}

void ExportDialog::setExportUnlabelled(bool res){
    export_unlabelled = res;
    settings->setValue("export_unlabelled", export_unlabelled);
}

void ExportDialog::setValidationSplit(int value){
    if (value < 0 || value > 100) return;
    validation_split_pc = value;
    settings->setValue("validation_split_pc", validation_split_pc);
}

void ExportDialog::toggleShuffle(bool shuffle){
    do_shuffle = shuffle;
    settings->setValue("do_shuffle", do_shuffle);
    ui->randomSplitCheckbox->setChecked(do_shuffle);
}

void ExportDialog::setOutputFolder(QString path){

    if(path == ""){
        QString openDir;
        if(output_folder == ""){
             openDir = QDir::homePath();
        }else{
             openDir = output_folder;
        }

        path = QFileDialog::getExistingDirectory(this, tr("Select output folder"),
                                                        openDir);
    }

    if(path != ""){
        output_folder = path;
        ui->outputFolderLineEdit->setText(output_folder);
        settings->setValue("output_folder", output_folder);
    }

    checkOK();
}

void ExportDialog::setBucketUri(QString uri){
    if(uri != ""){
        bucket_uri = uri;
    }
}

void ExportDialog::setNamesFile(QString path){

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

bool ExportDialog::checkOK(){

    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    // If output folder exists
    if(!QDir(output_folder).exists() || output_folder == ""){
        qDebug() << "Export output folder doesn't exist";
        return false;
    }

    if(ui->exportSelectComboBox->currentText() == "GCP AutoML"){
        if(bucket_uri == "") return false;
    }

    if(ui->exportSelectComboBox->currentText() == "Darknet"){
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

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

bool ExportDialog::getCreateLabelMap(void){
    return ui->labelMapCheckbox->isChecked();
}

void ExportDialog::toggleExporter(){

    current_exporter = ui->exportSelectComboBox->currentText();

    ui->namesFileLineEdit->setEnabled(current_exporter == "Darknet");
    ui->namesFilePushButton->setEnabled(current_exporter == "Darknet");
    ui->labelMapCheckbox->setEnabled(current_exporter == "Pascal VOC");
    ui->gcpBucketLineEdit->setEnabled(current_exporter == "GCP AutoML");

    checkOK();
}
