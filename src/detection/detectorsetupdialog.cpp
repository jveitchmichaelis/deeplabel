#include "detectorsetupdialog.h"
#include "ui_detectorsetupdialog.h"

DetectorSetupDialog::DetectorSetupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DetectorSetupDialog)
{
    ui->setupUi(this);

    connect(ui->cfgPathButton, SIGNAL(clicked()), this, SLOT(setCfgFile()));
    connect(ui->cfgPathLineEdit, SIGNAL(editingFinished()), this, SLOT(checkForm()));
    connect(ui->weightPathButton, SIGNAL(clicked(bool)), this, SLOT(setWeightsFile()));
    connect(ui->weightPathLineEdit, SIGNAL(editingFinished()), this, SLOT(checkForm()));
    connect(ui->namesPathButton, SIGNAL(clicked(bool)), this, SLOT(setNamesFile()));
    connect(ui->namesPathLineEdit, SIGNAL(editingFinished()), this, SLOT(checkForm()));

    settings = new QSettings("DeepLabel", "DeepLabel");

    darknet_cfg_file = settings->value("darknet_cfg", "").toString();
    darknet_weight_file = settings->value("darknet_weights", "").toString();
    darknet_names_file = settings->value("darknet_names", "").toString();

    image_width = settings->value("darknet_width", 0).toInt();
    image_height = settings->value("darknet_height", 0).toInt();
    image_channels = settings->value("darknet_channels", 0).toInt();

    updateFields();
    checkForm();

}

void DetectorSetupDialog::updateFields(){
    ui->cfgPathLineEdit->setText(darknet_cfg_file);
    ui->weightPathLineEdit->setText(darknet_weight_file);
    ui->namesPathLineEdit->setText(darknet_names_file);

    ui->imageWidthLabel->setText(QString::number(image_width));
    ui->imageHeightLabel->setText(QString::number(image_height));
    ui->imageChannelsLabel->setText(QString::number(image_channels));
}

void DetectorSetupDialog::checkForm(void){

    // Don't bother checking if nothing changed
    if(darknet_cfg_file == ui->cfgPathLineEdit->text() &&
       darknet_weight_file == ui->weightPathLineEdit->text() &&
       darknet_names_file == ui->namesPathLineEdit->text()) return;

    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    darknet_cfg_file = ui->cfgPathLineEdit->text();
    darknet_weight_file = ui->weightPathLineEdit->text();
    darknet_names_file = ui->namesPathLineEdit->text();

    if(darknet_cfg_file == "") return;
    if(darknet_weight_file == "") return;
    if(darknet_names_file == "") return;

    if(!QFile(darknet_cfg_file).exists()){
        qDebug() << "Config file doesn't exist";
        return;
    }

    if(!QFile(darknet_weight_file).exists()){
        qDebug() << "Weight file doesn't exist";
        return;
    }

    if(!QFile(darknet_names_file).exists()){
        qDebug() << "Names file doesn't exist";
        return;
    }

    getParamsFromConfig();

}

void DetectorSetupDialog::getParamsFromConfig(void){

    qDebug() << "Checking config file";

    QSettings darknet_settings(darknet_cfg_file, QSettings::IniFormat);

    darknet_settings.beginGroup("net");

    auto keys = darknet_settings.childKeys();

    if(!darknet_settings.contains("width")){
        qDebug() << "No width parameter";
        return;
    }
    if(!darknet_settings.contains("height")){
        qDebug() << "No height parameter";
        return;
    }
    if(!darknet_settings.contains("channels")){
        qDebug() << "No channels parameter";
        return;
    }

    auto width = darknet_settings.value("width").toInt();
    auto height = darknet_settings.value("height").toInt();
    auto channels = darknet_settings.value("channels").toInt();

    darknet_settings.endGroup();

    if(width > 0 && height > 0 && channels > 0){

        qDebug() << width << height << channels;

        image_width = width;
        image_height = height;
        image_channels = channels;

        settings->setValue("darknet_width", image_width);
        settings->setValue("darknet_height", image_height);
        settings->setValue("darknet_channels", image_channels);

        settings->setValue("darknet_cfg", darknet_cfg_file);
        settings->setValue("darknet_weights", darknet_weight_file);
        settings->setValue("darknet_names", darknet_names_file);

        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }

    updateFields();

}

QString DetectorSetupDialog::openFile(QString title, QString search_path, QString filter){
    QString openDir;

    if(search_path == ""){
         openDir = settings->value("project_folder").toString();
    }else{
         openDir = QFileInfo(search_path).absoluteDir().absolutePath();
    }

    auto path = QFileDialog::getOpenFileName(this, title,
                                             openDir,
                                             QString("All files (*.*);;%1").arg(filter),
                                             &filter);

    return path;
}

void DetectorSetupDialog::setCfgFile(void){

    QString filter = tr("Config (*.cfg)");
    QString title = tr("Select darknet config file");

    auto path = openFile(title, darknet_cfg_file, filter);

    if(path != ""){
        ui->cfgPathLineEdit->setText(path);
    }

    checkForm();
}


void DetectorSetupDialog::setWeightsFile(void){

    QString filter = tr("Weights (*.weights)");
    QString title = tr("Select darknet weights file");

    auto path = openFile(title, darknet_weight_file, filter);

    if(path != ""){
        ui->weightPathLineEdit->setText(path);
    }

    checkForm();
}

int DetectorSetupDialog::getTarget(void){
    if(ui->targetComboBox->currentText() == "CPU"){
        return cv::dnn::DNN_TARGET_CPU;
    }else if(ui->targetComboBox->currentText() == "OpenCL"){
        return cv::dnn::DNN_TARGET_OPENCL;
    }else if(ui->targetComboBox->currentText() == "OpenCL FP16"){
        return cv::dnn::DNN_TARGET_OPENCL_FP16;
    }
}

void DetectorSetupDialog::setNamesFile(void){

    QString filter = tr("Names (*.names)");
    QString title = tr("Select darknet names file");

    auto path = openFile(title, darknet_names_file, filter);

    if(path != ""){
        ui->namesPathLineEdit->setText(path);
    }

    checkForm();
}

DetectorSetupDialog::~DetectorSetupDialog()
{
    delete ui;
}
