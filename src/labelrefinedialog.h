#ifndef LABELREFINEDIALOG_H
#define LABELREFINEDIALOG_H

#include <QDialog>

namespace Ui {
class LabelRefineDialog;
}

class LabelRefineDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LabelRefineDialog(QWidget *parent = nullptr);
    ~LabelRefineDialog();

private:
    Ui::LabelRefineDialog *ui;
};

#endif // LABELREFINEDIALOG_H
