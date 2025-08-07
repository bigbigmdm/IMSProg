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
#include "hexutility.h"
#include "memory"
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
    QFontMetrics fm(ui->textEdit_buf->fontMetrics());
        int pixelsHigh = fm.height();
        int pixelsWidth = fm.horizontalAdvance("00> 4F 4E 46 49 00 00 00 00 02 00 00 00 00 00 00 00 ") + 20;
        int sectionHeight = pixelsHigh * 16 + 6;
        ui->textEdit_buf->setMinimumHeight(sectionHeight);
        ui->textEdit_buf->setMinimumWidth(pixelsWidth);
        ui->textEdit_ID->setMaximumHeight(pixelsHigh * 2 + 16);
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
        bool otp;
        int i,j, pageSize, numBlocks, pagesPerBlock;
        QString buftxt = "";
        otp = false;
        //Clearing fields
        clearAllFields();
        ui->textEdit_ID->clear();
        //READING STATUS REGISTERS
        std::shared_ptr<uint8_t[]> buf(new uint8_t[512]);
        QString currRegName;
        int retval;
        uint8_t currRegister, currBit, currByte;
        int stCH341 = 0;
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
                    retval = SPI_CONTROLLER_Read_NByte(buf.get(),1,SPI_CONTROLLER_SPEED_SINGLE);
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

                regReaded = true;
                //ch341a_spi_shutdown();
                //READING PARAMETER PAGE
                //stCH341 = ch341a_spi_init();


                usleep(100);
                SPI_CONTROLLER_Chip_Select_Low(); //Reading status
                SPI_CONTROLLER_Write_One_Byte(0x0f);
                SPI_CONTROLLER_Write_One_Byte(0xb0);
                retval = SPI_CONTROLLER_Read_NByte(buf.get(),1,SPI_CONTROLLER_SPEED_SINGLE);
                SPI_CONTROLLER_Chip_Select_High();
                usleep(1);
                if ((buf[0] & 0x40) == 0) //OPT Disabled ?
                {
                    otp = false;
                    // Enable OTP MODE

                    SPI_CONTROLLER_Chip_Select_Low();  //Write enable
                    SPI_CONTROLLER_Write_One_Byte(0x06);
                    SPI_CONTROLLER_Chip_Select_High();
                    usleep(1);

                    SPI_CONTROLLER_Chip_Select_Low();
                    SPI_CONTROLLER_Write_One_Byte(0x1f);
                    SPI_CONTROLLER_Write_One_Byte(0xb0);
                    SPI_CONTROLLER_Write_One_Byte(buf[0] | 0x40); //&bf to clear
                    SPI_CONTROLLER_Chip_Select_High();
                    usleep(1);
                }
                else otp = true;

                SPI_CONTROLLER_Chip_Select_Low();
                SPI_CONTROLLER_Write_One_Byte(0x13);
                SPI_CONTROLLER_Write_One_Byte(0x00);
                SPI_CONTROLLER_Write_One_Byte(0x00);
                SPI_CONTROLLER_Write_One_Byte(0x01);
                SPI_CONTROLLER_Chip_Select_High();
                usleep(1000);
                SPI_CONTROLLER_Chip_Select_Low();
                SPI_CONTROLLER_Write_One_Byte(0x03);
                SPI_CONTROLLER_Write_One_Byte(0x00);
                SPI_CONTROLLER_Write_One_Byte(0x00);
                SPI_CONTROLLER_Write_One_Byte(0x00);
                retval = SPI_CONTROLLER_Read_NByte(buf.get(),256,SPI_CONTROLLER_SPEED_SINGLE);
                SPI_CONTROLLER_Chip_Select_High();
                if (retval)
                   {
                      QMessageBox::about(this, tr("Error"), tr("Error reading Parameter Page!"));
                      return;
                   }
                usleep(100);
                if ((buf[0] == 0x4f) && (buf[1] == 0x4e) && (buf[2] == 0x46) && (buf[3] == 0x49) )
                {
                    //Parameter page supported
                    for (j = 0; j < 255; j = j + 16)
                    {
                        //address
                        buftxt = buftxt + bytePrint(static_cast<uint8_t>(j)) + "> ";
                        //dump
                        for (i = 0; i < 16; i++)
                        {
                            buftxt = buftxt + bytePrint(buf[j + i]) + " ";
                        }
                        buftxt = buftxt + "\n";
                    }
                    buftxt.chop(1);
                    ui->textEdit_buf->setText(buftxt);
                    // Parsing parameter page
                    //Manufacture
                    buftxt.clear();
                    for (i = 32; i < 44; i++) buftxt = buftxt + static_cast<char>(buf[i]);
                    ui->lineEdit_man->setText(buftxt);
                    //Model
                    buftxt.clear();
                    for (i = 44; i < 64; i++) buftxt = buftxt + static_cast<char>(buf[i]);
                    ui->lineEdit_model->setText(buftxt);
                    //Page size
                    buftxt.clear();
                    i = buf[80] + buf[81] * 256 + buf[82] * 256 * 256 + buf[83] * 256 * 256 * 256;
                    buftxt = QString::number(i);
                    pageSize = i;
                    ui->lineEdit_page->setText(buftxt);
                    //ECC size
                    buftxt.clear();
                    i = buf[84] + buf[85] * 256;
                    buftxt = QString::number(i);
                    ui->lineEdit_ECC->setText(buftxt);
                    //Number of pages per block
                    buftxt.clear();
                    i = buf[92] + buf[93] * 256 + buf[94] * 256 * 256 + buf[95] * 256 * 256 * 256;
                    buftxt = QString::number(i);
                    pagesPerBlock = i;
                    ui->lineEdit_pages->setText(buftxt);
                    //Number of blocks per logical unit
                    buftxt.clear();
                    i = buf[96] + buf[97] * 256 + buf[98] * 256 * 256 + buf[99] * 256 * 256 * 256;
                    buftxt = QString::number(i);
                    numBlocks = i;
                    ui->lineEdit_blocks->setText(buftxt);
                    //Block Size
                    i = pageSize * pagesPerBlock / 1024;
                    buftxt = QString::number(i) + " K";
                    ui->lineEdit_blocksize->setText(buftxt);
                    //Chip size
                    i = pageSize * pagesPerBlock * numBlocks / 1024 / 1024;
                    buftxt = QString::number(i) + " M";
                    ui->lineEdit_ChipSize->setText(buftxt);
                }
                else ui->textEdit_buf->setText(tr("The Parameter Page is not supported."));
                //Get unique ID
                SPI_CONTROLLER_Chip_Select_Low();
                SPI_CONTROLLER_Write_One_Byte(0x13);
                SPI_CONTROLLER_Write_One_Byte(0x00);
                SPI_CONTROLLER_Write_One_Byte(0x00);
                SPI_CONTROLLER_Write_One_Byte(0x00);
                SPI_CONTROLLER_Chip_Select_High();
                usleep(1000);
                SPI_CONTROLLER_Chip_Select_Low();
                SPI_CONTROLLER_Write_One_Byte(0x03);
                SPI_CONTROLLER_Write_One_Byte(0x00);
                SPI_CONTROLLER_Write_One_Byte(0x00);
                SPI_CONTROLLER_Write_One_Byte(0x00);
                retval = SPI_CONTROLLER_Read_NByte(buf.get(),256,SPI_CONTROLLER_SPEED_SINGLE);
                SPI_CONTROLLER_Chip_Select_High();
                if (retval)
                   {
                      QMessageBox::about(this, tr("Error"), tr("Error reading ID!"));
                      return;
                   }
                 buftxt.clear();
                 for (i = 0; i < 32; i++)
                 {
                     buftxt = buftxt + bytePrint(buf[i]) + " ";
                     if (i == 15)
                     {
                         buftxt.chop(1);
                         buftxt = buftxt + "\n";
                     }
                 }
                 ui->textEdit_ID->setText(buftxt);

                if (otp == false) //OPT Disabled
                {
                    // Disable OTP MODE
                    SPI_CONTROLLER_Chip_Select_Low(); //Reading status
                    SPI_CONTROLLER_Write_One_Byte(0x0f);
                    SPI_CONTROLLER_Write_One_Byte(0xb0);
                    retval = SPI_CONTROLLER_Read_NByte(buf.get(),1,SPI_CONTROLLER_SPEED_SINGLE);
                    SPI_CONTROLLER_Chip_Select_High();
                    usleep(1);

                    SPI_CONTROLLER_Chip_Select_Low();  //Write enable
                    SPI_CONTROLLER_Write_One_Byte(0x06);
                    SPI_CONTROLLER_Chip_Select_High();
                    usleep(1);

                    SPI_CONTROLLER_Chip_Select_Low();
                    SPI_CONTROLLER_Write_One_Byte(0x1f);
                    SPI_CONTROLLER_Write_One_Byte(0xb0);
                    SPI_CONTROLLER_Write_One_Byte(buf[0] & 0xbf);
                    SPI_CONTROLLER_Chip_Select_High();
                    usleep(1);
                }
                SPI_CONTROLLER_Chip_Select_Low();  //Write disable
                SPI_CONTROLLER_Write_One_Byte(0x04);
                SPI_CONTROLLER_Chip_Select_High();
                usleep(1);




                ch341a_spi_shutdown();
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
         case 3: //MXIC
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xe0;
           RegNumbers[4] = 0x10;

           ui->label_10->setText("BPWRD");
           ui->label_11->setText("X");
           ui->label_12->setText("BP2");
           ui->label_13->setText("BP1");
           ui->label_14->setText("BP0");
           ui->label_15->setText("INV");
           ui->label_16->setText("CMP");
           ui->label_17->setText("SP");

           ui->label_20->setText("OTP-P");
           ui->label_21->setText("OTP-E");
           ui->label_22->setText("X");
           ui->label_23->setText("ECC-E");
           ui->label_24->setText("X");
           ui->label_25->setText("CONT");
           ui->label_26->setText("X");
           ui->label_27->setText("QE");

           ui->label_30->setText("CRBSY");
           ui->label_31->setText("BBMT_F");
           ui->label_32->setText("ECC-1");
           ui->label_33->setText("ECC-0");
           ui->label_34->setText("P-FAIL");
           ui->label_35->setText("E-FAIL");
           ui->label_36->setText("WEL");
           ui->label_37->setText("BUSY");

           ui->label_40->setText("DS_IO");
           ui->label_41->setText("DS_IO");
           ui->label_42->setText("X");
           ui->label_43->setText("X");
           ui->label_44->setText("X");
           ui->label_45->setText("X");
           ui->label_46->setText("X");
           ui->label_47->setText("X");

           ui->label_50->setText("BFT3");
           ui->label_51->setText("BFT2");
           ui->label_52->setText("BFT1");
           ui->label_53->setText("BFT0");
           ui->label_54->setText("X");
           ui->label_55->setText("X");
           ui->label_56->setText("X");
           ui->label_57->setText("ENPGM");
         break;
         case 4: //ESMT
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xff;
           RegNumbers[4] = 0xff;
           setRegDisabled(3);
           setRegDisabled(4);

           ui->label_10->setText("BRWD");
           ui->label_11->setText("BP3");
           ui->label_12->setText("BP2");
           ui->label_13->setText("BP1");
           ui->label_14->setText("BP0");
           ui->label_15->setText("TB");
           ui->label_16->setText("WP_DIS");
           ui->label_17->setText("X");

           ui->label_20->setText("CFG2");
           ui->label_21->setText("CFG1");
           ui->label_22->setText("LOT_EN");
           ui->label_23->setText("ECC-E");
           ui->label_24->setText("X");
           ui->label_25->setText("X");
           ui->label_26->setText("CFG0");
           ui->label_27->setText("X");

           ui->label_30->setText("CRBSY");
           ui->label_31->setText("ECCS2");
           ui->label_32->setText("ECCS1");
           ui->label_33->setText("ECCS0");
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

void DialogNANDSr::clearAllFields()
{
    auto lineEdits = findChildren<QLineEdit*>();
    for (auto lineEdit : lineEdits)
    {
        lineEdit->clear();
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
