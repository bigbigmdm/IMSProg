/*
 * Copyright (C) 2023 - 2024 Mikhail Medvedev <e-ink-reader@yandex.ru>
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include "dialoginfo.h"
#include "ui_dialoginfo.h"

DialogInfo::DialogInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogInfo)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    pix24 = new QPixmap(":/res/img/ch341_24.png");
    pix93 = new QPixmap(":/res/img/ch341_93.png");
    pix25 = new QPixmap(":/res/img/ch341_spi.png");
    pix45 = new QPixmap(":/res/img/ch341_45.png");
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
     case 5:
       ui->label_slot->setText("25xx");
       ui->label_adapter->setText("45xx adapter");
       ui->label->setPixmap(*pix45);
     break;
     default:
       ui->label_slot->setText("-");
       ui->label_adapter->setText("-");
       ui->label->setPixmap(*pixnone);
     break;

   }

}
