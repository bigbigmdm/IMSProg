/*
 * Copyright (C) 2025 Mikhail Medvedev <e-ink-reader@yandex.ru>
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
#include "dialogsecurity.h"
#include "ui_dialogsecurity.h"
#include "mainwindow.h"
#include <QDebug>
#include <QFileInfo>
#include "qhexedit.h"

DialogSecurity::DialogSecurity(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSecurity)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window| Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
}

DialogSecurity::~DialogSecurity()
{
    delete ui;
}

void DialogSecurity::setAlgorithm(uint8_t currentAlg)
{
    //init variables
    int i;
    securCommands comPattern[] = {
    // id RDSCUR WRSCUR ERSCUR  ENSO  EXSO DUMRD
      { 0,     0,     0,    0,    0,     0,    0},
      { 1,  0x48,  0x42, 0x44,    0,     0,    1},  //Winbond, Gigadevice, Boya, UCUNDATA, Fudan, Zetta, Zbit, XMC
      { 2,  0x03,  0x02, 0x20, 0x3a,  0x04,    0},  //EON
      { 3,  0x68,  0x62, 0x64,    0,     0,    1},  //ISSI
      { 4,  0x03,  0x02, 0x00, 0xb1,  0xc1,    0},  //MXIC, Fidelix, Zetta
      { 5,  0x4b,  0xb1, 0x00,    0,     0,    0},  //PFlash
    };
    algSettings algSet[] = {
    //    id algType RegNum Size   rg0add   rg1add   rg2add   rg3add allErase a4byte curCommand
      { 0x00,      0,     0,   0,       0,       0,       0,       0,       0,     0,         0}, //Don't have security registers
      { 0x01,      0,     4,   4,  0x0000,  0x0001,  0x0010,  0x0011,       0,     0,         1}, //Fudan
      { 0x02,      0,     3,   8,  0x0010,  0x0020,  0x0030,       0,       0,     0,         1}, //Boya, Gigadevice, Zetta, Puya
      { 0x03,      0,     3,   4,  0x0010,  0x0020,  0x0030,       0,       0,     0,         1}, //Winbond, Boya, UCUNDATA, Zetta, Zbit, XMC, Spansion
      { 0x04,      0,     3,  16,  0x0010,  0x0020,  0x0030,       0,       0,     0,         1}, //Gigadevice, Boya, UCUNDATA, Zetta, Puya, Zbit, XMC, XTX
      { 0x05,      1,     3,   8,  0x3fd0,  0x3fe0,  0x3ff0,       0,       0,     0,         2}, //EON EN25QH32B
      { 0x06,      0,     3,   4,  0x0010,  0x0020,  0x0030,       0,       0,     1,         1}, //Winbond, Gigadevice
      { 0x07,      0,     2,   4,  0x0010,  0x0020,       0,       0,       0,     0,         1}, //UCUNDATA
      { 0x08,      0,     4,   4,  0x0000,  0x0001,  0x0002,  0x0003,       1,     0,         1}, //Gigadevice
      { 0x09,      0,     2,  16,  0x0020,  0x0030,       0,       0,       0,     0,         1}, //Gigadevice
      { 0x0a,      0,     1,  16,  0x0000,       0,       0,       0,       1,     0,         1}, //Fudan
      { 0x0b,      0,     1,   8,  0x0000,       0,       0,       0,       0,     0,         1}, //Gigadevice
      { 0x0c,      0,     4,   4,  0x0000,  0x0010,  0x0020,  0x0030,       0,     0,         1}, //Winbond
      { 0x0d,      1,     1,   8,  0x1ff0,       0,       0,       0,       0,     0,         2}, //EON EN25QH16B
      { 0x0e,      0,     3,   8,  0x0010,  0x0020,  0x0030,       0,       0,     1,         1}, //Boya
      { 0x0f,      0,     2,  16,  0x0000,  0x0010,       0,       0,       0,     0,         1}, //Gigadevice
      { 0x10,      0,     4,   4,  0x0000,  0x0001,  0x0002,  0x0003,       1,     0,         1}, //Fudan, XTX
      { 0x11,      0,     4,   4,  0x0000,  0x0010,  0x0020,  0x0030,       0,     0,         3}, //ISSI
      { 0x12,      1,     1,   8,  0x0000,       0,       0,       0,       0,     0,         4}, //Fidelix, Zetta
      { 0x13,      0,     3,   4,  0x0040,  0x0080,  0x00c0,       0,       0,     0,         1}, //CXF
      { 0x14,      0,     2,  16,  0x0010,  0x0020,       0,       0,       0,     0,         1}, //XTX
      { 0x15,      0,     2,   4,  0x0000,  0x0001,       0,       0,       0,     0,         1}, //XTX
      { 0x16,      1,     1,   8,  0x0000,       0,       0,       0,       0,     0,         4}, //MXIC
      { 0x17,      1,     1,   1,  0x0000,       0,       0,       0,       0,     0,         4}, //MXIC
      { 0x18,      1,     2,   8,  0x0000,  0x0020,       0,       0,       0,     0,         4}, //MXIC
      { 0x19,      0,     3,  16,  0x0010,  0x0020,  0x0030,       0,       0,     1,         1}, //Boya
      { 0x1a,      0,     1,  64,  0x0000,       0,       0,       0,       0,     1,         1}, //Gigadevice
      { 0x1b,      0,     3,  64,  0x0010,  0x0020,  0x0030,       0,       0,     1,         1}, //Gigadevice
      { 0x1c,      0,     3,  16,  0x0000,  0x0004,  0x0008,       0,       0,     0,         1}, //Giantec GT25Q32A
      { 0x1d,      0,     3,  16,  0x0000,  0x0008,  0x0010,       0,       0,     0,         1}, //Giantec GT25Q64A
      { 0x1e,      0,     1,   1,  0x0000,       0,       0,       0,       0,     0,         5}, //Pm25LQ032
      
    };
    curAlg = currentAlg;
    curSettings = algSet[curAlg];
    curCommands = comPattern[curSettings.curCommand];
    if (curCommands.ERSCUR == 0) ui->toolButton_erase->setDisabled(true);
    else ui->toolButton_erase->setDisabled(false);

    if (curAlg > 0)
    {
        for (i=1; i <= curSettings.regNumber; i++)
        {
            ui->comboBox_regnum->addItem(QString::number(i),i);
        }
        regData.reserve(2048);
        regData.resize(curSettings.size * 64);
        regData.fill(char(0xff));

        QFont heFont;
        heFont = QFont("Monospace", 10);
        hexEdit = new QHexEdit(ui->frame);
        hexEdit->setGeometry(0, 0, ui->frame->width(), ui->frame->height());
        hexEdit->setData(regData);
        hexEdit->setHexCaps(true);
        hexEdit->setFont(heFont);
        hexEdit->setData(regData);
    }
}

void DialogSecurity::on_toolButton_read_clicked()
{
    int retval, i;
    if (curSettings.id > 0)
    {
        std::shared_ptr<uint8_t[]> buf(new uint8_t[curSettings.size * 64]);
        uint8_t a23a16 = 0, a15a08 = 0;
        uint16_t curRgAddr = 0;
        int stCH341 = 0;
        uint8_t curRegister = static_cast<uint8_t>(ui->comboBox_regnum->currentData().toUInt());
        curRegister--;
        stCH341 = ch341a_spi_init();
        if (stCH341 == 0)
        {
           if (curRegister == 0) curRgAddr = curSettings.rg0addr;
           if (curRegister == 1) curRgAddr = curSettings.rg1addr;
           if (curRegister == 2) curRgAddr = curSettings.rg2addr;
           if (curRegister == 3) curRgAddr = curSettings.rg3addr;
           a23a16 = static_cast<uint8_t>(curRgAddr >> 8);
           a15a08 = static_cast<uint8_t>(curRgAddr & 0x00ff);

           if (curSettings.algType == 1)
           {
           //Enter OTP mode
           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(curCommands.ENSO);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);
           }

           if (curSettings.a4byte == 1)
           {
               SPI_CONTROLLER_Chip_Select_Low();
               SPI_CONTROLLER_Write_One_Byte(0xb7); // Enter 4 byte mode
               SPI_CONTROLLER_Chip_Select_High();
               usleep(1);
           }

           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(curCommands.RDSCUR); //Reading SR command
           if (curSettings.a4byte == 1) SPI_CONTROLLER_Write_One_Byte(0x00); //A32...A24
           SPI_CONTROLLER_Write_One_Byte(a23a16); //A23...A16
           SPI_CONTROLLER_Write_One_Byte(a15a08); //A15...A08
           SPI_CONTROLLER_Write_One_Byte(0x00); //A07...A00
           if (curCommands.DUMRD == 1) SPI_CONTROLLER_Write_One_Byte(0x00);  //Dummy byte
           retval = SPI_CONTROLLER_Read_NByte(buf.get(), curSettings.size * 64, SPI_CONTROLLER_SPEED_SINGLE);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);

           if (curSettings.algType == 1)
           {
           //Exit OTP mode
           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(curCommands.EXSO);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);
           }

           if (retval)
           {
               QMessageBox::about(this, tr("Error"), tr("Error reading register!"));
               return;
           }
           for (i = 0; i < curSettings.size * 64; i++)
           {
               regData[i] = char(buf[i]);
           }
           hexEdit->setData(regData);

        }
        else QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
        ch341a_spi_shutdown();
    }
}

void DialogSecurity::on_toolButton_write_clicked()
{
    int retval, i, j = 1, k;
    if (curSettings.id > 0)
    {
        uint8_t a23a16 = 0, a15a08 = 0;
        uint16_t curRgAddr = 0;
        uint8_t  sr = 0;
        std::shared_ptr<uint8_t[]> buf(new uint8_t[curSettings.size * 64]);
        int stCH341 = 0;
        uint8_t curRegister = static_cast<uint8_t>(ui->comboBox_regnum->currentData().toUInt());
        curRegister--;
        stCH341 = ch341a_spi_init();
        if (stCH341 == 0)
        {
           if (curRegister == 0) curRgAddr = curSettings.rg0addr;
           if (curRegister == 1) curRgAddr = curSettings.rg1addr;
           if (curRegister == 2) curRgAddr = curSettings.rg2addr;
           if (curRegister == 3) curRgAddr = curSettings.rg3addr;
           a23a16 = static_cast<uint8_t>(curRgAddr >> 8);
           a15a08 = static_cast<uint8_t>(curRgAddr & 0x00ff);

           if (curSettings.size * 64 > 256) j = curSettings.size / 4;
           else j = 1;

           if (curSettings.a4byte == 1)
           {
               SPI_CONTROLLER_Chip_Select_Low();
               SPI_CONTROLLER_Write_One_Byte(0xb7); // Enter 4 byte mode
               SPI_CONTROLLER_Chip_Select_High();
               usleep(1);
           }

           regData = hexEdit->data();
           for (i = 0; i < curSettings.size * 64; i++)
           {
               buf[i] = static_cast<uint8_t>(regData[i]);
           }

           for (k = 0; k < j; k++)
           {

               if (curSettings.algType == 1)
               {
               //Enter OTP mode
               SPI_CONTROLLER_Chip_Select_Low();
               SPI_CONTROLLER_Write_One_Byte(curCommands.ENSO);
               SPI_CONTROLLER_Chip_Select_High();
               usleep(1);
               }

               SPI_CONTROLLER_Chip_Select_Low();
               SPI_CONTROLLER_Write_One_Byte(0x06);  // Write Enable
               SPI_CONTROLLER_Chip_Select_High();
               usleep(1);

               SPI_CONTROLLER_Chip_Select_Low();
               SPI_CONTROLLER_Write_One_Byte(curCommands.WRSCUR);  // Write SR command
               if (curSettings.a4byte == 1) SPI_CONTROLLER_Write_One_Byte(0x00); //A32...A24
               SPI_CONTROLLER_Write_One_Byte(a23a16); //A23...A16
               SPI_CONTROLLER_Write_One_Byte(a15a08); //A15...A08
               SPI_CONTROLLER_Write_One_Byte(0x00);   //A07...A00
               if (curSettings.size < 4) retval = SPI_CONTROLLER_Write_NByte(buf.get(), curSettings.size * 64, SPI_CONTROLLER_SPEED_SINGLE);
               else retval = SPI_CONTROLLER_Write_NByte(buf.get(), 256, SPI_CONTROLLER_SPEED_SINGLE);
               SPI_CONTROLLER_Chip_Select_High();
               usleep(1);

              if (retval)
              {
                  QMessageBox::about(this, tr("Error"), tr("Error writing register!"));
                  return;
              }
              if (curSettings.size > 3)
              {
                  for (i = 0; i < 256; i++)
                  {
                      buf[i] = buf[i + 256 * (k +1)];
                  }
              }
              a15a08++;
              for (i = 0; i < 256; i++)
              {
                  usleep(3);
                  SPI_CONTROLLER_Chip_Select_Low();  // waiting for WIP bit is set to zero
                  SPI_CONTROLLER_Write_One_Byte(0x05);
                  SPI_CONTROLLER_Read_NByte(&sr,1,SPI_CONTROLLER_SPEED_SINGLE);
                  SPI_CONTROLLER_Chip_Select_High();
                  if ((sr & 0x01) == 0) break;
              }

              if (curSettings.algType == 1)
              {
              //Exit OTP mode
              SPI_CONTROLLER_Chip_Select_Low();
              SPI_CONTROLLER_Write_One_Byte(curCommands.EXSO);
              SPI_CONTROLLER_Chip_Select_High();
              usleep(1);
              }

              SPI_CONTROLLER_Chip_Select_Low();
              SPI_CONTROLLER_Write_One_Byte(0x04);  // Write Disable
              SPI_CONTROLLER_Chip_Select_High();
              usleep(1);
          }

        }
        else QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
        ch341a_spi_shutdown();
    }
}

void DialogSecurity::on_toolButton_erase_clicked()
{
    int stCH341 = 0;
    uint8_t curRegister = static_cast<uint8_t>(ui->comboBox_regnum->currentData().toUInt());
    curRegister--;
    uint8_t a23a16 = 0, a15a08 = 0;
    uint16_t curRgAddr = 0;
    stCH341 = ch341a_spi_init();
    if (stCH341 == 0)
    {
       if (curRegister == 0) curRgAddr = curSettings.rg0addr;
       if (curRegister == 1) curRgAddr = curSettings.rg1addr;
       if (curRegister == 2) curRgAddr = curSettings.rg2addr;
       if (curRegister == 3) curRgAddr = curSettings.rg3addr;
       a23a16 = static_cast<uint8_t>(curRgAddr >> 8);
       a15a08 = static_cast<uint8_t>(curRgAddr & 0x00ff);

           if (curSettings.algType == 1)
           {
           //Enter OTP mode
           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(curCommands.ENSO);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);
           }

           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x06);  // Write Enable
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);

           if (curSettings.a4byte == 1)
           {
               SPI_CONTROLLER_Chip_Select_Low();
               SPI_CONTROLLER_Write_One_Byte(0xb7); // Enter 4 byte mode
               SPI_CONTROLLER_Chip_Select_High();
               usleep(1);
           }

           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(curCommands.ERSCUR); // Erase SR command
           if (curSettings.a4byte == 1) SPI_CONTROLLER_Write_One_Byte(0x00); //A32...A24
           SPI_CONTROLLER_Write_One_Byte(a23a16);  //A23...A16
           if (curSettings.allErase == 0)
           {
              SPI_CONTROLLER_Write_One_Byte(a15a08);
           }
           else SPI_CONTROLLER_Write_One_Byte(0x00);
           SPI_CONTROLLER_Write_One_Byte(0x00);  //A7...A0

           if (curSettings.algType == 1)
           {
           //Exit OTP mode
           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(curCommands.EXSO);
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);
           }

           SPI_CONTROLLER_Chip_Select_Low();
           SPI_CONTROLLER_Write_One_Byte(0x04);  // Write Disable
           SPI_CONTROLLER_Chip_Select_High();
           usleep(1);
    }
       else QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
       ch341a_spi_shutdown();

}

void DialogSecurity::setPath(QString lastPath)
{
    curPath = lastPath;
}

void DialogSecurity::on_toolButton_open_clicked()
{
    QByteArray buf;
    QString fileName;

        fileName = QFileDialog::getOpenFileName(this,
                                    QString(tr("Open file")),
                                    curPath,
                                    "Data Images (*.bin *.BIN *.rom *.ROM);;All files (*.*)");

    QFileInfo info(fileName);
    curPath = info.filePath();
    QFile file(fileName);
    if (info.size() > curSettings.size * 64 )
    {
      QMessageBox::about(this, tr("Error"), tr("The file size exceeds the security register size."));
      return;
    }
    if (!file.open(QIODevice::ReadOnly))
    {

        return;
    }
    buf.resize(static_cast<int>(info.size()));
    buf = file.readAll();

    regData.replace(0, static_cast<int>(info.size()), buf);
    hexEdit->setData(regData);

    file.close();
    // path must be transfered to mainwindow
}

void DialogSecurity::on_toolButton_save_clicked()
{
    QString fileName;
    fileName = QFileDialog::getSaveFileName(this,
                                QString(tr("Save file")),
                                curPath,
                                "Data Images (*.bin *.BIN);;All files (*.*)");
    if (fileName.isEmpty()) return;
    QFileInfo info(fileName);
    curPath = info.filePath();

    if (QString::compare(info.suffix(), "bin", Qt::CaseInsensitive)) fileName = fileName + ".bin";
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::about(this, tr("Error"), tr("Error saving file!"));
        return;
    }
    file.write(hexEdit->data());
    file.close();
    // path must be transfered to mainwindow
}

void DialogSecurity::closeEvent(QCloseEvent* event)
{
    emit closeRequestHasArrived();
    QWidget::closeEvent(event);
}

