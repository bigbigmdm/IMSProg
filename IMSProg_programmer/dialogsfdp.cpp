/*
 * Copyright (C) 2023 - 2025 Mikhail Medvedev <e-ink-reader@yandex.ru>
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
#include "dialogsfdp.h"
#include "ui_dialogsfdp.h"
#include <QValidator>
#include <QRegExp>
#include "unistd.h"
#include "memory"
#include <QDebug>
#include <QRegularExpression>
#include "hexutility.h"
DialogSFDP::DialogSFDP(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSFDP)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    setLineEditFilter();
    setRegStatus(1, true);
    numOfRegisters = 3; // 3-not reading, 2 - three registers, 1 - two registers, 0 - one register
}

DialogSFDP::~DialogSFDP()
{
    delete ui;
}

void DialogSFDP::legendPrint(QString basic, QString extended, QString manufacture)
{
QString l0 ="", l1="", l2="", l3="", h0="", h1="", h2="", h3="", h4="";
l0.append(QString(tr("Legend:")));
l1.append(QString(tr(" - Basic area")));
l2.append(QString(tr(" - Extended area")));
l3.append(QString(tr(" - Manufacture area")));
h0.append(QString("</span>"));
h1.append(QString("<html><head/><body><p>" + l0 + "</p><p>"));
h2.append(QString("<br><span style=\" background:#f77;\">"));
h3.append(QString("<br><span style=\" background:#7f7;\">"));
h4.append(QString("</p></body></html>"));
ui->label_9->setText(h1 + basic + h0 + l1 + h2 + extended + h0 + l2 + h3 + manufacture + h0 + l3 + h4);
}

void DialogSFDP::on_pushButton_clicked()
{
    int stCH341 = 0;
    uint64_t sfdpSize = 0;
    uint32_t sfdpBlockSize = 0;
    bool sfdpSupport = false;
    unsigned char i, imax, twoAreaAddress=0xff, manufAreaAddress=0xff, twoAreaLen = 0xff, manAreaLen = 0xff;
    uint8_t jedecMan=0xff, idSize;
    std::shared_ptr<uint8_t[]> sfdpBuf(new uint8_t[256]);
    QString regData = "", VCCmin = "", VCCmax = "", speeds = "Single", addrTxt="";
    int retval = 0;
    stCH341 = ch341a_spi_init();
    ui->lineEdit_vcc_max->setText("");
    ui->lineEdit_vcc_min->setText("");
    ui->lineEdit_block->setText("");
    ui->lineEdit_size->setText("");
    ui->lineEdit_speeds->setText("");
    ui->lineEdit_otp->setText("");
    legendPrint("**", "**", "**");
    if (stCH341 == 0)
    {
        //Reading JEDEC ID
        SPI_CONTROLLER_Chip_Select_Low();
        SPI_CONTROLLER_Write_One_Byte(0x9f);
        retval = SPI_CONTROLLER_Read_NByte(sfdpBuf.get(),3,SPI_CONTROLLER_SPEED_SINGLE);
        SPI_CONTROLLER_Chip_Select_High();
        if (retval)
        {
           QMessageBox::about(this, tr("Error"), tr("Error reading JEDEC ID!"));
           return;
        }
        jedecMan = sfdpBuf[0];
        ui->lineEdit_jedec0->setText(bytePrint(sfdpBuf[0]));
        ui->lineEdit_jedec1->setText(bytePrint(sfdpBuf[1]));
        ui->lineEdit_jedec2->setText(bytePrint(sfdpBuf[2]));


        // Reading SFDP. Transfer to ch341 0x5a
        SPI_CONTROLLER_Chip_Select_Low();
        SPI_CONTROLLER_Write_One_Byte(0x5a);
        SPI_CONTROLLER_Write_One_Byte(0x00);
        SPI_CONTROLLER_Write_One_Byte(0x00);
        SPI_CONTROLLER_Write_One_Byte(0x00);
        SPI_CONTROLLER_Write_One_Byte(0x00);
        retval = SPI_CONTROLLER_Read_NByte(sfdpBuf.get(),256,SPI_CONTROLLER_SPEED_SINGLE);
        SPI_CONTROLLER_Chip_Select_High();
        if (retval)
        {
           QMessageBox::about(this, tr("Error"), tr("Error reading register!"));
           return;
        }

        //Analyse-parsing SFDP
        if ((sfdpBuf[0] == 0x53) && (sfdpBuf[1] == 0x46) && (sfdpBuf[2] == 0x44) && (sfdpBuf[3] == 0x50))
        {
           ui->lineEdit_sfdp->setText("Yes");
           sfdpSupport = true;
        }
        else
        {
           ui->lineEdit_sfdp->setText("No");
           sfdpSupport = false;
           ui->label->setText("");
           ui->label_10->setText("");
        }
        if (sfdpSupport)
        {
           if ((sfdpBuf[0x80] == 0x53) && (sfdpBuf[0x81] == 0x46) && (sfdpBuf[0x82] == 0x44) && (sfdpBuf[0x83] == 0x50)) imax = 127;
           else imax = 254;
           twoAreaAddress = sfdpBuf[0x0c];
           twoAreaLen = sfdpBuf[0x0b] * 4;
           if (jedecMan == sfdpBuf[0x10])
           {
               manufAreaAddress = sfdpBuf[0x14];
               manAreaLen = sfdpBuf[0x13] * 4;
           }
           else
           {
               manufAreaAddress = sfdpBuf[0x1c];
               manAreaLen = sfdpBuf[0x1b] * 4;
           }
           if (manufAreaAddress != 0xff)
           {
              if ((sfdpBuf[manufAreaAddress] != 0xff) && (sfdpBuf[manufAreaAddress + 1] != 0xff))
              {
                  VCCmax = bytePrint(sfdpBuf[manufAreaAddress + 1]) + bytePrint(sfdpBuf[manufAreaAddress]);
                  VCCmin = bytePrint(sfdpBuf[manufAreaAddress + 3]) + bytePrint(sfdpBuf[manufAreaAddress + 2]);
                  VCCmax.insert(1, ".");
                  VCCmin.insert(1, ".");
                  ui->lineEdit_vcc_max->setText(VCCmax);
                  ui->lineEdit_vcc_min->setText(VCCmin);
              }
              if (sfdpBuf[manufAreaAddress + 9] != 0xff)
              {
                 if ((sfdpBuf[manufAreaAddress + 9] & 0x08) != 0) ui->lineEdit_otp->setText("Yes");
                 if ((sfdpBuf[manufAreaAddress + 9] & 0x08) == 0) ui->lineEdit_otp->setText("No");
              }
              else ui->lineEdit_otp->setText("");
           }
           else
           {
               ui->lineEdit_vcc_max->setText("");
               ui->lineEdit_vcc_min->setText("");
           }
           sfdpSize =( sfdpBuf[twoAreaAddress + 4] + sfdpBuf [twoAreaAddress +5] * 256 + sfdpBuf[twoAreaAddress +6] * 256 * 256 + sfdpBuf[twoAreaAddress + 7] * 256 * 256 * 256 + 1) /8 /1024 ;
           ui->lineEdit_size->setText(QString::number(sfdpSize) + " K");
           if (sfdpBuf[twoAreaAddress + 0x20] != 0xff)
           {
               sfdpBlockSize = (1 << sfdpBuf[twoAreaAddress + 0x20]) / 1024;
               ui->lineEdit_block->setText(QString::number(sfdpBlockSize) + " K");
           }
           else
           {
               ui->lineEdit_block->setText("");
           }
           if (sfdpBuf[twoAreaAddress + 0x0f] == 0xbb) speeds = speeds + "/Dual";
           if (sfdpBuf[twoAreaAddress + 0x09] == 0xeb) speeds = speeds + "/Quad";
           ui->lineEdit_speeds->setText(speeds);
           legendPrint("00", bytePrint(twoAreaAddress), bytePrint(manufAreaAddress));
           //HEXDUMP
           regData = tr("<html><head/><body><p> Hex SFDP register data:\n");
           addrTxt = tr("<html><head/><body><p>Addr:<br>");
           for (i=0; i<232;i=i+16)
           {
               addrTxt = addrTxt + "0" + bytePrint(i) + "><br>";
           }
           addrTxt = addrTxt + "0F0></p></body></html>";
           ui->label_10->setText(addrTxt);
           for (i=0;i<=imax;i++)
           {
              if (i % 16 == 0) regData = regData +  "<br> ";
              if (i == 0x0c) regData = regData + "<span style=\" background:#f77;\">";
              if (i == 0x0d) regData = regData + "</span>";

              if ((i == 0x14) && (jedecMan == sfdpBuf[0x10]) && (sfdpBuf[0x06] !=0)) regData = regData + "<span style=\" background:#7f7;\">";
              if ((i == 0x15) && (jedecMan == sfdpBuf[0x10]) && (sfdpBuf[0x06] !=0)) regData = regData + "</span>";
              if ((i == 0x1c) && (jedecMan != sfdpBuf[0x10]) && (sfdpBuf[0x06] !=0)) regData = regData + "<span style=\" background:#7f7;\">";
              if ((i == 0x1d) && (jedecMan != sfdpBuf[0x10]) && (sfdpBuf[0x06] !=0)) regData = regData + "</span>";

              if (i == twoAreaAddress) regData = regData + "<span style=\" background:#f77;\">";
              if (i == twoAreaAddress + twoAreaLen) regData = regData + "</span>";
              if (i == manufAreaAddress) regData = regData + "<span style=\" background:#7f7;\">";
              if (i == manufAreaAddress + manAreaLen) regData = regData + "</span>";
              regData = regData + bytePrint(sfdpBuf[i]) + " ";
           }
           regData = regData + "</p></body></html>";
           ui->label->setText(regData);

        }
        //READING STATUS REGISTER 0
        SPI_CONTROLLER_Chip_Select_Low();
        SPI_CONTROLLER_Write_One_Byte(0x05);
        retval = SPI_CONTROLLER_Read_NByte(sfdpBuf.get(),2,SPI_CONTROLLER_SPEED_SINGLE);
        SPI_CONTROLLER_Chip_Select_High();
        usleep(1);
        if (retval)
        {
           QMessageBox::about(this, tr("Error"), tr("Error reading register!"));
           return;
        }
        printRegData(0, sfdpBuf[0]);
        //READING STATUS REGISTER 1
        numOfRegisters = 0;
        SPI_CONTROLLER_Chip_Select_Low();
        SPI_CONTROLLER_Write_One_Byte(0x35);
        retval = SPI_CONTROLLER_Read_NByte(sfdpBuf.get(),2,SPI_CONTROLLER_SPEED_SINGLE);
        SPI_CONTROLLER_Chip_Select_High();
        usleep(1);
        if (retval)
        {
           QMessageBox::about(this, tr("Error"), tr("Error reading register!"));
           return;
        }
        if (sfdpBuf[0] != 0xff)
        {
            numOfRegisters++;
            setRegStatus(1, true);
            printRegData(1, sfdpBuf[0]);
        }
        else setRegStatus(1, false);

        //READING STATUS REGISTER 2
        SPI_CONTROLLER_Chip_Select_Low();
        SPI_CONTROLLER_Write_One_Byte(0x15);
        retval = SPI_CONTROLLER_Read_NByte(sfdpBuf.get(),2,SPI_CONTROLLER_SPEED_SINGLE);
        SPI_CONTROLLER_Chip_Select_High();
        usleep(1);
        if (retval)
        {
           QMessageBox::about(this, tr("Error"), tr("Error reading register!"));
           return;
        }
        if (sfdpBuf[0] != 0xff)
        {
            numOfRegisters++;
            setRegStatus(2, true);
            printRegData(2,sfdpBuf[0]);
        }
        else setRegStatus(2, false);

        //Reading Unique ID
        SPI_CONTROLLER_Chip_Select_Low();
        SPI_CONTROLLER_Write_One_Byte(0x4b);
        SPI_CONTROLLER_Write_One_Byte(0x00);
        SPI_CONTROLLER_Write_One_Byte(0x00);
        SPI_CONTROLLER_Write_One_Byte(0x00);
        SPI_CONTROLLER_Write_One_Byte(0x00);
        retval = SPI_CONTROLLER_Read_NByte(sfdpBuf.get(),16,SPI_CONTROLLER_SPEED_SINGLE);
        SPI_CONTROLLER_Chip_Select_High();
        if (retval)
        {
           QMessageBox::about(this, tr("Error"), tr("Error reading unique ID!"));
           return;
        }
        if ((sfdpBuf[0x00] == sfdpBuf[0x08]) && (sfdpBuf[0x01] == sfdpBuf[0x09]) && (sfdpBuf[0x02] == sfdpBuf[0x0a]) && (sfdpBuf[0x03] == sfdpBuf[0x0b])) idSize = 8;
        else idSize = 16;
        regData = "";
        for (i=0; i<idSize; i++)
        {
            regData = regData + bytePrint(sfdpBuf[i]);
        }
        ui->lineEdit_chipid->setText(regData);

        //Close the CH341a device
        ch341a_spi_shutdown();
    }
    else QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));

}

void DialogSFDP::on_pushButton_2_clicked()
{
    DialogSFDP::close();
}

void DialogSFDP::on_pushButton_3_clicked()
{
   if (numOfRegisters < 3)
   {
    int stCH341 = 0;
    stCH341 = ch341a_spi_init();
    if (stCH341 == 0)
    {
       //scanning data from the form
       uint8_t r[3], k, i;
       int j;
       r[0] = 0;
       r[1] = 0;
       r[2] = 0;
       for (k = 0; k < 3; k++)
       {
           i = 128;
           for (j = 7; j >= 0; j--)
           {
               QString currRegName = "lineEdit_sr" + QString::number(k) + QString::number(j);
                       for (QLineEdit* edit : findChildren<QLineEdit*>())
                       {
                           if (edit->objectName() == currRegName)
                           {
                               if (QString::compare(edit->text(), "0", Qt::CaseInsensitive)) r[k] = r[k] + i;
                               i = i / 2;
                           }
                       }
           }
       }

       //Writing status registers 0,1 for Winbond

       SPI_CONTROLLER_Chip_Select_Low();
       SPI_CONTROLLER_Write_One_Byte(0x06);
       SPI_CONTROLLER_Chip_Select_High();
       usleep(1);

       SPI_CONTROLLER_Chip_Select_Low();
       SPI_CONTROLLER_Write_One_Byte(0x50);
       SPI_CONTROLLER_Chip_Select_High();
       usleep(1);

       SPI_CONTROLLER_Chip_Select_Low();
       SPI_CONTROLLER_Write_One_Byte(0x01);
       SPI_CONTROLLER_Write_One_Byte(r[0]);
       if (numOfRegisters > 0) SPI_CONTROLLER_Write_One_Byte(r[1]);
       SPI_CONTROLLER_Chip_Select_High();
       usleep(1);

       SPI_CONTROLLER_Chip_Select_Low();
       SPI_CONTROLLER_Write_One_Byte(0x04);
       SPI_CONTROLLER_Chip_Select_High();
       usleep(1);

       //Close the CH341a device
       ch341a_spi_shutdown();

       //Writing status registers 0,1 for not Winbond
       if (numOfRegisters > 0)
       {
           stCH341 = ch341a_spi_init();

           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x06);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);

           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x50);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);

           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x01);
           SPI_CONTROLLER_Write_One_Byte(r[0]);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);

           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x04);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);

           //Close the CH341a device
           ch341a_spi_shutdown();
           usleep(1);

           stCH341 = ch341a_spi_init();
           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x06);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);

           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x50);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);

           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x31);
           SPI_CONTROLLER_Write_One_Byte(r[1]);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);

           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x04);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);

           //Close the CH341a device
           ch341a_spi_shutdown();
       }

       //Writing status register 2
       if (numOfRegisters > 1)
       {
           stCH341 = ch341a_spi_init();

           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x06);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);

           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x50);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);

           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x11);
           SPI_CONTROLLER_Write_One_Byte(r[2]);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);

           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x04);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);

           //Close the CH341a device
           ch341a_spi_shutdown();
       }
   }
   else QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
  }
  else QMessageBox::about(this, tr("Error"), tr("Before writing the registers, please press the `Read` button!"));
}

void DialogSFDP::setLineEditFilter()
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

void DialogSFDP:: setRegStatus(uint8_t regNumber, bool state)
{
    QString searchText = "lineEdit_sr" + QString::number(regNumber) + "\\d+";
    QRegularExpression regex(searchText);

        for (QLineEdit* edit : findChildren<QLineEdit*>())
        {
            if (regex.match(edit->objectName()).hasMatch())
            {
                if (state == false)
                {
                    edit->setDisabled(true);
                    edit->setText("");
                }
                else
                {
                    edit->setDisabled(false);
                }
            }
        }
        if (state == true)
        {
            if (regNumber == 1) ui->label_11->setDisabled(false);
            if (regNumber == 2) ui->label_34->setDisabled(false);
        }
        else
        {
            if (regNumber == 1) ui->label_11->setDisabled(true);
            if (regNumber == 2) ui->label_34->setDisabled(true);
        }
}

void DialogSFDP::closeEvent(QCloseEvent* event)
{
    emit closeRequestHasArrived();
    QWidget::closeEvent(event);
}

void DialogSFDP::printRegData(uint8_t regNumber, uint8_t regData)
{
    uint8_t i;
    int j;
    i = 128;

    for (j = 7; j >= 0; j--)
    {
        QString currRegName = "lineEdit_sr" + QString::number(regNumber) + QString::number(j);
        for (QLineEdit* edit : findChildren<QLineEdit*>())
        {
            if (edit->objectName() == currRegName)
            {
                edit->setText(QString::number(((regData & i) >> j)));
            }
        }
        i = i / 2;
    }
}
