#ifndef DETECTORSETUPDIALOG_H
#define DETECTORSETUPDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QSettings>
#include <QPushButton>
#include <QtDebug>
#include <QDir>

#include <opencv2/dnn.hpp>

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
    QString getNames(void){return darknet_names_file;}
    QString getWeights(void){return darknet_weight_file;}
    QString getCfg(void){return darknet_cfg_file;}
    int getTarget();
    ~DetectorSetupDialog();

private slots:

    void setCfgFile();
    void setNamesFile();
    void setWeightsFile();
    void checkForm();

private:
    Ui::DetectorSetupDialog *ui;
    void getParamsFromConfig();
    void updateFields();

    QSettings* settings;

    QString darknet_cfg_file;
    QString darknet_weight_file;
    QString darknet_names_file;
    int image_width;
    int image_height;
    int image_channels;
    QString openFile(QString title, QString search_path="", QString filter="");
};

#endif // DETECTORSETUPDIALOG_H
