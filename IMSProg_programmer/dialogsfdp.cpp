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
#include "dialogsfdp.h"
#include "ui_dialogsfdp.h"
#include <QValidator>
#include <QRegExp>
#include "unistd.h"
#include "memory"
#include <QDebug>
DialogSFDP::DialogSFDP(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSFDP)
{
    ui->setupUi(this);
    setLineEditFilter();
    r1Enable();
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
        ui->lineEdit_sr07->setText(QString::number(((sfdpBuf[0] & 128) >> 7)));
        ui->lineEdit_sr06->setText(QString::number(((sfdpBuf[0] & 64) >> 6)));
        ui->lineEdit_sr05->setText(QString::number(((sfdpBuf[0] & 32) >> 5)));
        ui->lineEdit_sr04->setText(QString::number(((sfdpBuf[0] & 16) >> 4)));
        ui->lineEdit_sr03->setText(QString::number(((sfdpBuf[0] & 8) >> 3)));
        ui->lineEdit_sr02->setText(QString::number(((sfdpBuf[0] & 4) >> 2)));
        ui->lineEdit_sr01->setText(QString::number(((sfdpBuf[0] & 2) >> 1)));
        ui->lineEdit_sr00->setText(QString::number((sfdpBuf[0] & 1)));
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
            r1Enable();
            ui->lineEdit_sr17->setText(QString::number(((sfdpBuf[0] & 128) >> 7)));
            ui->lineEdit_sr16->setText(QString::number(((sfdpBuf[0] & 64) >> 6)));
            ui->lineEdit_sr15->setText(QString::number(((sfdpBuf[0] & 32) >> 5)));
            ui->lineEdit_sr14->setText(QString::number(((sfdpBuf[0] & 16) >> 4)));
            ui->lineEdit_sr13->setText(QString::number(((sfdpBuf[0] & 8) >> 3)));
            ui->lineEdit_sr12->setText(QString::number(((sfdpBuf[0] & 4) >> 2)));
            ui->lineEdit_sr11->setText(QString::number(((sfdpBuf[0] & 2) >> 1)));
            ui->lineEdit_sr10->setText(QString::number((sfdpBuf[0] & 1)));
        }
        else r1Disable();

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
            r2Enable();
            ui->lineEdit_sr27->setText(QString::number(((sfdpBuf[0] & 128) >> 7)));
            ui->lineEdit_sr26->setText(QString::number(((sfdpBuf[0] & 64) >> 6)));
            ui->lineEdit_sr25->setText(QString::number(((sfdpBuf[0] & 32) >> 5)));
            ui->lineEdit_sr24->setText(QString::number(((sfdpBuf[0] & 16) >> 4)));
            ui->lineEdit_sr23->setText(QString::number(((sfdpBuf[0] & 8) >> 3)));
            ui->lineEdit_sr22->setText(QString::number(((sfdpBuf[0] & 4) >> 2)));
            ui->lineEdit_sr21->setText(QString::number(((sfdpBuf[0] & 2) >> 1)));
            ui->lineEdit_sr20->setText(QString::number((sfdpBuf[0] & 1)));
        }
        else r2Disable();

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
       uint8_t r0, r1, r2;

       r0 = 0;
       r1 = 0;
       r2 = 0;
       if (QString::compare(ui->lineEdit_sr07->text(), "0", Qt::CaseInsensitive)) r0 = r0 + 128;
       if (QString::compare(ui->lineEdit_sr06->text(), "0", Qt::CaseInsensitive)) r0 = r0 +  64;
       if (QString::compare(ui->lineEdit_sr05->text(), "0", Qt::CaseInsensitive)) r0 = r0 +  32;
       if (QString::compare(ui->lineEdit_sr04->text(), "0", Qt::CaseInsensitive)) r0 = r0 +  16;
       if (QString::compare(ui->lineEdit_sr03->text(), "0", Qt::CaseInsensitive)) r0 = r0 +   8;
       if (QString::compare(ui->lineEdit_sr02->text(), "0", Qt::CaseInsensitive)) r0 = r0 +   4;
       if (QString::compare(ui->lineEdit_sr01->text(), "0", Qt::CaseInsensitive)) r0 = r0 +   2;
       if (QString::compare(ui->lineEdit_sr00->text(), "0", Qt::CaseInsensitive)) r0 = r0 +   1;

       if (QString::compare(ui->lineEdit_sr17->text(), "0", Qt::CaseInsensitive)) r1 = r1 + 128;
       if (QString::compare(ui->lineEdit_sr16->text(), "0", Qt::CaseInsensitive)) r1 = r1 +  64;
       if (QString::compare(ui->lineEdit_sr15->text(), "0", Qt::CaseInsensitive)) r1 = r1 +  32;
       if (QString::compare(ui->lineEdit_sr14->text(), "0", Qt::CaseInsensitive)) r1 = r1 +  16;
       if (QString::compare(ui->lineEdit_sr13->text(), "0", Qt::CaseInsensitive)) r1 = r1 +   8;
       if (QString::compare(ui->lineEdit_sr12->text(), "0", Qt::CaseInsensitive)) r1 = r1 +   4;
       if (QString::compare(ui->lineEdit_sr11->text(), "0", Qt::CaseInsensitive)) r1 = r1 +   2;
       if (QString::compare(ui->lineEdit_sr10->text(), "0", Qt::CaseInsensitive)) r1 = r1 +   1;

       if (QString::compare(ui->lineEdit_sr27->text(), "0", Qt::CaseInsensitive)) r2 = r2 + 128;
       if (QString::compare(ui->lineEdit_sr26->text(), "0", Qt::CaseInsensitive)) r2 = r2 +  64;
       if (QString::compare(ui->lineEdit_sr25->text(), "0", Qt::CaseInsensitive)) r2 = r2 +  32;
       if (QString::compare(ui->lineEdit_sr24->text(), "0", Qt::CaseInsensitive)) r2 = r2 +  16;
       if (QString::compare(ui->lineEdit_sr23->text(), "0", Qt::CaseInsensitive)) r2 = r2 +   8;
       if (QString::compare(ui->lineEdit_sr22->text(), "0", Qt::CaseInsensitive)) r2 = r2 +   4;
       if (QString::compare(ui->lineEdit_sr21->text(), "0", Qt::CaseInsensitive)) r2 = r2 +   2;
       if (QString::compare(ui->lineEdit_sr20->text(), "0", Qt::CaseInsensitive)) r2 = r2 +   1;

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
       SPI_CONTROLLER_Write_One_Byte(r0);
       if (numOfRegisters > 0) SPI_CONTROLLER_Write_One_Byte(r1);
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
           SPI_CONTROLLER_Write_One_Byte(r0);
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
           SPI_CONTROLLER_Write_One_Byte(r1);
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
           SPI_CONTROLLER_Write_One_Byte(r2);
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
    ui->lineEdit_sr00->setValidator(validator);
    ui->lineEdit_sr01->setValidator(validator);
    ui->lineEdit_sr02->setValidator(validator);
    ui->lineEdit_sr03->setValidator(validator);
    ui->lineEdit_sr04->setValidator(validator);
    ui->lineEdit_sr05->setValidator(validator);
    ui->lineEdit_sr06->setValidator(validator);
    ui->lineEdit_sr07->setValidator(validator);

    ui->lineEdit_sr10->setValidator(validator);
    ui->lineEdit_sr11->setValidator(validator);
    ui->lineEdit_sr12->setValidator(validator);
    ui->lineEdit_sr13->setValidator(validator);
    ui->lineEdit_sr14->setValidator(validator);
    ui->lineEdit_sr15->setValidator(validator);
    ui->lineEdit_sr16->setValidator(validator);
    ui->lineEdit_sr17->setValidator(validator);

    ui->lineEdit_sr20->setValidator(validator);
    ui->lineEdit_sr21->setValidator(validator);
    ui->lineEdit_sr22->setValidator(validator);
    ui->lineEdit_sr23->setValidator(validator);
    ui->lineEdit_sr24->setValidator(validator);
    ui->lineEdit_sr25->setValidator(validator);
    ui->lineEdit_sr26->setValidator(validator);
    ui->lineEdit_sr27->setValidator(validator);
}

void DialogSFDP::r1Disable()
{
   ui->lineEdit_sr10->setDisabled(true);
   ui->lineEdit_sr11->setDisabled(true);
   ui->lineEdit_sr12->setDisabled(true);
   ui->lineEdit_sr13->setDisabled(true);
   ui->lineEdit_sr14->setDisabled(true);
   ui->lineEdit_sr15->setDisabled(true);
   ui->lineEdit_sr16->setDisabled(true);
   ui->lineEdit_sr17->setDisabled(true);
   ui->label_11->setDisabled(true);
   ui->lineEdit_sr10->setText("");
   ui->lineEdit_sr11->setText("");
   ui->lineEdit_sr12->setText("");
   ui->lineEdit_sr13->setText("");
   ui->lineEdit_sr14->setText("");
   ui->lineEdit_sr15->setText("");
   ui->lineEdit_sr16->setText("");
   ui->lineEdit_sr17->setText("");
}

void DialogSFDP::r2Disable()
{
   ui->lineEdit_sr20->setDisabled(true);
   ui->lineEdit_sr21->setDisabled(true);
   ui->lineEdit_sr22->setDisabled(true);
   ui->lineEdit_sr23->setDisabled(true);
   ui->lineEdit_sr24->setDisabled(true);
   ui->lineEdit_sr25->setDisabled(true);
   ui->lineEdit_sr26->setDisabled(true);
   ui->lineEdit_sr27->setDisabled(true);
   ui->label_34->setDisabled(true);
   ui->lineEdit_sr20->setText("");
   ui->lineEdit_sr21->setText("");
   ui->lineEdit_sr22->setText("");
   ui->lineEdit_sr23->setText("");
   ui->lineEdit_sr24->setText("");
   ui->lineEdit_sr25->setText("");
   ui->lineEdit_sr26->setText("");
   ui->lineEdit_sr27->setText("");
}

void DialogSFDP::r1Enable()
{
    ui->lineEdit_sr10->setDisabled(false);
    ui->lineEdit_sr11->setDisabled(false);
    ui->lineEdit_sr12->setDisabled(false);
    ui->lineEdit_sr13->setDisabled(false);
    ui->lineEdit_sr14->setDisabled(false);
    ui->lineEdit_sr15->setDisabled(false);
    ui->lineEdit_sr16->setDisabled(false);
    ui->lineEdit_sr17->setDisabled(false);
    ui->label_11->setDisabled(false);
}

void DialogSFDP::r2Enable()
{
    ui->lineEdit_sr20->setDisabled(false);
    ui->lineEdit_sr21->setDisabled(false);
    ui->lineEdit_sr22->setDisabled(false);
    ui->lineEdit_sr23->setDisabled(false);
    ui->lineEdit_sr24->setDisabled(false);
    ui->lineEdit_sr25->setDisabled(false);
    ui->lineEdit_sr26->setDisabled(false);
    ui->lineEdit_sr27->setDisabled(false);
    ui->label_34->setDisabled(false);
}

void DialogSFDP::closeEvent(QCloseEvent* event)
{
    emit closeRequestHasArrived();
    QWidget::closeEvent(event);
}
