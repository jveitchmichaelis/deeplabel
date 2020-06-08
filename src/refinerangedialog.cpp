#include "refinerangedialog.h"
#include "ui_refinerangedialog.h"

RefineRangeDialog::RefineRangeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RefineRangeDialog)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);

    connect(ui->startSpinBox, SIGNAL(valueChanged(int)), this, SLOT(check()));
    connect(ui->endSpinBox, SIGNAL(valueChanged(int)), this, SLOT(check()));
}

void RefineRangeDialog::setMaxImage(int max_images){
    ui->startSpinBox->setRange(1, max_images+1);
    ui->endSpinBox->setRange(1, max_images+1);
}

int RefineRangeDialog::getStart(){
    return ui->startSpinBox->value();
}


int RefineRangeDialog::getEnd(){
    return ui->endSpinBox->value();
}

void RefineRangeDialog::setCurrentImage(int i){
    ui->startSpinBox->setValue(i);
    ui->endSpinBox->setValue(i);
}

bool RefineRangeDialog::check(){

    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    if(getStart() > getEnd()) return false;

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

RefineRangeDialog::~RefineRangeDialog()
{
    delete ui;
}
