#include "dialoginfo.h"
#include "ui_dialoginfo.h"

DialogInfo::DialogInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogInfo)
{
    ui->setupUi(this);
    pix24 = new QPixmap(":/res/img/ch341_24.png");
    pix93 = new QPixmap(":/res/img/ch341_93.png");
    pix25 = new QPixmap(":/res/img/ch341_spi.png");
    pix2518 = new QPixmap(":/res/img/ch341_spi_18.png");
    pixnone = new QPixmap(":/res/img/ch341_unknown.png");
}

DialogInfo::~DialogInfo()
{
    delete pix24;
    delete pix93;
    delete pix25;
    delete pix2518;
    delete pixnone;
    delete ui;
}
void DialogInfo::on_pushButton_clicked()
{
   DialogInfo::close();
}
void DialogInfo::setChip(const uint chipType)
{
   switch (chipType)
   {
     case 1:
        ui->label->setPixmap(*pix24);
        ui->label_slot->setText("24xx");
        ui->label_adapter->setText("-");
     break;
     case 2:
       ui->label->setPixmap(*pix25);
       ui->label_slot->setText("25xx");
       ui->label_adapter->setText("-");
     break;
     case 3:
       ui->label->setPixmap(*pix2518);
       ui->label_slot->setText("25xx");
       ui->label_adapter->setText("1.8V-Adapter");
     break;
     case 4:
       ui->label_slot->setText("25xx");
       ui->label_adapter->setText("93xx adapter");
       ui->label->setPixmap(*pix93);
     break;
     default:
       ui->label_slot->setText("-");
       ui->label_adapter->setText("-");
       ui->label->setPixmap(*pixnone);
     break;

   }

}
