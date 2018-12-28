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

    connect(ui->namesFilePushButton, SIGNAL(clicked()), this, SLOT(setNamesFile()));
    connect(ui->outputFolderPushButton, SIGNAL(clicked()), this, SLOT(setOutputFolder()));
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

void ExportDialog::setValidationSplit(int value){
    if (value < 0 || value > 100) return;
    validation_split_pc = value;
}

void ExportDialog::toggleShuffle(bool shuffle){
    do_shuffle = shuffle;
}

void ExportDialog::setOutputFolder(){

    QString openDir = QDir::homePath();
    QString path = QFileDialog::getExistingDirectory(this, tr("Select output folder"),
                                                    openDir);

    if(path != ""){
        output_folder = path;
        ui->outputFolderLineEdit->setText(output_folder);
    }

    checkOK();
}

void ExportDialog::setNamesFile(){

    QString openDir = QDir::homePath();
    QString path = QFileDialog::getOpenFileName(this, tr("Select darknet names file"),
                                                    openDir);

    if(path != ""){
        names_file = path;
        ui->namesFileLineEdit->setText(names_file);
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
