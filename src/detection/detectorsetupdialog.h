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
    model_framework getFramework(void){return framework;}
    int getTarget();
    ~DetectorSetupDialog();

private slots:

    void setCfgFile();
    void setNamesFile();
    void setWeightsFile();
    void setFramework();
    void setTarget();

    void checkForm();

private:
    Ui::DetectorSetupDialog *ui;
    bool getParamsFromConfig();
    void updateFields();

    QSettings* settings;

    QString cfg_file;
    QString weight_file;
    QString names_file;
    int image_width;
    int image_height;
    int image_channels;
    int target;
    model_framework framework = FRAMEWORK_TENSORFLOW;
    QString openFile(QString title, QString search_path="", QString filter="");
};

#endif // DETECTORSETUPDIALOG_H
