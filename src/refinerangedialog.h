#ifndef REFINERANGEDIALOG_H
#define REFINERANGEDIALOG_H

#include <QDialog>
#include <QPushButton>

namespace Ui {
class RefineRangeDialog;
}

class RefineRangeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RefineRangeDialog(QWidget *parent = nullptr);
    ~RefineRangeDialog();

    int getStart();
    int getEnd();
public slots:
    void setMaxImage(int max_images);
    void setCurrentImage(int i);
private slots:
    bool check();
private:
    Ui::RefineRangeDialog *ui;

};

#endif // REFINERANGEDIALOG_H
