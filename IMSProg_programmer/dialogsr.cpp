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
#include "dialogsr.h"
#include "ui_dialogsr.h"
#include <QValidator>
#include <QRegExp>
#include "unistd.h"
#include <QDebug>

DialogSR::DialogSR(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSR)
{
    ui->setupUi(this);
    setLineEditFilter();
}

DialogSR::~DialogSR()
{
    delete ui;
}

void DialogSR::on_pushButton_read_clicked()
{
    //READING STATUS REGISTER
    uint8_t *buf;
    int retval;
    int stCH341 = 0;
    buf = (uint8_t *)malloc(2);
    stCH341 = ch341a_spi_init();
    if (stCH341 == 0)
        {
           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x05);
           retval = SPI_CONTROLLER_Read_NByte(buf,1,SPI_CONTROLLER_SPEED_SINGLE);
           qDebug() << "retval=" << retval;
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);
           if (retval)
              {
                 QMessageBox::about(this, tr("Error"), tr("Error reading register!"));
                 return;
              }
            ui->lineEdit_sr07->setText(QString::number(((buf[0] & 128) >> 7)));
            ui->lineEdit_sr06->setText(QString::number(((buf[0] & 64) >> 6)));
            ui->lineEdit_sr05->setText(QString::number(((buf[0] & 32) >> 5)));
            ui->lineEdit_sr04->setText(QString::number(((buf[0] & 16) >> 4)));
            ui->lineEdit_sr03->setText(QString::number(((buf[0] & 8) >> 3)));
            ui->lineEdit_sr02->setText(QString::number(((buf[0] & 4) >> 2)));
            ui->lineEdit_sr01->setText(QString::number(((buf[0] & 2) >> 1)));
            ui->lineEdit_sr00->setText(QString::number((buf[0] & 1)));
            ch341a_spi_shutdown();
       }
    else QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
}

void DialogSR::on_pushButton_write_clicked()
{
    //READING STATUS REGISTER
    uint8_t r0 = 0;
    int stCH341 = 0;
    stCH341 = ch341a_spi_init();
    if (stCH341 == 0)
        {
           if (QString::compare(ui->lineEdit_sr07->text(), "0", Qt::CaseInsensitive)) r0 = r0 + 128;
           if (QString::compare(ui->lineEdit_sr06->text(), "0", Qt::CaseInsensitive)) r0 = r0 +  64;
           if (QString::compare(ui->lineEdit_sr05->text(), "0", Qt::CaseInsensitive)) r0 = r0 +  32;
           if (QString::compare(ui->lineEdit_sr04->text(), "0", Qt::CaseInsensitive)) r0 = r0 +  16;
           if (QString::compare(ui->lineEdit_sr03->text(), "0", Qt::CaseInsensitive)) r0 = r0 +   8;
           if (QString::compare(ui->lineEdit_sr02->text(), "0", Qt::CaseInsensitive)) r0 = r0 +   4;
           if (QString::compare(ui->lineEdit_sr01->text(), "0", Qt::CaseInsensitive)) r0 = r0 +   2;
           if (QString::compare(ui->lineEdit_sr00->text(), "0", Qt::CaseInsensitive)) r0 = r0 +   1;
           //Writing status registers
           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x06);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);

           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x01);
           SPI_CONTROLLER_Write_One_Byte(r0);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);

           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x04);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);

           ch341a_spi_shutdown();
        }
    else QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
}

void DialogSR::setLineEditFilter()
{
    QRegExp reHex( "[0-1]{1}" );
    QRegExpValidator *validator = new QRegExpValidator(reHex, this);
    ui->lineEdit_sr00->setValidator(validator);
    ui->lineEdit_sr01->setValidator(validator);
    ui->lineEdit_sr02->setValidator(validator);
    ui->lineEdit_sr03->setValidator(validator);
    ui->lineEdit_sr04->setValidator(validator);
    ui->lineEdit_sr05->setValidator(validator);
    ui->lineEdit_sr06->setValidator(validator);
    ui->lineEdit_sr07->setValidator(validator);
}

void DialogSR::setChipType(const uint chipType)
{
   switch (chipType)
   {

     case 3:
       ui->label_20->setText("/RDY");
       ui->label_19->setText("WEN");
       ui->label_13->setText("WPEN");
     break;

     case 4:
       ui->label_20->setText("WIP");
       ui->label_19->setText("WEL");
       ui->label_13->setText("SRWD");
     break;

     default:
     break;
   }
}

void DialogSR::closeEvent(QCloseEvent* event)
{
    emit closeRequestHasArrived();
    QWidget::closeEvent(event);
}
