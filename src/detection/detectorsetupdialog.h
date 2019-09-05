#ifndef DETECTORSETUPDIALOG_H
#define DETECTORSETUPDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QSettings>
#include <QPushButton>
#include <QtDebug>
#include <QDir>

#include <opencv2/dnn.hpp>
#include <detection/detectoropencv.h>

namespace Ui {
class DetectorSetupDialog;
}

class DetectorSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DetectorSetupDialog(QWidget *parent = nullptr);
    int getWidth(void){return image_width;}
    int getHeight(void){return image_height;}
    int getChannels(void){return image_channels;}
    QString getNames(void){return names_file;}
    QString getWeights(void){return weight_file;}
    QString getCfg(void){return cfg_file;}
    bool getConvertGrayscale(void);
    bool getConvertDepth(void);
    model_framework getFramework(void){return framework;}
    int getTarget();
    ~DetectorSetupDialog();

private slots:

    void setCfgFile();
    void setNamesFile();
    void setWeightsFile();
    void setFramework();
    void setTarget();
    void setConvertGrayscale(void);
    void setConvertDepth(void);

    void checkForm();


private:
    Ui::DetectorSetupDialog *ui;
    bool getParamsFromConfig();
    void updateFields();


    QSettings* settings;

    QString cfg_file;
    QString weight_file;
    QString names_file;
    int image_width = 320;
    int image_height = 240;
    int image_channels = 3; // default
    int target = 0;
    bool convert_grayscale = true;
    bool convert_depth = true;
    model_framework framework = FRAMEWORK_TENSORFLOW;
    QString openFile(QString title, QString search_path="", QString filter="");
};

#endif // DETECTORSETUPDIALOG_H
