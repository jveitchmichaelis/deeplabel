#include "labelrefinedialog.h"
#include "ui_labelrefinedialog.h"

LabelRefineDialog::LabelRefineDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LabelRefineDialog)
{
    ui->setupUi(this);
}

LabelRefineDialog::~LabelRefineDialog()
{
    delete ui;
}
