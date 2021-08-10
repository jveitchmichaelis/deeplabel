#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QSettings>
#include <QtDebug>

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = nullptr);
    ~ExportDialog();

    QString getExporter(){return current_exporter; }
    QString getOutputFolder(){return output_folder; }
    QString getNamesFile(){return names_file; }
    int getShuffle(){return do_shuffle; }
    double getValidationSplit(){
        return static_cast<double>(validation_split_pc)/100.0;
    }
    bool getAppendLabels(){return append_labels;}

    QString getBucket(){ return bucket_uri; }

    bool getCreateLabelMap();
    bool getExportUnlablled();
    bool getValidationSplitEnabled();
    QString getFilePrefix();

private slots:
    void setNamesFile(QString path="");
    void setOutputFolder(QString path="");
    void setValidationSplit(int value);
    void toggleShuffle(bool shuffle);
    void toggleAppendLabels(bool enable);
    void toggleValidationSplit(bool enable);
    void toggleExportUnlabelled(bool res);
    void toggleExporter();
    void setBucketUri(QString uri);
    void setFilePrefix(QString prefix);

private:
    Ui::ExportDialog *ui;
    bool checkOK();

    QSettings *settings;

    QString output_folder = "";
    QString names_file = "";
    QString bucket_uri = "";
    QString file_prefix = "";
    QString current_exporter = "Darknet";
    bool do_shuffle = false;
    bool append_labels = false;
    bool validation_split_enable = false;
    int validation_split_pc = 80;
    bool export_unlabelled;

};

#endif // EXPORTDIALOG_H
