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
    pix35 = new QPixmap(":/res/img/ch341_35.png");
    pix45 = new QPixmap(":/res/img/ch341_45.png");
    pix2518 = new QPixmap(":/res/img/ch341_spi_18.png");
    pix3518 = new QPixmap(":/res/img/ch341_spi_18_wson.png");

    pix24v7 = new QPixmap(":/res/img/v1_7_i2c_3v3.png");
    pix25v718 = new QPixmap(":/res/img/v1_7_spi_1v8.png");
    pix25v733 = new QPixmap(":/res/img/v1_7_spi_3v3.png");
    pix93v17 = new QPixmap(":/res/img/v1_7_mw_3v3.png");
    pix45v17 = new QPixmap(":/res/img/v1_7_45_3v3.png");
    pix35v718 = new QPixmap(":/res/img/v1_7_NAND_1v8.png");
    pix35v733 = new QPixmap(":/res/img/v1_7_NAND_3v3.png");

    pix24x347_3v3_0 = new QPixmap(":/res/img/ch347_i2c_3v3.png");
    pix25x347_1v8_0 = new QPixmap(":/res/img/ch347_spi_1v8.png");
    pix25x347_3v3_0 = new QPixmap(":/res/img/ch347_spi_3v3.png");
    pix93x347_3v3_0 = new QPixmap(":/res/img/ch347_mv_3v3.png");
    pix45x347_3v3_0 = new QPixmap(":/res/img/ch347_45_3v3.png");
    pix35x347_1v8_0 = new QPixmap(":/res/img/ch347_spi_18_wson.png");
    pix35x347_3v3_0 = new QPixmap(":/res/img/ch347_NAND_3v3.png");

    pix24x347_3v3_1 = new QPixmap(":/res/img/v1_1_i2c_3v3.png");
    pix25x347_1v8_1 = new QPixmap(":/res/img/v1_1_spi_1v8.png");
    pix25x347_3v3_1 = new QPixmap(":/res/img/v1_1_spi_3v3.png");
    pix93x347_3v3_1 = new QPixmap(":/res/img/v1_1_mw_3v3.png");
    pix45x347_3v3_1 = new QPixmap(":/res/img/v1_1_45_3v3.png");
    pix35x347_1v8_1 = new QPixmap(":/res/img/v1_1_NAND_1v8.png");
    pix35x347_3v3_1 = new QPixmap(":/res/img/v1_1_NAND_3v3.png");


    pixnone = new QPixmap(":/res/img/ch341_unknown.png");
}

DialogInfo::~DialogInfo()
{
    delete pix24;
    delete pix93;
    delete pix25;
    delete pix35;
    delete pix2518;
    delete pix3518;

    delete pix24v7;
    delete pix25v718;
    delete pix25v733;
    delete pix93v17;
    delete pix45v17;
    delete pix35v718;
    delete pix35v733;

    delete pix24x347_3v3_0;
    delete pix25x347_1v8_0;
    delete pix25x347_3v3_0;
    delete pix93x347_3v3_0;
    delete pix45x347_3v3_0;
    delete pix35x347_1v8_0;
    delete pix35x347_3v3_0;

    delete pix24x347_3v3_1;
    delete pix25x347_1v8_1;
    delete pix25x347_3v3_1;
    delete pix93x347_3v3_1;
    delete pix45x347_3v3_1;
    delete pix35x347_1v8_1;
    delete pix35x347_3v3_1;

    delete pixnone;
    delete ui;
}
void DialogInfo::on_pushButton_clicked()
{
   DialogInfo::close();
}

void DialogInfo::setProgrammer(const uint8_t progType)
{
  currentProg = progType;
}

void DialogInfo::setChip(const uint chipType)
{
   switch (chipType)
   {
     case 1:
        if (currentProg == 0) ui->label->setPixmap(*pix24);
        if (currentProg == 1) ui->label->setPixmap(*pix24v7);
        if (currentProg == 2) ui->label->setPixmap(*pix24x347_3v3_0);
        if (currentProg == 3) ui->label->setPixmap(*pix24x347_3v3_1);
        ui->label_slot->setText("24xx");
        ui->label_adapter->setText("-");
     break;
     case 2:
       if (currentProg == 0) ui->label->setPixmap(*pix25);
       if (currentProg == 1) ui->label->setPixmap(*pix25v733);
       if (currentProg == 2) ui->label->setPixmap(*pix25x347_3v3_0);
       if (currentProg == 3) ui->label->setPixmap(*pix25x347_3v3_1);
       ui->label_slot->setText("25xx");
       ui->label_adapter->setText("-");
     break;
     case 3:
       ui->label->setPixmap(*pix2518);
       if (currentProg == 0) ui->label->setPixmap(*pix2518);
       if (currentProg == 1) ui->label->setPixmap(*pix25v718);
       if (currentProg == 2) ui->label->setPixmap(*pix25x347_1v8_0);
       if (currentProg == 3) ui->label->setPixmap(*pix25x347_1v8_1);
       ui->label_slot->setText("25xx");
       if (currentProg == 0) ui->label_adapter->setText("1.8V-Adapter");
       if (currentProg == 1) ui->label_adapter->setText("-");
       if (currentProg == 2) ui->label_adapter->setText("1.8V-Adapter");
       if (currentProg == 3) ui->label_adapter->setText("-");
     break;
     case 4:
       ui->label_slot->setText("25xx");
       ui->label_adapter->setText("93xx adapter");
       if (currentProg == 0) ui->label->setPixmap(*pix93);
       if (currentProg == 1) ui->label->setPixmap(*pix93v17);
       if (currentProg == 2) ui->label->setPixmap(*pix93x347_3v3_0);
       if (currentProg == 3) ui->label->setPixmap(*pix93x347_3v3_1);
     break;
     case 5:
       ui->label_slot->setText("25xx");
       ui->label_adapter->setText("45xx adapter");
       if (currentProg == 0) ui->label->setPixmap(*pix45);
       if (currentProg == 1) ui->label->setPixmap(*pix45v17);
       if (currentProg == 2) ui->label->setPixmap(*pix45x347_3v3_0);
       if (currentProg == 3) ui->label->setPixmap(*pix45x347_3v3_1);
     break;
     case 6:
       ui->label_slot->setText("25xx");
       ui->label_adapter->setText("WSON adapter");
       if (currentProg == 0) ui->label->setPixmap(*pix35);
       if (currentProg == 1) ui->label->setPixmap(*pix35v733);
       if (currentProg == 2) ui->label->setPixmap(*pix35x347_3v3_0);
       if (currentProg == 3) ui->label->setPixmap(*pix35x347_3v3_1);
     break;
     case 7:
       if (currentProg == 0) ui->label->setPixmap(*pix3518);
       if (currentProg == 1) ui->label->setPixmap(*pix35v718);
       if (currentProg == 2) ui->label->setPixmap(*pix35x347_1v8_0);
       if (currentProg == 3) ui->label->setPixmap(*pix35x347_1v8_1);
       ui->label_slot->setText("25xx");
       if (currentProg == 0) ui->label_adapter->setText("1.8V + WSON");
       if (currentProg == 1) ui->label_adapter->setText("WSON");
       if (currentProg == 2) ui->label_adapter->setText("1.8V + WSON");
       if (currentProg == 3) ui->label_adapter->setText("WSON");
     break;
     default:
       ui->label_slot->setText("-");
       ui->label_adapter->setText("-");
       ui->label->setPixmap(*pixnone);
     break;

   }

}
