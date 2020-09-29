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
    connect(ui->frameworkComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setFramework()));
    connect(ui->targetComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setTarget()));
    connect(ui->namesPathLineEdit, SIGNAL(editingFinished()), this, SLOT(checkForm()));
    connect(ui->convertGrayscaleCheckbox, SIGNAL(clicked(bool)), this, SLOT(setConvertGrayscale()));
    connect(ui->convertDepthCheckbox, SIGNAL(clicked(bool)), this, SLOT(setConvertDepth()));

    settings = new QSettings("DeepLabel", "DeepLabel");

    cfg_file = settings->value("model_cfg", "").toString();
    weight_file = settings->value("model_weights", "").toString();
    names_file = settings->value("model_names", "").toString();

    convert_grayscale = settings->value("model_convert_grayscale", true).toBool();
    ui->convertGrayscaleCheckbox->setChecked(convert_grayscale);

    convert_depth = settings->value("model_convert_depth", true).toBool();
    ui->convertDepthCheckbox->setChecked(convert_depth);

    image_width = settings->value("model_width", 0).toInt();
    image_height = settings->value("model_height", 0).toInt();
    image_channels = settings->value("model_channels", 3).toInt();

    framework = static_cast<model_framework>(settings->value("model_framework", 0).toInt());
    if(framework == FRAMEWORK_DARKNET){
        ui->frameworkComboBox->setCurrentText("Darknet (YOLO)");
        ui->imageHeightLabel->show();
        ui->imageWidthLabel->show();
    }else if(framework == FRAMEWORK_TENSORFLOW){
        ui->frameworkComboBox->setCurrentText("Tensorflow");
        ui->imageHeightLabel->hide();
        ui->imageWidthLabel->hide();
    }

    target = settings->value("model_target", cv::dnn::DNN_TARGET_CPU).toInt();
    if(target == cv::dnn::DNN_TARGET_CPU){
        ui->targetComboBox->setCurrentText("CPU");
    }else if(target == cv::dnn::DNN_TARGET_OPENCL){
        ui->targetComboBox->setCurrentText("OpenCL");
    }else if(target == cv::dnn::DNN_TARGET_OPENCL_FP16){
        ui->targetComboBox->setCurrentText("OpenCL FP16");
    }
#ifdef WITH_CUDA
    else if(target == cv::dnn::DNN_TARGET_CUDA){
        ui->targetComboBox->setCurrentText("CUDA");
    }else if(target == cv::dnn::DNN_TARGET_CUDA_FP16){
        ui->targetComboBox->setCurrentText("CUDA FP16");
    }
#endif
    updateFields();
    checkForm();

}

void DetectorSetupDialog::updateFields(){
    ui->cfgPathLineEdit->setText(cfg_file);
    ui->weightPathLineEdit->setText(weight_file);
    ui->namesPathLineEdit->setText(names_file);

    ui->imageWidthLabel->setText(QString::number(image_width));
    ui->imageHeightLabel->setText(QString::number(image_height));
    ui->imageChannelsLabel->setText(QString::number(image_channels));
}

void DetectorSetupDialog::setConvertGrayscale(void){
    convert_grayscale = ui->convertGrayscaleCheckbox->isChecked();
    settings->setValue("model_convert_grayscale", convert_grayscale);
}

bool DetectorSetupDialog::getConvertGrayscale(void){
    return convert_grayscale;
}

void DetectorSetupDialog::setConvertDepth(void){
    convert_depth = ui->convertDepthCheckbox->isChecked();
    settings->setValue("model_convert_depth", convert_depth);
}

bool DetectorSetupDialog::getConvertDepth(void){
    return convert_depth;
}

void DetectorSetupDialog::checkForm(void){

    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    cfg_file = ui->cfgPathLineEdit->text();
    weight_file = ui->weightPathLineEdit->text();
    names_file = ui->namesPathLineEdit->text();

    if(cfg_file == "") return;
    if(weight_file == "") return;
    if(names_file == "") return;

    if(!QFile(cfg_file).exists()){
        qDebug() << "Config file doesn't exist";
        return;
    }else if(!getParamsFromConfig()){
        return;
    }

    if(!QFile(weight_file).exists()){
        qDebug() << "Weight file doesn't exist";
        return;
    }

    if(!QFile(names_file).exists()){
        qDebug() << "Names file doesn't exist";
        return;
    }

    // At this point, all good.
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    settings->setValue("model_width", image_width);
    settings->setValue("model_height", image_height);
    settings->setValue("modelchannels", image_channels);

    settings->setValue("model_cfg", cfg_file);
    settings->setValue("model_weights", weight_file);
    settings->setValue("model_names", names_file);

}

bool DetectorSetupDialog::getParamsFromConfig(void){

    qDebug() << "Checking config file";

    if(framework == FRAMEWORK_DARKNET){
        QSettings darknet_settings(cfg_file, QSettings::IniFormat);

        darknet_settings.beginGroup("net");

        auto keys = darknet_settings.childKeys();

        if(!darknet_settings.contains("width")){
            qDebug() << "No width parameter";
            return false;
        }
        if(!darknet_settings.contains("height")){
            qDebug() << "No height parameter";
            return false;
        }
        if(!darknet_settings.contains("channels")){
            qDebug() << "No channels parameter";
            return false;
        }

        auto width = darknet_settings.value("width").toInt();
        auto height = darknet_settings.value("height").toInt();
        auto channels = darknet_settings.value("channels").toInt();

        darknet_settings.endGroup();

        qDebug() << width << height << channels;

        if(width > 0 && height > 0 && channels > 0){

            qDebug() << width << height << channels;

            image_width = width;
            image_height = height;
            image_channels = channels;

        }else{
            return false;
        }

    }else if(framework == FRAMEWORK_TENSORFLOW){
        // In theory we can parse the .pbtxt file to figure out
        // the input layer parameters, but that either means bringing in
        // protobuf or loading the entire network via OpenCV.

    }

    updateFields();

    return true;

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

    QString filter, title;

    if(framework == FRAMEWORK_DARKNET){
        filter = tr("Config (*.cfg)");
        title = tr("Select darknet config file");
    }else if(framework == FRAMEWORK_TENSORFLOW){
        filter = tr("Config (*.pbtxt)");
        title = tr("Select tensorflow config file");
    }else{
        return;
    }

    auto path = openFile(title, cfg_file, filter);

    if(path != ""){
        ui->cfgPathLineEdit->setText(path);
    }

    checkForm();
}


void DetectorSetupDialog::setWeightsFile(void){

    QString filter, title;

    if(framework == FRAMEWORK_DARKNET){
        filter = tr("Weights (*.weights)");
        title = tr("Select darknet weights file");
    }else if(framework == FRAMEWORK_TENSORFLOW){
        filter = tr("Config (*.pb)");
        title = tr("Select tensorflow frozen graph");
    }else{
        return;
    }

    auto path = openFile(title, weight_file, filter);

    if(path != ""){
        ui->weightPathLineEdit->setText(path);
    }

    checkForm();
}

void DetectorSetupDialog::setFramework(void){
    if(ui->frameworkComboBox->currentText().startsWith("Darknet")){
        framework = FRAMEWORK_DARKNET;
        settings->setValue("model_framework", framework);
    }else if(ui->frameworkComboBox->currentText().startsWith("Tensorflow")){
        framework = FRAMEWORK_TENSORFLOW;
        settings->setValue("model_framework", framework);
    }
}

void DetectorSetupDialog::setTarget(void){
    if(ui->targetComboBox->currentText() == "CPU"){
        target = cv::dnn::DNN_TARGET_CPU;
        settings->setValue("model_target", target);
    }else if(ui->targetComboBox->currentText() == "OpenCL"){
        target = cv::dnn::DNN_TARGET_OPENCL;
        settings->setValue("model_target", target);
    }else if(ui->targetComboBox->currentText() == "OpenCL FP16"){
        target = cv::dnn::DNN_TARGET_OPENCL_FP16;
        settings->setValue("model_target", target);
    }else if(ui->targetComboBox->currentText() == "CUDA"){
        target = cv::dnn::DNN_TARGET_CUDA;
        settings->setValue("model_target", target);
    }else if(ui->targetComboBox->currentText() == "CUDA FP16"){
        target = cv::dnn::DNN_TARGET_CUDA_FP16;
        settings->setValue("model_target", target);
    }

}

int DetectorSetupDialog::getTarget(void){
    return target;
}

void DetectorSetupDialog::setNamesFile(void){

    QString filter = tr("Names (*.names)");
    QString title = tr("Select darknet names file");

    auto path = openFile(title, names_file, filter);

    if(path != ""){
        ui->namesPathLineEdit->setText(path);
    }

    checkForm();
}

DetectorSetupDialog::~DetectorSetupDialog()
{
    delete ui;
}
