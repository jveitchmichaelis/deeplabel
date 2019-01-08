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

    connect(ui->namesFileLineEdit, SIGNAL(textEdited(QString)), SLOT(setNamesFile(QString)));
    connect(ui->namesFilePushButton, SIGNAL(clicked()), this, SLOT(setNamesFile()));
    connect(ui->outputFolderLineEdit, SIGNAL(textEdited(QString)), SLOT(setOutputFolder(QString)));
    connect(ui->outputFolderPushButton, SIGNAL(clicked()), this, SLOT(setOutputFolder()));

    settings = new QSettings("DeepLabel", "DeepLabel");

    setValidationSplit(settings->value("validation_split_pc", 80).toInt());
    toggleShuffle(settings->value("do_shuffle", false).toBool());

    if(settings->contains("output_folder")){
        auto path = settings->value("output_folder").toString();
        if(path != ""){
            setOutputFolder(path);
        }
    }

    if(settings->contains("names_file")){
        auto path = settings->value("names_file").toString();
        if(path != ""){
            setNamesFile();
         }
    }
}

ExportDialog::~ExportDialog()
{
    delete settings;
    delete ui;
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

void ExportDialog::checkOK(){

    // If output folder exists
    if(!QDir(output_folder).exists() || output_folder != ""){
        return;
    }

    if(ui->exportSelectComboBox->currentText() == "Darknet"){
        // If we're using darknet, check the names
        // file exists and contains something
        if(!QFile::exists(names_file)) return;

        QStringList class_list;
        QFile fh(names_file);

        if (fh.open(QIODevice::ReadOnly)) {

            while (!fh.atEnd()) {
                // Darknet name file is just a newline delimited list of classes
                QByteArray line = fh.readLine();
                class_list.append(line);
            }
        }

        if(class_list.size() == 0) return;
    }
}

void ExportDialog::toggleExporter(){

    current_exporter = ui->exportSelectComboBox->currentText();

    if(current_exporter == "Darknet"){
        ui->namesFileLineEdit->setEnabled(true);
        ui->namesFilePushButton->setEnabled(true);
    }else{
        ui->namesFileLineEdit->setEnabled(false);
        ui->namesFilePushButton->setEnabled(false);
    }



    checkOK();
}
