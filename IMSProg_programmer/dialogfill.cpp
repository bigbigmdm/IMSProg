#include "dialogfill.h"
#include "ui_dialogfill.h"

DialogFill::DialogFill(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogFill)
{
    ui->setupUi(this);
}

DialogFill::~DialogFill()
{
    delete ui;
}
