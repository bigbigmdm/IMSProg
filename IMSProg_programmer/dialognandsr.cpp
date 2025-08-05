/*
 * Copyright (C) 2023 - 2025 Mikhail Medvedev <e-ink-reader@yandex.ru>
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include "dialognandsr.h"
#include "ui_dialognandsr.h"
#include <QLineEdit>
#include <QWidget>
#include <QApplication>
#include <QValidator>
#include <QRegExp>
#include "unistd.h"
#include <QDebug>
#include <QRegularExpression>

DialogNANDSr::DialogNANDSr(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNANDSr)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    setLineEditFilter();
    regReaded = false;
}

DialogNANDSr::~DialogNANDSr()
{
    delete ui;
}

void DialogNANDSr::setLineEditFilter()
{
    QRegExp reHex( "[0-1]{1}" );
    QRegExpValidator *validator = new QRegExpValidator(reHex, this);
    //searching all lineEdit_srXX, XX - numbers
    QString searchText = "lineEdit_sr\\d+";
        QRegularExpression regex(searchText);

            for (QLineEdit* edit : findChildren<QLineEdit*>())
            {
                if (regex.match(edit->objectName()).hasMatch())
                {
                    edit->setValidator(validator);
                }
            }
}

void DialogNANDSr::on_pushButton_read_clicked()
{
    //READING STATUS REGISTERS
        uint8_t *buf;
        QString currRegName;
        int retval;
        uint8_t currRegister, currBit, currByte;
        int stCH341 = 0;
        buf = (uint8_t *)malloc(2);
        stCH341 = ch341a_spi_init();
        if (stCH341 == 0)
            {

            for (currRegister = 0; currRegister < 6; currRegister++)
            {
                if (RegNumbers[currRegister] != 0xff)
                {
                    SPI_CONTROLLER_Chip_Select_Low();
                    SPI_CONTROLLER_Write_One_Byte(0x0f);
                    SPI_CONTROLLER_Write_One_Byte(RegNumbers[currRegister]);
                    retval = SPI_CONTROLLER_Read_NByte(buf,1,SPI_CONTROLLER_SPEED_SINGLE);
                    SPI_CONTROLLER_Chip_Select_High();
                    usleep(1);
                    if (retval)
                       {
                          QMessageBox::about(this, tr("Error"), tr("Error reading register!"));
                          return;
                       }
                    //showing data
                    currByte = 1;
                    for (currBit = 0; currBit < 8; currBit++)
                    {
                        currRegName = "lineEdit_sr" + QString::number(currRegister) + QString::number(currBit);
                        for (QLineEdit* edit : findChildren<QLineEdit*>())
                                {
                                    if (edit->objectName() == currRegName)
                                    {
                                        edit->setText(QString::number((buf[0] & currByte) >> currBit));
                                    }
                                }
                        currByte = currByte << 1;
                    }
                }
            }
                ch341a_spi_shutdown();
                regReaded = true;
           }
        else QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
}

void DialogNANDSr::on_pushButton_write_clicked()
{
    uint8_t regData;
    int stCH341 = 0;
    uint8_t currRegister, currBit, currByte;
    QString currRegName, currValue;
    stCH341 = ch341a_spi_init();
    if (stCH341 == 0)
    {
        for (currRegister = 0; currRegister < 6; currRegister++)
        {
            //parsing data from lineEdits
            currByte = 1;
            regData = 0;
            if (RegNumbers[currRegister] != 0xff)
            {
                for (currBit = 0; currBit < 8; currBit++)
                {
                    currRegName = "lineEdit_sr" + QString::number(currRegister) + QString::number(currBit);
                    for (QLineEdit* edit : findChildren<QLineEdit*>())
                            {
                                if (edit->objectName() == currRegName)
                                {
                                    //edit->setText(QString::number((buf[0] & currByte) >> currBit));
                                    currValue = edit->text();
                                    if (QString::compare(currValue, "0", Qt::CaseInsensitive)) regData = regData + currByte;
                                }
                            }
                    currByte = currByte << 1;
                }
            }

            //Writing status registers

            SPI_CONTROLLER_Chip_Select_Low();
            SPI_CONTROLLER_Write_One_Byte(0x06);
            SPI_CONTROLLER_Chip_Select_High();
            usleep(1);

            SPI_CONTROLLER_Chip_Select_Low();
            SPI_CONTROLLER_Write_One_Byte(0x1f);
            SPI_CONTROLLER_Write_One_Byte(RegNumbers[currRegister]);
            SPI_CONTROLLER_Write_One_Byte(regData);
            SPI_CONTROLLER_Chip_Select_High();
            usleep(1);

            SPI_CONTROLLER_Chip_Select_Low();
            SPI_CONTROLLER_Write_One_Byte(0x04);
            SPI_CONTROLLER_Chip_Select_High();
            usleep(1);
        }

       //Close the CH341a device
       ch341a_spi_shutdown();
    }
    else QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
   //}
   //else QMessageBox::about(this, tr("Error"), tr("Before writing the registers, please press the `Read` button!"));
}

void DialogNANDSr::closeEvent(QCloseEvent* event)
{
    emit closeRequestHasArrived();
    QWidget::closeEvent(event);
}

void DialogNANDSr::setPattern(const uint pattern)
{
    allRegEnabled();
    switch (pattern)
       {
        case 0: //Winbond
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xff;
           RegNumbers[4] = 0xff;
           setRegDisabled(3);
           setRegDisabled(4);

           ui->label_10->setText("SRP0");
           ui->label_11->setText("BP3");
           ui->label_12->setText("BP2");
           ui->label_13->setText("BP1");
           ui->label_14->setText("BP0");
           ui->label_15->setText("TB");
           ui->label_16->setText("WP-E");
           ui->label_17->setText("SRP1");

           ui->label_20->setText("OTP-L");
           ui->label_21->setText("OTP-E");
           ui->label_22->setText("SR1-L");
           ui->label_23->setText("ECC-E");
           ui->label_24->setText("BUF");
           ui->label_25->setText("X");
           ui->label_26->setText("X");
           ui->label_27->setText("X");

           ui->label_30->setText("X");
           ui->label_31->setText("LUT-F");
           ui->label_32->setText("ECC-1");
           ui->label_33->setText("ECC-0");
           ui->label_34->setText("P-FAIL");
           ui->label_35->setText("E-FAIL");
           ui->label_36->setText("WEL");
           ui->label_37->setText("BUSY");

           ui->label_40->setText("X");
           ui->label_41->setText("X");
           ui->label_42->setText("X");
           ui->label_43->setText("X");
           ui->label_44->setText("X");
           ui->label_45->setText("X");
           ui->label_46->setText("X");
           ui->label_47->setText("X");

           ui->label_50->setText("X");
           ui->label_51->setText("X");
           ui->label_52->setText("X");
           ui->label_53->setText("X");
           ui->label_54->setText("X");
           ui->label_55->setText("X");
           ui->label_56->setText("X");
           ui->label_57->setText("X");
         break;

         case 1: //Gigadevice 1
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xd0;
           RegNumbers[4] = 0xf0;

           ui->label_10->setText("BWRD");
           ui->label_11->setText("X");
           ui->label_12->setText("BP2");
           ui->label_13->setText("BP1");
           ui->label_14->setText("BP0");
           ui->label_15->setText("INV");
           ui->label_16->setText("CMP");
           ui->label_17->setText("X");

           ui->label_20->setText("OTP-P");
           ui->label_21->setText("OTP-E");
           ui->label_22->setText("X");
           ui->label_23->setText("ECC-E");
           ui->label_24->setText("X");
           ui->label_25->setText("X");
           ui->label_26->setText("X");
           ui->label_27->setText("QE");

           ui->label_30->setText("X");
           ui->label_31->setText("X");
           ui->label_32->setText("ECC-1");
           ui->label_33->setText("ECC-0");
           ui->label_34->setText("P-FAIL");
           ui->label_35->setText("E-FAIL");
           ui->label_36->setText("WEL");
           ui->label_37->setText("BUSY");

           ui->label_40->setText("X");
           ui->label_41->setText("DS_S1");
           ui->label_42->setText("DS-S0");
           ui->label_43->setText("X");
           ui->label_44->setText("X");
           ui->label_45->setText("X");
           ui->label_46->setText("X");
           ui->label_47->setText("X");

           ui->label_50->setText("X");
           ui->label_51->setText("X");
           ui->label_52->setText("ECCS1");
           ui->label_53->setText("ECCS0");
           ui->label_54->setText("X");
           ui->label_55->setText("X");
           ui->label_56->setText("X");
           ui->label_57->setText("X");
         break;
         case 2: //Gigadevice 2
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xff;
           RegNumbers[4] = 0xff;
           setRegDisabled(3);
           setRegDisabled(4);

           ui->label_10->setText("BWRD");
           ui->label_11->setText("X");
           ui->label_12->setText("BP2");
           ui->label_13->setText("BP1");
           ui->label_14->setText("BP0");
           ui->label_15->setText("INV");
           ui->label_16->setText("CMP");
           ui->label_17->setText("X");

           ui->label_20->setText("OTP-P");
           ui->label_21->setText("OTP-E");
           ui->label_22->setText("X");
           ui->label_23->setText("ECC-E");
           ui->label_24->setText("X");
           ui->label_25->setText("X");
           ui->label_26->setText("X");
           ui->label_27->setText("QE");

           ui->label_30->setText("X");
           ui->label_31->setText("X");
           ui->label_32->setText("ECC-1");
           ui->label_33->setText("ECC-0");
           ui->label_34->setText("P-FAIL");
           ui->label_35->setText("E-FAIL");
           ui->label_36->setText("WEL");
           ui->label_37->setText("BUSY");

           ui->label_40->setText("X");
           ui->label_41->setText("X");
           ui->label_42->setText("X");
           ui->label_43->setText("X");
           ui->label_44->setText("X");
           ui->label_45->setText("X");
           ui->label_46->setText("X");
           ui->label_47->setText("X");

           ui->label_50->setText("X");
           ui->label_51->setText("X");
           ui->label_52->setText("X");
           ui->label_53->setText("X");
           ui->label_54->setText("X");
           ui->label_55->setText("X");
           ui->label_56->setText("X");
           ui->label_57->setText("X");
         break;
       }
}

void DialogNANDSr::allRegEnabled()
{
    auto lineEdits = findChildren<QLineEdit*>();
    for (auto lineEdit : lineEdits)
    {
        lineEdit->setDisabled(false);
    }
}

void DialogNANDSr:: setRegDisabled(uint8_t regNumber)
{
    QString searchText = "lineEdit_sr" + QString::number(regNumber) + "\\d+";
    QRegularExpression regex(searchText);

        for (QLineEdit* edit : findChildren<QLineEdit*>())
        {
            if (regex.match(edit->objectName()).hasMatch())
            {
                edit->setDisabled(true);
            }
        }
}
