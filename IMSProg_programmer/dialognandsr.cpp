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
#include <QLabel>
#include <QWidget>
#include <QApplication>
#include <QValidator>
#include <QString>
#include <QStringList>
#include "unistd.h"
#include "hexutility.h"
#include "memory"
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
    QRegularExpression reHex( "[0-1]{1}" );
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(reHex, this);
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
        int i, j, pageSize, numBlocks, pagesPerBlock;
        QString buftxt = "";
        otp = false;
        //Clearing fields
        clearAllFields();
        ui->textEdit_ID->clear();
        //READING STATUS REGISTERS
        std::shared_ptr<uint8_t[]> buf(new uint8_t[512]);
        QString currRegName;
        int retval;
        int currRegister;
        uint8_t  currBit, currByte, idBlockAddr;
        int stCH341 = 0;
        idBlockAddr = 0x00;
        stCH341 = ch341a_spi_init();
        if (stCH341 == 0)
            {
            for (currRegister = 0; currRegister < 5; currRegister++)
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
                        currByte = static_cast<uint8_t>(currByte << 1);
                    }
                }
            }

                regReaded = true;
                //READING PARAMETER PAGE
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
                if (!((buf[0] == 0x4f) && (buf[1] == 0x4e) && (buf[2] == 0x46) && (buf[3] == 0x49)))
                {
                    //Non standard parameter page placed
                    SPI_CONTROLLER_Chip_Select_Low();
                    SPI_CONTROLLER_Write_One_Byte(0x13);
                    SPI_CONTROLLER_Write_One_Byte(0x00);
                    SPI_CONTROLLER_Write_One_Byte(0x00);
                    SPI_CONTROLLER_Write_One_Byte(0x04);
                    SPI_CONTROLLER_Chip_Select_High();
                    usleep(1000);
                    SPI_CONTROLLER_Chip_Select_Low();
                    SPI_CONTROLLER_Write_One_Byte(0x03);
                    SPI_CONTROLLER_Write_One_Byte(0x00);
                    SPI_CONTROLLER_Write_One_Byte(0x00);
                    SPI_CONTROLLER_Write_One_Byte(0x00);
                    retval = SPI_CONTROLLER_Read_NByte(buf.get(),256,SPI_CONTROLLER_SPEED_SINGLE);
                    SPI_CONTROLLER_Chip_Select_High();
                    idBlockAddr = 0x06;
                }

                if ((buf[0] == 0x4f) && (buf[1] == 0x4e) && (buf[2] == 0x46) && (buf[3] == 0x49))
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
                    ui->lineEdit_man->setText(strtrip(buftxt));
                    //Model
                    buftxt.clear();
                    for (i = 44; i < 64; i++) buftxt = buftxt + static_cast<char>(buf[i]);
                    ui->lineEdit_model->setText(strtrip(buftxt));
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
                SPI_CONTROLLER_Write_One_Byte(idBlockAddr);
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
                    currByte = static_cast<uint8_t>(currByte << 1);
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
}

void DialogNANDSr::closeEvent(QCloseEvent* event)
{
    emit closeRequestHasArrived();
    QWidget::closeEvent(event);
}

void DialogNANDSr::setPattern(const uint pattern)
{
    allRegEnabled();
    QStringList r0, r1, r2, r3, r4;
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
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("SRP0  ,BP3   ,BP2   ,BP1   ,BP0   ,TB    ,WP-E  ,SRP1 "));
           setRegLabels(1, QString("OTP-L ,OTP-E ,SR1-L ,ECC-E ,BUF   ,X     ,X     ,QE   "));
           setRegLabels(2, QString("X     ,LUT-F ,ECC-1 ,ECC-0 ,P-FAIL,E-FAIL,WEL   ,BUSY "));
         break;

         case 1: //Gigadevice 1
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xd0;
           RegNumbers[4] = 0xf0;
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("BWRD  ,X     ,BP2   ,BP1   ,BP0   ,INV   ,CMP   ,X    "));
           setRegLabels(1, QString("OTP-P ,OTP-E ,X     ,ECC-E ,X     ,X     ,X     ,QE   "));
           setRegLabels(2, QString("X     ,X     ,ECC-1 ,ECC-0 ,P-FAIL,E-FAIL,WEL   ,BUSY "));
           setRegLabels(3, QString("X     ,DS_S1 ,DS-S0 ,X     ,X     ,X     ,X     ,X    "));
           setRegLabels(4, QString("X     ,X     ,ECCS1 ,ECCS0 ,BPS   ,X     ,X     ,X    "));
         break;

         case 2: //Gigadevice 2
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xff;
           RegNumbers[4] = 0xff;
           setRegDisabled(3);
           setRegDisabled(4);
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("BWRD  ,X     ,BP2   ,BP1   ,BP0   ,INV   ,CMP   ,X    "));
           setRegLabels(1, QString("OTP-P ,OTP-E ,X     ,ECC-E ,X     ,X     ,X     ,QE   "));
           setRegLabels(2, QString("X     ,X     ,ECC-1 ,ECC-0 ,P-FAIL,E-FAIL,WEL   ,BUSY "));

         break;
         case 3: //MXIC
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xe0;
           RegNumbers[4] = 0x10;
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("BPWRD ,X     ,BP2   ,BP1   ,BP0   ,INV   ,CMP   ,SP    "));
           setRegLabels(1, QString("OTP-P ,OTP-E ,X     ,ECC-E ,X     ,CONT  ,X     ,QE    "));
           setRegLabels(2, QString("CRBSY ,BBMT_F,ECC-1 ,ECC-0 ,P-FAIL,E-FAIL,WEL   ,BUSY  "));
           setRegLabels(3, QString("DS_IO ,DS_IO ,X     ,X     ,X     ,X     ,X     ,X     "));
           setRegLabels(4, QString("BFT3  ,BFT2  ,BFT1  ,BFT00 ,X     ,X     ,X     ,ENPGM "));
         break;

         case 4: //ESMT
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xff;
           RegNumbers[4] = 0xff;
           setRegDisabled(3);
           setRegDisabled(4);
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("BWRD  ,BP3   ,BP2   ,BP1   ,BP0   ,TB    ,WP_DIS,X    "));
           setRegLabels(1, QString("CFG2  ,CFG1  ,LOT_EN,ECC-E ,X     ,X     ,CFG0  ,X    "));
           setRegLabels(2, QString("CRBSY ,ECCS2 ,ECCS1 ,ECCS0 ,P-FAIL,E-FAIL,WEL   ,BUSY "));
         break;

         case 5: //ESMT2
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xd0;
           RegNumbers[4] = 0xff;
           setRegDisabled(4);
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("PRP0  ,BP3   ,BP2   ,BP1   ,BP0   ,T/BP  ,WPE   ,PRP1 "));
           setRegLabels(1, QString("OTP-P ,OTP-E ,PR-L  ,ECC-E ,X     ,X     ,X     ,X    "));
           setRegLabels(2, QString("X     ,X     ,ECC_S1,ECC_S0,P-FAIL,E-FAIL,WEL  ,BUSY  "));
           setRegLabels(3, QString("X     ,DRV_S1,DRV_S0,X     ,X     ,X     ,X     ,X    "));
         break;

         case 6: //MXIC2
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xe0;
           RegNumbers[4] = 0x10;
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("BPWRD ,X     ,BP2   ,BP1   ,BP0   ,INV    ,CMP   ,SP    "));
           setRegLabels(1, QString("OTP-P ,OTP-E ,X     ,ECC-E ,X     ,X      ,X     ,QE    "));
           setRegLabels(2, QString("CRBSY ,X     ,X     ,X     ,P-FAIL,E-FAIL ,WEL   ,BUSY  "));
           setRegLabels(3, QString("DS_IO ,DS_IO ,X     ,X     ,X     ,X      ,X     ,X     "));
           setRegLabels(4, QString("X     ,X     ,X     ,X     ,X     ,RANDOPT,RANDEN,ENPGM "));
         break;
         case 7: //MXIC3
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xff;
           RegNumbers[4] = 0xff;
           setRegDisabled(3);
           setRegDisabled(4);
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("BPWRD ,X     ,BP2   ,BP1   ,BP0   ,INV   ,CMP   ,SP    "));
           setRegLabels(1, QString("OTP-P ,OTP-E ,X     ,ECC-E ,X     ,X     ,X     ,QE    "));
           setRegLabels(2, QString("X     ,X     ,X     ,X     ,P-FAIL,E-FAIL,WEL   ,BUSY  "));
         break;
         case 8: //Winbond2
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xff;
           RegNumbers[4] = 0xff;
           setRegDisabled(3);
           setRegDisabled(4);
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("SRP0  ,BP3   ,BP2   ,BP1   ,BP0   ,TB    ,WP-E  ,SRP1  "));
           setRegLabels(1, QString("OTP-L ,OTP-E ,SR1-L ,ECC-E ,BUF   ,OSD-1 ,OSD-0 ,H-DIS "));
           setRegLabels(2, QString("X     ,LUT-F ,ECC-1 ,ECC-0 ,P-FAIL,E-FAIL,WEL   ,BUSY  "));
         break;
         case 9: //Winbond3
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xd0;
           RegNumbers[4] = 0xff;
           setRegDisabled(4);
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("SRP0  ,BP3   ,BP2   ,BP1   ,BP0   ,TB    ,WP-E  ,SRP1  "));
           setRegLabels(1, QString("OTP-L ,OTP-E ,SR1-L ,ECC-E ,BUF   ,OSD-1 ,OSD-0 ,H-DIS "));
           setRegLabels(2, QString("X     ,LUT-F ,ECC-1 ,ECC-0 ,P-FAIL,E-FAIL,WEL   ,BUSY  "));
           setRegLabels(3, QString("X     ,ODS1  ,ODS0  ,X     ,DLP-E ,HS    ,X     ,X     "));
         break;
         case 10: //Dosilicon
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xd0;
           RegNumbers[4] = 0xff;
           setRegDisabled(4);
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("BRWD  ,X     ,BP2   ,BP1   ,BP0   ,INV   ,CMP   ,X     "));
           setRegLabels(1, QString("OTPPRT,OTP_EN,X     ,ECC_EN,X     ,X     ,X     ,QE    "));
           setRegLabels(2, QString("ECCS3 ,ECCS2 ,ECCS1 ,ECCS0 ,P_FAIL,E_FAIL,WEL   ,BUSY  "));
           setRegLabels(3, QString("X     ,DS_IO1,DS_IO2,X     ,X     ,X     ,X     ,X     "));
         break;
         case 11: //XTX2
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xff;
           RegNumbers[4] = 0xff;
           setRegDisabled(3);
           setRegDisabled(4);
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("BRWD  ,X     ,BP2   ,BP1   ,BP0   ,INV   ,CMP   ,X     "));
           setRegLabels(1, QString("OTPPRT,OTP_EN,X     ,ECC_EN,X     ,X     ,X     ,QE    "));
           setRegLabels(2, QString("X     ,X     ,ECCS3 ,ECCS2 ,P_FAIL,E_FAIL,WEL   ,BUSY  "));
         break;
         case 12: //XTX3
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xd0;
           RegNumbers[4] = 0xff;
           setRegDisabled(4);
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("BRWD  ,X     ,BP2   ,BP1   ,BP0   ,INV   ,CMP   ,X     "));
           setRegLabels(1, QString("OTPPRT,OTP_EN,X     ,ECC_EN,CRM   ,X     ,HSE   ,QE    "));
           setRegLabels(2, QString("ECCS3 ,ECCS2 ,ECCS1 ,ECCS0 ,P_FAIL,E_FAIL,WEL   ,BUSY  "));
           setRegLabels(3, QString("X     ,DS_IO1,DS_IO2,X     ,X     ,X     ,X     ,X     "));
         break;
         case 13: //XTX
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xd0;
           RegNumbers[4] = 0xff;
           setRegDisabled(4);
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("BRWD  ,X     ,BP2   ,BP1   ,BP0   ,INV   ,CMP   ,X     "));
           setRegLabels(1, QString("OTPPRT,OTP_EN,X     ,ECC_EN,X     ,X     ,X     ,QE    "));
           setRegLabels(2, QString("ECCS3 ,ECCS2 ,ECCS1 ,ECCS0 ,P_FAIL,E_FAIL,WEL   ,BUSY  "));
           setRegLabels(3, QString("X     ,DS_IO1,DS_IO2,X     ,X     ,X     ,X     ,X     "));
         break;
         case 14: //Micron
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xd0;
           RegNumbers[4] = 0xff;
           setRegDisabled(4);
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("BRWD  ,BP3   ,BP2   ,BP1   ,BP0   ,T/BP  ,WP_DIS,X    "));
           setRegLabels(1, QString("CFG2  ,CFG1  ,LOT_EN,ECC-E,DS_S1 ,DS_S0 ,CFG0   ,CON_RD"));
           setRegLabels(2, QString("CRBSY ,ECCS2 ,ECCS1 ,ECCS0,P-FAIL,E-FAIL,WEL    ,BUSY  "));
           setRegLabels(3, QString("X     ,DS0   ,X     ,X     ,X     ,X     ,X     ,X    "));
         break;
         case 15: //Foressy
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0x80;
           RegNumbers[4] = 0x84;
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("BRWD  ,BP3   ,BP2   ,BP1   ,BP0   ,TB    ,X     ,SP    "));
           setRegLabels(1, QString("OTP-L ,OTP-E ,X     ,ECC-E ,X     ,DRV1  ,DRV0  ,QE    "));
           setRegLabels(2, QString("X     ,X     ,ECCS1 ,ECCS0 ,P-FAIL,E-FAIL,WEL   ,BUSY  "));
           setRegLabels(3, QString("X     ,X     ,X     ,X     ,S0ES3 ,S0ES2 ,S0ES1 ,S0ES0 "));
           setRegLabels(4, QString("X     ,X     ,X     ,X     ,S1ES3 ,S1ES2 ,S1ES1 ,S1ES0 "));
         break;
         case 16: //Foressy2
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xd0;
           RegNumbers[4] = 0x10;
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("BRWD  ,BP3   ,BP2   ,BP1   ,BP0   ,TB    ,X     ,SP    "));
           setRegLabels(1, QString("OTP-L ,OTP-E ,X     ,ECC-E ,X     ,DRV1  ,DRV0  ,QE    "));
           setRegLabels(2, QString("X     ,X     ,ECCS1 ,ECCS0 ,P-FAIL,E-FAIL,WEL   ,BUSY  "));
           setRegLabels(3, QString("X     ,X     ,X     ,X     ,X     ,X     ,X     ,ECC-M "));
           setRegLabels(4, QString("X     ,X     ,X     ,X     ,S0ES3 ,S0ES2 ,S0ES1 ,S0ES0 "));
         break;
         case 17: //Gigadevice 3
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xd0;
           RegNumbers[4] = 0xff;
           setRegDisabled(4);
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("BWRD  ,X     ,BP2   ,BP1   ,BP0   ,INV   ,CMP   ,X    "));
           setRegLabels(1, QString("OTP-P ,OTP-E ,X     ,ECC-E ,X     ,X     ,X     ,QE   "));
           setRegLabels(2, QString("X     ,ECC-2 ,ECC-1 ,ECC-0 ,P-FAIL,E-FAIL,WEL   ,BUSY "));
           setRegLabels(3, QString("X     ,DS_S1 ,DS-S0 ,X     ,X     ,X     ,X     ,X    "));
         break;

         case 18: //Gigadevice 1, other Security registers
           RegNumbers[0] = 0xa0;
           RegNumbers[1] = 0xb0;
           RegNumbers[2] = 0xc0;
           RegNumbers[3] = 0xd0;
           RegNumbers[4] = 0xf0;
           //                       7      6      5      4      3      2      1      0
           setRegLabels(0, QString("BWRD  ,X     ,BP2   ,BP1   ,BP0   ,INV   ,CMP   ,X    "));
           setRegLabels(1, QString("OTP-P ,OTP-E ,X     ,ECC-E ,X     ,X     ,X     ,QE   "));
           setRegLabels(2, QString("X     ,X     ,ECC-1 ,ECC-0 ,P-FAIL,E-FAIL,WEL   ,BUSY "));
           setRegLabels(3, QString("X     ,DS_S1 ,DS-S0 ,X     ,X     ,X     ,X     ,X    "));
           setRegLabels(4, QString("X     ,X     ,ECCS1 ,ECCS0 ,BPS   ,X     ,X     ,X    "));
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

void DialogNANDSr::setRegDisabled(uint8_t regNumber)
{
    QString searchTextE = "lineEdit_sr" + QString::number(regNumber) + "\\d+";
    QString searchTextL = "label_" + QString::number(regNumber) + "\\d+";
    QRegularExpression regexE(searchTextE);
    QRegularExpression regexL(searchTextL);

        for (QLineEdit* edit : findChildren<QLineEdit*>())
        {
            if (regexE.match(edit->objectName()).hasMatch())
            {
                edit->setDisabled(true);
            }
        }

        for (QLabel* label : findChildren<QLabel*>())
        {

            if (regexL.match(label->objectName()).hasMatch())
            {
                label->setText("X");
            }

        }
}

void DialogNANDSr::setRegLabels(uint8_t regNumber, QString lt)
{
  lt.replace( " ", "" );
  QStringList list = lt.split(",");
  int lastIndex = list.size() - 1;

      for (int i = 0; i <= lastIndex; ++i)
      {
          QString labelName = QString("label_") + QString::number(regNumber) + QString::number(i);

          if (QLabel* label = findChild<QLabel*>(labelName))
          {
              label->setText(list.at(lastIndex - i));
          }
      }
}

QString DialogNANDSr::strtrip(const QString& str)
{
  int n = str.size() - 1;
  for (; n >= 0; --n) {
    if (!str.at(n).isSpace()) {
      return str.left(n + 1);
    }
  }
  return "";
}
