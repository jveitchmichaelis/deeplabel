#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QSettings>
#include <QtDebug>

namespace Ui {
class ImportDialog;
}

class ImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportDialog(QWidget *parent = nullptr);
    QString getImporter(){return current_importer; }
    QString getInputFile(){return input_file; }
    QString getNamesFile(){return names_file; }
    QString getAnnotationFile(){return annotation_file; }
    bool getImportUnlabelled();
    ~ImportDialog();

 private slots:
    void setNamesFile(QString path="");
    void setInputFile(QString path="");
    void setAnnotationFile(QString path="");
    void toggleImporter();
    void setImportUnlabelled(bool res);

private:
    bool checkOK();
    QSettings *settings;

    Ui::ImportDialog *ui;
    QString input_file = "";
    QString names_file = "";
    QString annotation_file = "";
    QString current_importer = "Darknet";
    bool import_unlabelled;
    bool checkNamesFile(QString names_file);
};

#endif // IMPORTDIALOG_H
