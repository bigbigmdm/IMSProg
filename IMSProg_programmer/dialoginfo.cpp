#include "dialoginfo.h"
#include "ui_dialoginfo.h"

DialogInfo::DialogInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogInfo)
{
    ui->setupUi(this);
}

DialogInfo::~DialogInfo()
{
    delete ui;
}
void DialogInfo::on_pushButton_clicked()
{
   DialogInfo::close();
}
void DialogInfo::setChip(const uint chipType)
{
   QPixmap *pix24 = new QPixmap(":/res/img/ch341_24.png");
   QPixmap *pix93 = new QPixmap(":/res/img/ch341_93.png");
   QPixmap *pix25 = new QPixmap(":/res/img/ch341_spi.png");
   QPixmap *pix2518 = new QPixmap(":/res/img/ch341_spi_18.png");
   QPixmap *pixnone = new QPixmap(":/res/img/ch341_unknown.png");

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
