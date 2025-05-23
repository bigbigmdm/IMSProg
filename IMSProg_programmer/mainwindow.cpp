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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QErrorMessage>
#include <QDragEnterEvent>
#include <QtGui>
#include <QFileInfo>
#include "qhexedit.h"
#include "dialogsp.h"
#include "dialogrp.h"
#include "dialogsetaddr.h"
#include "dialogsecurity.h"
#include "hexutility.h"
#include <stddef.h>
#include <stdint.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
 ui->setupUi(this);

 max_rec = 0;
 isHalted = false;
 lastDirectory = QDir::homePath(); //"/home/";
 grnKeyStyle = "QPushButton{color:#fff;background-color: rgb(120, 183, 140);border-radius: 20px;border: 2px solid #094065;border-radius:8px;font-weight:600;}QPushButton::pressed{background-color: rgb(115, 210, 22);}";
 redKeyStyle = "QPushButton{color:#fff;background-color:#f66;border-radius: 20px;border: 2px solid #094065;border-radius:8px;font-weight:600;}";
 timer = new QTimer();
 connect(timer, SIGNAL(timeout()), this, SLOT(slotTimerAlarm()));
 timer->start(2000);

 ui->statusMessage->setText("");
 ui->actionStop->setDisabled(true);
 ui->statusBar->addPermanentWidget(ui->statusMessage,1);
 ui->statusBar->addPermanentWidget(ui->lStatus,0);
 ui->statusBar->addPermanentWidget(ui->eStatus,0);
 ui->statusBar->addPermanentWidget(ui->jLabel,0);
 ui->statusBar->addPermanentWidget(ui->jedecEdit,0);
 ui->statusBar->addPermanentWidget(ui->cLabel,0);
 ui->statusBar->addPermanentWidget(ui->crcEdit,0);
 ui->progressBar->setValue(0);
 ui->comboBox_name->addItems({""});
 ui->comboBox_man->addItems({""});

 ui->comboBox_vcc->addItem(" ", 0);
 ui->comboBox_vcc->addItem("3.3 V", 1);
 ui->comboBox_vcc->addItem("1.8 V", 2);
 ui->comboBox_vcc->addItem("5.0 V", 3);

 ui->comboBox_type->addItem("SPI_FLASH", 0);
 ui->comboBox_type->addItem("24_EEPROM", 1);
 ui->comboBox_type->addItem("93_EEPROM", 2);
 ui->comboBox_type->addItem("25_EEPROM", 3);
 ui->comboBox_type->addItem("95_EEPROM", 4);
 ui->comboBox_type->addItem("45_EEPROM", 5);

 ui->comboBox_addr4bit->addItem("No", 0);
 ui->comboBox_addr4bit->addItem("Yes", 0x01);
 ui->comboBox_addr4bit->addItem("Winbond", 0x11);
 ui->comboBox_addr4bit->addItem("Spansion", 0x21);

 ui->comboBox_page->addItem(" ", 0);
 ui->comboBox_page->addItem("1", 1);
 ui->comboBox_page->addItem("2", 2);
 ui->comboBox_page->addItem("4", 4);
 ui->comboBox_page->addItem("8", 8);
 ui->comboBox_page->addItem("16", 16);
 ui->comboBox_page->addItem("32", 32);
 ui->comboBox_page->addItem("64", 64);
 ui->comboBox_page->addItem("128", 128);
 ui->comboBox_page->addItem("256", 256);
 ui->comboBox_page->addItem("264", 264);
 ui->comboBox_page->addItem("512", 512);
 ui->comboBox_page->addItem("528", 528);

 ui->comboBox_block->addItem(" ", 0);
 ui->comboBox_block->addItem("64 K", 64 * 1024);

 ui->comboBox_i2cSpeed->addItem("20 kHz",  0);
 ui->comboBox_i2cSpeed->addItem("100 kHz", 1);
 ui->comboBox_i2cSpeed->addItem("400 kHz", 2);
 ui->comboBox_i2cSpeed->addItem("750 kHz", 3);
 ui->comboBox_i2cSpeed->setCurrentIndex(2);
 currentI2CBusSpeed = 2;

 currentChipSize = 0;
 currentNumBlocks = 0;
 currentBlockSize = 0;
 currentPageSize = 0;
 currentAlgorithm = 0;
 currentChipType = 0;
 blockStartAddr = 0;
 blockLen = 0;
 currentAddr4bit = 0;
 filled = 0;
 numberOfReads = 0;
 cmdStarted = false;
 // connect and status check
 statusCH341 = ch341a_spi_init();
 ch341StatusFlashing();
 chipData.reserve(256 * 1024 *1024 + 2048);
 chipData.resize(256);
 chipData.fill(char(0xff));
 oldChipData.reserve(256 * 1024 *1024 + 2048);
 oldChipData.resize(256);
 oldChipData.fill(char(0xff));
 ch341a_spi_shutdown();
 QFont heFont;
 heFont = QFont("Monospace", 10);
 hexEdit = new QHexEdit(ui->frame);
 hexEdit->setGeometry(0,0,ui->frame->width(),ui->frame->height());
 hexEdit->setData(chipData);
 hexEdit->setHexCaps(true);
 hexEdit->setFont(heFont);
 QStringList commandLineParams = QCoreApplication::arguments();
 QString commandLineFileName ="";
 if (commandLineParams.count() > 1)
   {
        commandLineFileName = commandLineParams[1];
        QFileInfo commandLine(commandLineFileName);
        if ((commandLine.exists()) && !(QString::compare(commandLine.suffix(), "bin", Qt::CaseInsensitive)))
        {
            lastDirectory = commandLineFileName;
            cmdStarted = true;
        }
   }
 progInit();
 if (cmdStarted) on_actionOpen_triggered();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
  //Reading data from chip  
  newFileName = ui->comboBox_name->currentText();
  int res = 0;
  uint32_t numBlocks, step;
  statusCH341 = ch341a_init(currentChipType, currentI2CBusSpeed);
  if (statusCH341 == 0)
  {
    ui->crcEdit->setText("");
    if (((currentNumBlocks > 0) && (currentBlockSize >0) && (currentChipType == 0)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 1)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 2)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 3)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 4)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 5)))
    {
       doNotDisturb();
       ch341StatusFlashing();
       if (numberOfReads > 0)
       {
           oldChipData = chipData;
           oldFileName = ui->comboBox_name->currentText();
       }
       uint32_t addr = 0;
       uint32_t curBlock = 0;
       uint32_t j, k;
       switch (currentChipType)
          {
          case 0:             //SPI
             step = currentBlockSize;
             numBlocks = currentNumBlocks;
          break;
          case 1:             //I2C
             step = 128;
             numBlocks = currentChipSize / step;
          break;
          case 2:             //MicroWire
          case 3:             //25xxx
          case 4:             //95xxx
          case 5:             //45xx
             step = currentPageSize;
             numBlocks = currentChipSize / step;
          break;
          default:
          return;
          }
       //progerssbar settings
       ui->progressBar->setRange(0, static_cast<int>(numBlocks));
       ui->progressBar->setValue(0);
       std::shared_ptr<uint8_t[]> buf(new uint8_t[step]);
       ui->pushButton->setStyleSheet(redKeyStyle);
       ui->statusMessage->setText(tr("Reading data from ") + ui->comboBox_name->currentText());
       for (k = 0; k < numBlocks; k++)
       {
           switch (currentChipType)
              {
              case 0:            //SPI
                 res = snor_read_param(buf.get(), curBlock * step, step, step, currentAddr4bit);
              break;
              case 1:            //I2C
               res = ch341readEEPROM_param(buf.get(), curBlock * step, step, currentChipSize, currentPageSize, currentAlgorithm);//currentAlgorithm);
               if (res==0) res = 1;
              break;
              case 2:
                 //MicroWire
               res = Read_EEPROM_3wire_param(buf.get(), static_cast<int>(curBlock * step), static_cast<int>(step), static_cast<int>(currentChipSize), currentAlgorithm);
               if (res==0) res = 1;
              break;
              case 3:
                 //25xxx
              case 4:
                 //95xxx
                 res = s95_read_param(buf.get(),curBlock * step, step, step, currentAlgorithm);
              break;
              case 5:
                 //45xx
                 res = at45_read_param(buf.get(),curBlock * step, step, step, currentAlgorithm);
              break;
              default:
                 //Unsupport
                 QMessageBox::about(this, tr("Error"), tr("Unsupported chip type!"));
                 doNotDisturbCancel();
                 ch341a_spi_shutdown();
              return;
              }
          // if res=-1 - error, stop
          if (statusCH341 != 0)
            {
                QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
                doNotDisturbCancel();
                break;
            }
          if (res <= 0)
            {
               QMessageBox::about(this, tr("Error"), tr("Error reading block ") + QString::number(curBlock));
               ch341a_spi_shutdown();
               ui->pushButton->setStyleSheet(grnKeyStyle);
               doNotDisturbCancel();
               return;
            }
         for (j = 0; j < step; j++)
            {
                  chipData[addr + j] = char(buf[addr + j - k * step]);
            }
          addr = addr + step;
          if (curBlock * step < 0x500) hexEdit->setData(chipData); //show buffer in hehedit while chip data is visible
          curBlock++;
          qApp->processEvents();
          ui->progressBar->setValue(static_cast<int>(curBlock));
          if (isHalted)
          {
              isHalted = false;
              ch341a_spi_shutdown();
              doNotDisturbCancel();
              return;
          }
       }
    }
    else
    {
       //Not correct Number found size of blocks
       if (currentChipType == 0) QMessageBox::about(this, tr("Error"), tr("Before reading from chip please press 'Detect' button."));
       if (currentChipType  >0 ) QMessageBox::about(this, tr("Error"), tr("Please select the chip parameters - manufacture and chip name"));
    }
    hexEdit->setData(chipData);
    ui->statusMessage->setText("");
    ui->progressBar->setValue(0);
    ui->pushButton->setStyleSheet(grnKeyStyle);
    ui->crcEdit->setText(getCRC32(chipData));
    newFileName = ui->comboBox_name->currentText();
  }
  else
  {
      ch341StatusFlashing();
      QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
  }
  ch341a_spi_shutdown();
  doNotDisturbCancel();
  filled = 0;
  numberOfReads++;
}

void MainWindow::on_pushButton_2_clicked()
{
    timer->stop();
    //searching the connected chip in database
    u8 sr;
    statusCH341 = ch341a_spi_init();
    ch341StatusFlashing();
    if (statusCH341 != 0)
      {
        QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
        timer->start();
        return;
      }

    ui->pushButton_2->setStyleSheet(redKeyStyle);
    ui->crcEdit->setText("");
    int i, index;
    // print JEDEC info
    unsigned char bufid[5]={0xff,0xff,0xff,0xff,0xff};
    snor_read_devid(bufid, 5);
    if ((bufid[0] == 0xff) && (bufid[1] == 0xff) && (bufid[2] == 0xff) && (currentChipType != 5))
    {
        QMessageBox::about(this, tr("Error"), tr("The chip is not connect or missing!"));
        ui->pushButton_2->setStyleSheet(grnKeyStyle);
        ch341a_spi_shutdown();
        timer->start();
        return;
    }
    ui->jedecEdit->setText(bytePrint(bufid[0]) + " " + bytePrint(bufid[1]) + " " + bytePrint(bufid[2]));
    for (i = 0; i< max_rec; i++)
    {
        if ((bufid[0] == chips[i].chipJedecIDMan) && (bufid[1] == chips[i].chipJedecIDDev) && (bufid[2] == chips[i].chipJedecIDCap))
        {            
            index = ui->comboBox_type->findText(chips[i].chipTypeTxt);
            if ( index != -1 )
            { // -1 for not found
               ui->comboBox_type->setCurrentIndex(index);
            }
            index = ui->comboBox_man->findText(chips[i].chipManuf);
            if ( index != -1 )
            { // -1 for not found
                ui->comboBox_man->setCurrentIndex(index);
            }
            index = ui->comboBox_name->findText(chips[i].chipName);
            if ( index != -1 )
            { // -1 for not found
                ui->comboBox_name->setCurrentIndex(index);
            }
            index = ui->comboBox_size->findData(chips[i].chipSize);
            if ( index != -1 )
            { // -1 for not found
               ui->comboBox_size->setCurrentIndex(index);
            }
            index = ui->comboBox_page->findData(chips[i].sectorSize);
            if ( index != -1 )
            { // -1 for not found
               ui->comboBox_page->setCurrentIndex(index);
            }
            index = ui->comboBox_block->findData(chips[i].blockSize);
            if ( index != -1 )
            { // -1 for not found
               ui->comboBox_block->setCurrentIndex(index);
            }
            index = ui->comboBox_addr4bit->findData(chips[i].addr4bit);
            if ( index != -1 )
            { // -1 for not found
               ui->comboBox_addr4bit->setCurrentIndex(index);
            }
            index = ui->comboBox_vcc->findText(chips[i].chipVCC);
            if ( index != -1 )
            { // -1 for not found
               ui->comboBox_vcc->setCurrentIndex(index);
            }

            ui->pushButton_2->setStyleSheet(grnKeyStyle);
            break;
        }
    }

    // 45xxx without JEDEC info
    if ((currentChipType == 5) && (bufid[0] == 0xff) && (bufid[1] == 0xff) && (bufid[2] == 0xff) )
    {
        //calculating capacity
        int capacityIndex = 0;
        if (!at45_read_sr(&sr))
        {
            if ((sr & 0xC0) == 0x80) //45xx chip found
            {
                sr = sr & 0x38;
                switch (sr)
                   {
                   case 0x08:
                      //132K
                      capacityIndex = 1;
                   break;
                   case 0x10:
                      //264K
                      capacityIndex = 2;
                   break;
                   case 0x18:
                      //528K
                      capacityIndex = 3;
                   break;
                   case 0x20:
                      //1056K
                      capacityIndex = 4;
                   break;
                   case 0x28:
                      //2112K
                      capacityIndex = 5;
                   break;
                   case 30:
                      //4224K
                      capacityIndex = 6;
                   break;
                   }
                ui->comboBox_size->setCurrentIndex(capacityIndex);
                for (i = 0; i< max_rec; i++)
                {
                    if ((chips[i].chipTypeHex == 0x05) && (chips[i].chipSize == ui->comboBox_size->currentData().toUInt()))
                    {
                       if (!QString::compare(chips[i].chipManuf, "ATMEL", Qt::CaseInsensitive))
                       {
                           index = ui->comboBox_type->findText(chips[i].chipTypeTxt);
                           if ( index != -1 )
                           { // -1 for not found
                              ui->comboBox_type->setCurrentIndex(index);
                           }
                           index = ui->comboBox_man->findText(chips[i].chipManuf);
                           if ( index != -1 )
                           { // -1 for not found
                               ui->comboBox_man->setCurrentIndex(index);
                           }
                           index = ui->comboBox_name->findText(chips[i].chipName);
                           if ( index != -1 )
                           { // -1 for not found
                               ui->comboBox_name->setCurrentIndex(index);
                           }
                           index = ui->comboBox_size->findData(chips[i].chipSize);
                           if ( index != -1 )
                           { // -1 for not found
                              ui->comboBox_size->setCurrentIndex(index);
                           }
                           index = ui->comboBox_page->findData(chips[i].sectorSize);
                           if ( index != -1 )
                           { // -1 for not found
                              ui->comboBox_page->setCurrentIndex(index);
                           }
                           index = ui->comboBox_block->findData(chips[i].blockSize);
                           if ( index != -1 )
                           { // -1 for not found
                              ui->comboBox_block->setCurrentIndex(index);
                           }
                           index = ui->comboBox_addr4bit->findData(chips[i].addr4bit);
                           if ( index != -1 )
                           { // -1 for not found
                              ui->comboBox_addr4bit->setCurrentIndex(index);
                           }
                           index = ui->comboBox_vcc->findText(chips[i].chipVCC);
                           if ( index != -1 )
                           { // -1 for not found
                              ui->comboBox_vcc->setCurrentIndex(index);
                           }

                           ui->pushButton_2->setStyleSheet(grnKeyStyle);
                           break;
                       }
                    }
                }
            }
        }

    }

    if ((currentChipType == 5) && (ui->comboBox_size->currentIndex() > 0))
    {
        //calculate buffer size
        if (!at45_read_sr(&sr))
        {
            if ((sr & 0x01) == 1)
            {
                if ( ui->comboBox_page->currentData() == 264)
                {
                    index = ui->comboBox_page->findData(256);
                    if ( index != -1 )
                    { // -1 for not found
                       ui->comboBox_page->setCurrentIndex(index);
                    }
                }
                if ( ui->comboBox_page->currentData() == 528)
                {
                    index = ui->comboBox_page->findData(512);
                    if ( index != -1 )
                    { // -1 for not found
                       ui->comboBox_page->setCurrentIndex(index);
                    }
                }
            }
        }
    }

    ui->pushButton_2->setStyleSheet(grnKeyStyle);
    ui->crcEdit->setText(getCRC32(chipData));
    ch341a_spi_shutdown();
    timer->start();
}

void MainWindow::on_comboBox_size_currentIndexChanged(int index)
{
    currentChipSize = ui->comboBox_size->currentData().toUInt();
    currentBlockSize = ui->comboBox_block->currentData().toUInt();
    currentPageSize = ui->comboBox_page->currentData().toUInt();
    currentAddr4bit = ui->comboBox_addr4bit->currentData().toUInt();
    if ((currentChipSize !=0) && (currentBlockSize!=0) && (currentChipType == 0))
    {
        currentNumBlocks = currentChipSize / currentBlockSize;
        preparingToCompare(1);
        numberOfReads = 0;
        chipData.resize(static_cast<int>(currentChipSize));
        chipData.fill(char(0xff));
        filled = 1;
        hexEdit->setData(chipData);
    }
    if ((currentChipSize !=0) && (currentPageSize!=0)  && (currentChipType > 0))
    {
    currentNumBlocks = currentChipSize / currentPageSize;
    preparingToCompare(1);
    numberOfReads = 0;
    chipData.resize(static_cast<int>(currentChipSize));
    chipData.fill(char(0xff));
    filled = 1;
    hexEdit->setData(chipData);
    }
    index = index + 0;
}

void MainWindow::on_comboBox_page_currentIndexChanged(int index)
{
    currentChipSize = ui->comboBox_size->currentData().toUInt();
    currentBlockSize = ui->comboBox_block->currentData().toUInt();
    currentPageSize = ui->comboBox_page->currentData().toUInt();
    currentAddr4bit = ui->comboBox_addr4bit->currentData().toUInt();
    if ((currentChipSize !=0) && (currentBlockSize!=0) && (currentChipType ==0))
    {
        currentNumBlocks = currentChipSize / currentBlockSize;
    }
    if ((currentChipSize !=0) && (currentPageSize!=0)  && (currentChipType > 0))
    {
    currentNumBlocks = currentChipSize / currentPageSize;
    }
    index = index + 0;
}

void MainWindow::on_actionDetect_triggered()
{
   MainWindow::on_pushButton_2_clicked();
}

void MainWindow::on_actionSave_triggered()
{

    lastDirectory.replace(".cap", ".bin");
    lastDirectory.replace(".CAP", ".bin");
    lastDirectory.replace(".hex", ".bin");
    lastDirectory.replace(".HEX", ".bin");

    ui->statusMessage->setText(tr("Saving file"));
    fileName = QFileDialog::getSaveFileName(this,
                                QString(tr("Save file")),
                                lastDirectory,
                                "Data Images (*.bin *.BIN);;All files (*.*)");
    if (fileName.isEmpty()) return;
    QFileInfo info(fileName);
    lastDirectory = info.filePath();

    if (QString::compare(info.suffix(), "bin", Qt::CaseInsensitive)) fileName = fileName + ".bin";
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::about(this, tr("Error"), tr("Error saving file!"));
        return;
    }
    file.write(hexEdit->data());
    file.close();
    ui->statusMessage->setText("");
}

void MainWindow::on_actionErase_triggered()
{
    //statusCH341 = ch341a_spi_init();
    int ret;
    uint32_t curBlock, numBlocks, step;
    statusCH341 = ch341a_init(currentChipType, currentI2CBusSpeed);
    ch341StatusFlashing();
    if (statusCH341 != 0)
      {
        QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
        return;
      }
    ui->statusMessage->setText(tr("Erasing the ") + ui->comboBox_name->currentText());
    ui->checkBox->setStyleSheet("QCheckBox{font-weight:600;}");
    ui->centralWidget->repaint();
    ui->progressBar->setRange(0, 100);
    doNotDisturb();
    if (currentChipType == 0)
    {
       if (currentNumBlocks > 0)
       {
           ui->progressBar->setRange(0, static_cast<int>(currentNumBlocks));
           for (uint32_t curBlock = 0; curBlock < currentNumBlocks; curBlock++)
           {
               //config_stream(2);
               ret = snor_block_erase( curBlock,  currentBlockSize, static_cast<u8>(currentAddr4bit));
               if (ret != 0)
                 {
                   QMessageBox::about(this, tr("Error"), tr("Error erasing sector ") + QString::number(curBlock));
                   ch341a_spi_shutdown();
                   doNotDisturbCancel();
                   return;
                 }
               qApp->processEvents();
               ui->progressBar->setValue( static_cast<int>(curBlock));
               if (isHalted)
               {
                   isHalted = false;
                   ch341a_spi_shutdown();
                   doNotDisturbCancel();
                   return;
               }
           }
       }

    }
    if ((currentChipType == 4) || ((currentChipType == 3) && ((currentAlgorithm & 0x20) == 0)))
    {
        uint32_t k;
        int res = 0;
        step = currentPageSize;
        numBlocks = currentChipSize / step;
        std::shared_ptr<uint8_t[]> buf(new uint8_t[step]);
        config_stream(2);
        if (isHalted)
        {
            isHalted = false;
            ch341a_spi_shutdown();
            doNotDisturbCancel();
            return;
        }
        ui->progressBar->setRange(0, static_cast<int>(numBlocks));
        for (k = 0; k < step; k++)
        {
            buf[k] = 0xff;
        }
        for (curBlock = 0; curBlock < currentNumBlocks; curBlock++)
        {
            //res = ch341writeEEPROM_param(buf, curBlock * 128, 128, currentPageSize, currentAlgorithm);
            res =  s95_write_param(buf.get(), curBlock * step, step, step, currentAlgorithm);
            qApp->processEvents();
            ui->progressBar->setValue( static_cast<int>(curBlock));
            if (res <= 0)
              {
                QMessageBox::about(this, tr("Error"), tr("Error erasing sector ") + QString::number(curBlock));
                ch341a_spi_shutdown();
                doNotDisturbCancel();
                return;
              }
        }
    }
    if ((currentChipType == 3) && ((currentAlgorithm & 0x20) > 0))
    {
        ui->progressBar->setValue(50);
        s95_full_erase();
        sleep(1);
    }
    if (currentChipType == 2)
    {
        config_stream(1);
        mw_gpio_init();
        ui->progressBar->setValue(50);
        Erase_EEPROM_3wire_param(currentAlgorithm);
        sleep(1);
    }
    if (currentChipType == 1)
    {
        uint32_t curBlock = 0;
        uint32_t k;
        int res = 0;
        step = 128;
        numBlocks = currentChipSize / step;
        std::shared_ptr<uint8_t[]> buf(new uint8_t[step]);
        config_stream(2);
        if (isHalted)
        {
            isHalted = false;
            ch341a_spi_shutdown();
            doNotDisturbCancel();
            return;
        }
        ui->progressBar->setRange(0, static_cast<int>(numBlocks));
        for (k = 0; k < step; k++)
        {
            buf[k] = 0xff;
        }
        for (curBlock = 0; curBlock < numBlocks; curBlock++)
        {
            res = ch341writeEEPROM_param(buf.get(), curBlock * 128, 128, currentPageSize, currentAlgorithm);
            if (res==0) res = 1;
            qApp->processEvents();
            ui->progressBar->setValue( static_cast<int>(curBlock));
            if (res <= 0)
              {
                QMessageBox::about(this, tr("Error"), tr("Error erasing sector ") + QString::number(curBlock));
                ch341a_spi_shutdown();
                doNotDisturbCancel();
                return;
              }
        }

    }
    if ((currentChipType == 5) && ((currentAlgorithm & 0x10) > 0))
    {
        ui->progressBar->setValue(50);
        at45_full_erase();
        sleep(1);
    }
    if ((currentChipType == 5) && ((currentAlgorithm & 0x10) == 0))
    {
        numBlocks = currentChipSize / currentPageSize / 8;
        ui->progressBar->setRange(0, static_cast<int>(numBlocks));
        uint32_t curBlock = 0;
        for (curBlock = 0; curBlock < numBlocks; curBlock++)
        {
            at45_sector_erase(curBlock,  currentPageSize);
             ui->progressBar->setValue(static_cast<int>(curBlock));
        }
    }
    doNotDisturbCancel();
    ui->checkBox->setStyleSheet("");
    ui->statusMessage->setText("");
    ui->progressBar->setValue(0);
    ui->centralWidget->repaint();
    ch341a_spi_shutdown();
}

void MainWindow::on_actionUndo_triggered()
{
    hexEdit->undo();
}

void MainWindow::on_actionRedo_triggered()
{
    hexEdit->redo();
}

void MainWindow::on_actionOpen_triggered()
{    
    QByteArray buf;
    ui->statusMessage->setText(tr("Opening file"));
    if (numberOfReads == 0) oldFileName = fileName;
    else oldFileName = ui->comboBox_name->currentText();
    if (!cmdStarted)
    {
        fileName = QFileDialog::getOpenFileName(this,
                                    QString(tr("Open file")),
                                    lastDirectory,
                                    "Data Images (*.bin *.BIN *.rom *.ROM);;All files (*.*)");
    }
   else fileName = lastDirectory;
   cmdStarted = false;

    QFileInfo info(fileName);
    lastDirectory = info.filePath();
    // if ChipSze = 0 (Chip is not selected) IMSProg using at hexeditor only. chipsize -> hexedit.data
    // if ChipSize < FileSize - showing error message
    // if Filesize <= ChipSize - filling fileArray to hexedit.Data, the end of the array chipData remains filled 0xff
    QFile file(fileName);
    ui->statusMessage->setText("");
    if ((info.size() > currentChipSize) && (currentChipSize != 0))
    {
      QMessageBox::about(this, tr("Error"), tr("The file size exceeds the chip size. Please select another chip or file or use `Save part` to split the file."));
      return;
    }
    if (!file.open(QIODevice::ReadOnly))
    {

        return;
    }
    ui->statusMessage->setText(tr("Current file: ") + info.fileName());
    preparingToCompare(0);
    filled = 0;
    buf.resize(static_cast<int>(info.size()));
    buf = file.readAll();
    if (currentChipSize == 0)
    {
        chipData.resize(static_cast<int>(info.size()));
    }

    chipData.replace(0, static_cast<int>(info.size()), buf);
    hexEdit->setData(chipData);

    file.close();

    ui->crcEdit->setText(getCRC32(chipData));
}

void MainWindow::on_actionExtract_from_ASUS_CAP_triggered()
{
    QByteArray buf;
    ui->statusMessage->setText(tr("Opening file"));
    fileName = QFileDialog::getOpenFileName(this,
                                QString(tr("Open file")),
                                lastDirectory,
                                "ASUS Data Images (*.cap *.CAP);;All files (*.*)");
    QFileInfo info(fileName);
    ui->statusMessage->setText("");
    lastDirectory = info.filePath();
    if ((info.size() - 0x800 > currentChipSize) && (currentChipSize != 0))
    {
      QMessageBox::about(this, tr("Error"), tr("The file size exceeds the chip size. Please select another chip or file or use `Save part` to split the file."));
      return;
    }
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {

        return;
    }
    ui->statusMessage->setText(tr("Current file: ") + info.fileName());
    buf.resize(int(info.size()));
    buf = file.readAll();
    file.close();
    fileName.clear();
    buf.remove(0,0x800);
    if (currentChipSize == 0)
    {
        chipData.resize(static_cast<int>(info.size()) - 0x800);
    }
    chipData.replace(0, static_cast<int>(info.size()) - 0x800, buf);
    hexEdit->setData(chipData);
    ui->crcEdit->setText(getCRC32(chipData));
}

void MainWindow::on_actionWrite_triggered()
{
    //Writting data to chip
    int res = 0;
    uint32_t numBlocks, step;
    statusCH341 = ch341a_init(currentChipType, currentI2CBusSpeed);
    if (statusCH341 == 0)
    {
    if (((currentNumBlocks > 0) && (currentBlockSize >0) && (currentChipType == 0)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 1)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 2)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 3)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 4)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 5)))
        {
        doNotDisturb();
        switch (currentChipType)
                      {
                      case 0:                 //SPI
                         step = currentBlockSize;
                         numBlocks = currentNumBlocks;
                      break;
                      case 1:                 //I2C
                         step = 128;
                         numBlocks = currentChipSize / step;
                      break;
                      case 2:                 //MicroWire
                      case 3:                 //25xxx
                      case 4:                 //95xxx
                      case 5:                 //45xx
                         step = currentPageSize;
                         numBlocks = currentChipSize / step;
                      break;
                      default:
                      return;
                      }
    ch341StatusFlashing();
    uint32_t addr = 0;
    uint32_t curBlock = 0;    
    uint32_t j, k;
    ui->statusMessage->setText(tr("Writing data to ") + ui->comboBox_name->currentText());
    //progerssbar settings
    ui->progressBar->setRange(0, static_cast<int>(numBlocks));
    ui->checkBox_2->setStyleSheet("QCheckBox{font-weight:800;}");
    chipData = hexEdit->data();
    std::shared_ptr<uint8_t[]> buf(new uint8_t[step]);
    for (k = 0; k < numBlocks; k++)
      {

         for (j = 0; j < step; j++)
            {
               buf[addr + j - k * step] =  static_cast<uint8_t>(chipData[addr + j]) ;
            }
         switch (currentChipType)
                       {
                       case 0:                           //SPI
                          res =  snor_write_param(buf.get(), addr, step, step, currentAddr4bit);
                       break;
                       case 1:                           //I2C
                          res = ch341writeEEPROM_param(buf.get(), curBlock * 128, 128, currentPageSize, currentAlgorithm);
                          if (res==0) res = 1;
                       break;
                       case 2:                           //MicroWire
                          res = Write_EEPROM_3wire_param(buf.get(), static_cast<int>(curBlock * step), static_cast<int>(step), static_cast<int>(currentChipSize), currentAlgorithm);
                          if (res==0) res = 1;
                       break;
                       case 3:                           //25xxx
                       case 4:                           //M95xx
                          res =  s95_write_param(buf.get(), addr, step, step, currentAlgorithm);
                       break;
                       case 5:
                          //AT45DBxx
                          res =  at45_write_param(buf.get(), addr, step, step, currentAlgorithm);
                       break;
                       default:
                          //Unsupport
                          QMessageBox::about(this, tr("Error"), tr("Unsupported chip type!"));
                          doNotDisturbCancel();
                          ch341a_spi_shutdown();
                          ui->checkBox_2->setStyleSheet("");
                       return;
                       }
         // if res=-1 - error, stop
         if (statusCH341 != 0)
           {
             QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
             doNotDisturbCancel();
             ch341a_spi_shutdown();
             break;
           }         
         if (res <= 0)
           {
             QMessageBox::about(this, tr("Error"), tr("Error writing sector ") + QString::number(curBlock));
             doNotDisturbCancel();
             ch341a_spi_shutdown();
             return;
           }
         addr = addr + step;
         curBlock++;
         qApp->processEvents();
         if (isHalted)
         {
             isHalted = false;
             ch341a_spi_shutdown();
             doNotDisturbCancel();
             return;
         }
         ui->progressBar->setValue( static_cast<int>(curBlock));
      }
    }
    else
    {
    //Not correct Number fnd size of blocks
     QMessageBox::about(this, tr("Error"), tr("Before reading from chip please press 'Detect' button."));
    }
    doNotDisturbCancel();
    ui->progressBar->setValue(0);
    ui->checkBox_2->setStyleSheet("");
    ui->statusMessage->setText("");    
    }
    else
    {
        ch341StatusFlashing();
        QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
    }
    ch341a_spi_shutdown();
}

void MainWindow::on_actionRead_triggered()
{
    MainWindow::on_pushButton_clicked();
}

void MainWindow::on_actionExit_triggered()
{
    ch341a_spi_shutdown();
    MainWindow::close();
}

void MainWindow::on_comboBox_man_currentIndexChanged(int index)
{
    int i, index2;
    QString txt="";
    if (max_rec > 0)
    {
       txt = ui->comboBox_man->currentText().toUtf8();
       ui->comboBox_name->clear();
       ui->comboBox_name->addItem("");
       for (i = 0; i<max_rec; i++)
       {
           //replacing items to combobox chip Name
           if (txt.compare(chips[i].chipManuf)==0 && (currentChipType == chips[i].chipTypeHex))
           {
           index2 = ui->comboBox_name->findText(chips[i].chipName);
                    if ( index2 == -1 ) ui->comboBox_name->addItem(chips[i].chipName);
           }
       }
        ui->comboBox_name->setCurrentIndex(0);
        ui->comboBox_vcc->setCurrentIndex(0);
        ui->comboBox_page->setCurrentIndex(0);
        ui->comboBox_block->setCurrentIndex(0);
        ui->comboBox_size->setCurrentIndex(0);
        ui->comboBox_addr4bit->setCurrentIndex(0);
        ui->statusMessage->setText("");
   }
 index = index + 0;
}

void MainWindow::on_comboBox_name_currentIndexChanged(const QString &arg1)
{
    int i, index;
    oldFileName = fileName;
    QString manName = ui->comboBox_man->currentText();
    if (arg1.compare("") !=0)
    {

       for (i = 0; i < max_rec; i++)
       {
           if ((manName.compare(chips[i].chipManuf)==0) && (arg1.compare(chips[i].chipName)==0))
           {
               index = ui->comboBox_size->findData(chips[i].chipSize);
               if ( index != -1 )
               { // -1 for not found
                  ui->comboBox_size->setCurrentIndex(index);
               }
               index = ui->comboBox_page->findData(chips[i].sectorSize);
               if ( index != -1 )
               { // -1 for not found
                  ui->comboBox_page->setCurrentIndex(index);
               }
               index = ui->comboBox_block->findData(chips[i].blockSize);
               if ( index != -1 )
               { // -1 for not found
                  ui->comboBox_block->setCurrentIndex(index);
               }
               index = ui->comboBox_addr4bit->findData(chips[i].addr4bit);
               if ( index != -1 )
               { // -1 for not found
                  ui->comboBox_addr4bit->setCurrentIndex(index);
               }
               index = ui->comboBox_vcc->findText(chips[i].chipVCC);
               if ( index != -1 )
               { // -1 for not found
                  ui->comboBox_vcc->setCurrentIndex(index);
               }
               currentAlgorithm = chips[i].algorithmCode;
           }
       }
       currentChipSize = ui->comboBox_size->currentData().toUInt();
       currentBlockSize = ui->comboBox_block->currentData().toUInt();
       currentPageSize = ui->comboBox_page->currentData().toUInt();
       currentAddr4bit = ui->comboBox_addr4bit->currentData().toUInt();
       preparingToCompare(1);

       if ((currentChipSize !=0) && (currentBlockSize!=0) && (currentChipType == 0))
       {
           currentNumBlocks = currentChipSize / currentBlockSize;
           chipData.resize(static_cast<int>(currentChipSize));
           chipData.fill(char(0xff));
           filled = 1;
           hexEdit->setData(chipData);
       }
       if ((currentChipSize !=0) && (currentPageSize!=0)  && (currentChipType > 0))
       {
           currentNumBlocks = currentChipSize / currentPageSize;
           chipData.resize(static_cast<int>(currentChipSize));
           chipData.fill(char(0xff));
           filled = 1;
           hexEdit->setData(chipData);
       }

    }
}

void MainWindow::on_actionVerify_triggered()
{
    //Reading and veryfying data from chip
    int res = 0;
    uint32_t step, numBlocks;
    statusCH341 = ch341a_init(currentChipType, currentI2CBusSpeed);
    if (statusCH341 == 0)
    {
       if (((currentNumBlocks > 0) && (currentBlockSize >0) && (currentChipType == 0)) ||
            ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 1)) ||
            ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 2)) ||
            ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 3)) ||
            ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 4)) ||
            ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 5)))
           {
               ui->crcEdit->setText("");
               doNotDisturb();
               switch (currentChipType)
                             {
                             case 0:                 //SPI
                                step = currentBlockSize;
                                numBlocks = currentNumBlocks;
                             break;
                             case 1:                 //I2C
                                step = 128;
                                numBlocks = currentChipSize / step;
                             break;
                             case 2:                 //MicroWire
                             case 3:                 //25xxx
                             case 4:                 //95xxx
                             case 5:                 //45xx
                                step = currentPageSize;
                                numBlocks = currentChipSize / step;
                             break;
                             default:
                             return;
                             }
               ch341StatusFlashing();
               uint32_t addr = 0;
               uint32_t curBlock = 0;
               uint32_t j, k;               
               //progerssbar settings
               ui->progressBar->setRange(0, static_cast<int>(numBlocks));
               ui->progressBar->setValue(0);
               std::shared_ptr<uint8_t[]> buf(new uint8_t[step]);
               chipData = hexEdit->data();
               ui->checkBox_3->setStyleSheet("QCheckBox{font-weight:800;}");
               ui->statusMessage->setText(tr("Veryfing data from ") + ui->comboBox_name->currentText());
               for (k = 0; k < numBlocks; k++)
               {
                   switch (currentChipType)
                      {
                      case 0:
                         //SPI
                         res = snor_read_param(buf.get(),curBlock * step, step, step, currentAddr4bit);
                      break;
                      case 1:
                         //I2C
                       res = ch341readEEPROM_param(buf.get(), curBlock * step, step, currentChipSize, currentPageSize, currentAlgorithm);//currentAlgorithm);
                       if (res==0) res = 1;
                      break;
                      case 2:
                         //MicroWire
                       res = Read_EEPROM_3wire_param(buf.get(), static_cast<int>(curBlock * step), static_cast<int>(step), static_cast<int>(currentChipSize), currentAlgorithm);
                       if (res==0) res = 1;
                      break;
                      case 3:
                         //25xxx
                      case 4:
                         //95xxx
                         res = s95_read_param(buf.get(), curBlock * step, step, step, currentAlgorithm);
                      break;
                      case 5:
                         res = at45_read_param(buf.get(), curBlock * step, step, step, currentAlgorithm);
                      break;
                      default:
                         //Unsupport
                         QMessageBox::about(this, tr("Error"), tr("Unsupported chip type!"));
                         doNotDisturbCancel();
                         ch341a_spi_shutdown();
                         ui->checkBox_3->setStyleSheet("");
                      return;
                      }
                    // if res=-1 - error, stop
                    if (statusCH341 != 0)
                    {
                       QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
                       doNotDisturbCancel();
                       break;
                    }
                    if (res <= 0)
                    {
                        QMessageBox::about(this, tr("Error"), tr("Error reading block ") + QString::number(curBlock));
                        ch341a_spi_shutdown();
                        ui->pushButton->setStyleSheet(grnKeyStyle);
                        doNotDisturbCancel();
                        return;
                    }
                    for (j = 0; j < step; j++)
                    {
                      if (chipData[addr + j] != char(buf[addr + j - k * step]))
                          {
                            //error compare
                            QMessageBox::about(this, tr("Error"), tr("Error comparing data!\nAddress:   ") + hexiAddr(addr + j) + tr("\nBuffer: ") + bytePrint( static_cast<unsigned char>(chipData[addr + j])) + tr("    Chip: ") + bytePrint(buf[addr + j - k * step]));
                            ui->statusMessage->setText("");
                            ui->checkBox_3->setStyleSheet("");
                            ch341a_spi_shutdown();
                            doNotDisturbCancel();
                            return;
                           }
                     }
                     addr = addr + step;
                     curBlock++;
                     qApp->processEvents();
                     if (isHalted)
                     {
                         isHalted = false;
                         ch341a_spi_shutdown();
                         doNotDisturbCancel();
                         return;
                     }
                     ui->progressBar->setValue(static_cast<int>(curBlock));
                 }
             }
             else
             {
                //Not correct Number fnd size of blocks
               if (currentChipType == 0) QMessageBox::about(this, tr("Error"), tr("Before reading from chip please press 'Detect' button."));
               if (currentChipType == 1) QMessageBox::about(this, tr("Error"), tr("Please select the chip parameters - manufacture and chip name."));

             }
             doNotDisturbCancel();
             ui->statusMessage->setText("");
             ui->progressBar->setValue(0);
             ui->checkBox_3->setStyleSheet("");
             ui->crcEdit->setText(getCRC32(chipData));
             ch341a_spi_shutdown();
             QMessageBox::about(this, tr("Ok!"), tr("The operation was successful!"));
    }
      else
      {
          ch341StatusFlashing();
          QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
      }
}

void MainWindow::on_pushButton_3_clicked()
{
    ui->pushButton_3->setStyleSheet(redKeyStyle);
    if (ui->checkBox->isChecked()) MainWindow::on_actionErase_triggered();
    if (ui->checkBox_2->isChecked()) MainWindow::on_actionWrite_triggered();
    if (ui->checkBox_3->isChecked()) MainWindow::on_actionVerify_triggered();
    ui->pushButton_3->setStyleSheet(grnKeyStyle);
}

void MainWindow::receiveAddr(QString addressData)
{
    uint32_t ee, blockEndAddr = 0;
    int e,t;
    QString endType;
    e = addressData.indexOf("-");
    t = addressData.length();
    blockStartAddr = 0;
    blockLen = 0;
    endType = addressData.mid(t - 1, 1);
    blockStartAddr = hexToInt(addressData.mid(0, e));
    if (endType.compare("*")==0)
    {
        blockEndAddr = hexToInt(addressData.mid(e + 1, t - e - 2));
        if (blockEndAddr < blockStartAddr)
        {
            QMessageBox::about(this, tr("Error"), tr("The end address must be greater than the starting address."));
            return;
        }
        blockLen = blockEndAddr - blockStartAddr + 1;
    }
    else blockLen = hexToInt(addressData.mid(e + 1, t - e - 2));
    block.resize(static_cast<int>(blockLen));
    chipData = hexEdit->data();
    for (ee = 0; ee < blockLen; ee++)
    {
        block[ee] = chipData[ee + blockStartAddr];
    }
    ui->statusMessage->setText(tr("Saving block"));
    fileName = QFileDialog::getSaveFileName(this,
                                QString(tr("Save block")),
                                lastDirectory,
                                "Data Images (*.bin *.BIN);;All files (*.*)");
    QFileInfo info(fileName);
    lastDirectory = info.filePath();
    if (QString::compare(info.suffix(), "bin", Qt::CaseInsensitive)) fileName = fileName + ".bin";
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::about(this, tr("Error"), tr("Error saving file!"));
        return;
    }
    file.write(block);
    file.close();
    ui->statusMessage->setText("");
}

void MainWindow::receiveAddr2(QString addressData)
{
    uint32_t ee;
    QString endType;
    blockStartAddr = 0;
    blockLen = 0;
    blockStartAddr = hexToInt(addressData);
    ui->statusMessage->setText(tr("Opening block"));
    fileName = QFileDialog::getOpenFileName(this,
                                QString(tr("Open block")),
                                lastDirectory,
                                "Data Images (*.bin *.BIN);;All files (*.*)");
    ui->statusMessage->setText(tr("Current file: ") + fileName);
    QFileInfo info(fileName);
    lastDirectory = info.filePath();
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {

        return;
    }
    block = file.readAll();
    blockLen = static_cast<uint32_t>(block.size());
    chipData = hexEdit->data();
    if (blockStartAddr + blockLen > static_cast<uint32_t>(chipData.size()))
    {
        QMessageBox::about(this, tr("Error"), tr("The end address out of image size!"));
        return;
    }
    for (ee=0; ee < blockLen; ee++)
    {
        chipData[ee + blockStartAddr] = block[ee];
    }
    hexEdit->setData(chipData);
    file.close();
    ui->statusMessage->setText("");
}

void MainWindow::on_actionSave_Part_triggered()
{
    DialogSP* savePartDialog = new DialogSP();
    savePartDialog->show();

    connect(savePartDialog, SIGNAL(sendAddr(QString)), this, SLOT(receiveAddr(QString)));
}

void MainWindow::on_actionLoad_Part_triggered()
{
    DialogRP* loadPartDialog = new DialogRP();
    loadPartDialog->show();

    connect(loadPartDialog, SIGNAL(sendAddr2(QString)), this, SLOT(receiveAddr2(QString)));
}

void MainWindow::on_actionFind_Replace_triggered()
{
    //DialogSP* savePartDialog = new DialogSP();
    //savePartDialog->show();
    SearchDialog* searchDialog = new SearchDialog(hexEdit);
    searchDialog->show();
}

void MainWindow::ch341StatusFlashing()
{
    if (statusCH341 == 0)
    {
        ui->eStatus->setText(tr("Connected"));
        ui->eStatus -> setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 5px;color:#000;background:#9f0;font-weight:600;border-style:inset;}");
    }
    else
    {
        ui->eStatus->setText(tr("Not connected"));
        ui->eStatus -> setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 5px;color:#fff;background:#f00;font-weight:600;border-style:inset;}");
    }
}

void MainWindow::on_comboBox_type_currentIndexChanged(int index)
{
    int i, index2;
    ui->comboBox_man->clear();
    ui->comboBox_name->clear();
    ui->comboBox_man->addItem("");
    ui->comboBox_name->addItem("");    
    ui->jedecEdit->setText("");
    currentChipType = static_cast<uint8_t>(ui->comboBox_type->itemData(index).toInt());

    ui->comboBox_size->clear();
    ui->comboBox_size->addItem(" ", 0);

    switch (currentChipType)
       {
       case 0:
          //SPI
          ui->comboBox_size->addItem("64 K", 64 * 1024);
          ui->comboBox_size->addItem("128 K", 128 * 1024);
          ui->comboBox_size->addItem("256 K", 256 * 1024);
          ui->comboBox_size->addItem("512 K", 512 * 1024);
          ui->comboBox_size->addItem("1 M", 1024 * 1024);
          ui->comboBox_size->addItem("2 M", 2048 * 1024);
          ui->comboBox_size->addItem("4 M", 4096 * 1024);
          ui->comboBox_size->addItem("8 M", 8192 * 1024);
          ui->comboBox_size->addItem("16 M", 16384 * 1024);
          ui->comboBox_size->addItem("32 M", 32768 * 1024);
          ui->comboBox_size->addItem("64 M", 65536 * 1024);
          ui->comboBox_size->addItem("128 M", 65536 * 2048);
          ui->comboBox_size->addItem("256 M", 65536 * 4096);
       break;
       case 1:
          //I2C
          ui->comboBox_size->addItem("128 B", 128);
          ui->comboBox_size->addItem("256 B", 256);
          ui->comboBox_size->addItem("512 B", 512);
          ui->comboBox_size->addItem("1 K", 1 * 1024);
          ui->comboBox_size->addItem("2 K", 2 * 1024);
          ui->comboBox_size->addItem("4 K", 4 * 1024);
          ui->comboBox_size->addItem("8 K", 8 * 1024);
          ui->comboBox_size->addItem("16 K", 16 * 1024);
          ui->comboBox_size->addItem("32 K", 32 * 1024);
          ui->comboBox_size->addItem("64 K", 64 * 1024);
          ui->comboBox_size->addItem("128 K", 128 * 1024);
          ui->comboBox_size->addItem("256 K", 256 * 1024);
       break;
       case 2:
          //MicroWire
          ui->comboBox_size->addItem("128 B", 128);
          ui->comboBox_size->addItem("256 B", 256);
          ui->comboBox_size->addItem("512 B", 512);
          ui->comboBox_size->addItem("1 K", 1 * 1024);
          ui->comboBox_size->addItem("2 K", 2 * 1024);
       break;
       case 3:
          //25xxx
       case 4:
          //95xxx
          ui->comboBox_size->addItem("128 B", 128);
          ui->comboBox_size->addItem("256 B", 256);
          ui->comboBox_size->addItem("512 B", 512);
          ui->comboBox_size->addItem("1 K", 1 * 1024);
          ui->comboBox_size->addItem("2 K", 2 * 1024);
          ui->comboBox_size->addItem("4 K", 4 * 1024);
          ui->comboBox_size->addItem("8 K", 8 * 1024);
          ui->comboBox_size->addItem("16 K", 16 * 1024);
          ui->comboBox_size->addItem("32 K", 32 * 1024);
          ui->comboBox_size->addItem("64 K", 64 * 1024);
          ui->comboBox_size->addItem("128 K", 128 * 1024);
          ui->comboBox_size->addItem("256 K", 256 * 1024);
          ui->comboBox_size->addItem("512 K", 512 * 1024);
       break;
       case 5:
          //95xxx
          ui->comboBox_size->addItem("132 K",   132 * 1024);
          ui->comboBox_size->addItem("264 K",   264 * 1024);
          ui->comboBox_size->addItem("528 K",   528 * 1024);
          ui->comboBox_size->addItem("1056 K", 1056 * 1024);
          ui->comboBox_size->addItem("2112 K", 2112 * 1024);
          ui->comboBox_size->addItem("4224 K", 4224 * 1024);
          ui->comboBox_size->addItem("8448 K", 8448 * 1024);
       break;
       default:
          //Unsupport
       return;
       }

    for (i = 0; i<max_rec; i++)
    {
        //replacing items to combobox Manufacture
        index2 = ui->comboBox_man->findText(chips[i].chipManuf);
                    if (( index2 == -1 ) && (chips[i].chipTypeHex == currentChipType)) ui->comboBox_man->addItem(chips[i].chipManuf);
    }
     ui->comboBox_man->setCurrentIndex(0);
     ui->statusMessage->setText("");
     ui->comboBox_man->setCurrentIndex(0);
     ui->comboBox_vcc->setCurrentIndex(0);
     ui->comboBox_name->setCurrentIndex(0);
     ui->comboBox_page->setCurrentIndex(0);
     ui->comboBox_block->setCurrentIndex(0);
     ui->comboBox_size->setCurrentIndex(0);
     ui->comboBox_addr4bit->setCurrentIndex(0);
     if ((index > 0) && (index < 3))
     {
         ui->pushButton_2->hide();
         ui->comboBox_block->hide();
         ui->comboBox_addr4bit->hide();
         ui->label_8->hide();
         ui->label_9->hide();
         ui->actionDetect->setDisabled(true);
         ui->actionChip_info->setDisabled(true);

     }
     if (index !=0) ui->actionSecurity_registers->setEnabled(false);

     if (index == 0)
     {
         ui->pushButton_2->show();
         ui->comboBox_addr4bit->show();
         ui->label_8->show();
         ui->label_9->show();
         ui->comboBox_block->show();
         ui->actionDetect->setEnabled(true);
         ui->actionChip_info->setEnabled(true);
         ui->actionSecurity_registers->setEnabled(true);
     }
     if (index != 1)
     {
         ui->comboBox_i2cSpeed->hide();
         ui->label_10->hide();
     }
     else
     {
         ui->comboBox_i2cSpeed->show();
         ui->label_10->show();
     }
     if (index > 2)
     {
         ui->pushButton_2->hide();
         ui->comboBox_block->hide();
         ui->comboBox_addr4bit->hide();
         ui->label_8->hide();
         ui->label_9->hide();
         ui->actionDetect->setDisabled(true);
         ui->actionChip_info->setEnabled(true);
     }
     if (index == 5)
     {
          ui->pushButton_2->show();
          ui->actionDetect->setEnabled(true);
     }
}

void MainWindow::on_comboBox_addr4bit_currentIndexChanged(int index)
{
   currentAddr4bit = ui->comboBox_addr4bit->currentData().toUInt();
   index++;
}

void MainWindow::on_comboBox_i2cSpeed_currentIndexChanged(int index)
{
   currentI2CBusSpeed = static_cast<uint8_t>(ui->comboBox_i2cSpeed->currentData().toUInt());
   index++;
}

void MainWindow::on_actionAbout_triggered()
{
    DialogAbout* aboutDialog = new DialogAbout();
    aboutDialog->show();
}

void MainWindow::on_actionChecksum_calculate_triggered()
{
   //Refreshing CRC32
    ui->crcEdit->setText(getCRC32(chipData));
}

void MainWindow::on_actionEdit_chips_Database_triggered()
{
    QString programPath = "./IMSProg_editor";
    bool programExists = QFileInfo::exists(programPath) && !QDir(programPath).exists();

    if (!programExists)
    {
        // if does not exists on realive path check another
        QString systemProgramPath = QStandardPaths::findExecutable(programPath);

        if (!systemProgramPath.isEmpty())
        {
            // exists on system path
            programPath = systemProgramPath;
            programExists = true;
        }
    }

    if (programExists)
    {
        QProcess::execute(programPath, QStringList());
        progInit();
    }
    else
    {
       QMessageBox::about(this, tr("Error"), tr("File 'IMSProg_editor' not found!"));
    }
}

void MainWindow::doNotDisturb()
{
   ui->actionDetect->setDisabled(true);
   ui->actionOpen->setDisabled(true);
   ui->actionSave->setDisabled(true);
   ui->actionLoad_Part->setDisabled(true);
   ui->actionSave_Part->setDisabled(true);
   ui->actionExport_to_Intel_HEX->setDisabled(true);
   ui->actionImport_from_Intel_HEX->setDisabled(true);
   ui->actionExtract_from_ASUS_CAP->setDisabled(true);
   ui->actionEdit_chips_Database->setDisabled(true);
   ui->actionExit->setDisabled(true);
   ui->actionRead->setDisabled(true);
   ui->actionWrite->setDisabled(true);
   ui->actionErase->setDisabled(true);
   ui->actionVerify->setDisabled(true);
   ui->actionFind_Replace->setDisabled(true);
   ui->actionUndo->setDisabled(true);
   ui->actionRedo->setDisabled(true);
   ui->actionChecksum_calculate->setDisabled(true);
   ui->actionGoto_address->setDisabled(true);
   ui->actionCompare_files->setDisabled(true);
   ui->actionChip_info->setDisabled(true);
   ui->actionSecurity_registers->setDisabled(true);
   ui->actionStop->setDisabled(false);

   ui->pushButton->blockSignals(true);
   ui->pushButton_2->blockSignals(true);
   ui->pushButton_3->blockSignals(true);

   ui->comboBox_type->setDisabled(true);
   ui->comboBox_man->setDisabled(true);
   ui->comboBox_name->setDisabled(true);
   ui->comboBox_size->setDisabled(true);
   ui->comboBox_page->setDisabled(true);
   ui->comboBox_block->setDisabled(true);
   ui->comboBox_vcc->setDisabled(true);
   ui->comboBox_addr4bit->setDisabled(true);
   ui->comboBox_i2cSpeed->setDisabled(true);

   hexEdit->blockSignals(true);
   timer->stop();
}

void MainWindow::doNotDisturbCancel()
{
      if (currentChipType == 0) ui->actionDetect->setDisabled(false);
      ui->actionOpen->setDisabled(false);
      ui->actionSave->setDisabled(false);
      ui->actionLoad_Part->setDisabled(false);
      ui->actionSave_Part->setDisabled(false);
      ui->actionExport_to_Intel_HEX->setDisabled(false);
      ui->actionImport_from_Intel_HEX->setDisabled(false);
      ui->actionExtract_from_ASUS_CAP->setDisabled(false);
      ui->actionEdit_chips_Database->setDisabled(false);
      ui->actionExit->setDisabled(false);
      ui->actionRead->setDisabled(false);
      ui->actionWrite->setDisabled(false);
      ui->actionErase->setDisabled(false);
      ui->actionVerify->setDisabled(false);
      ui->actionFind_Replace->setDisabled(false);
      ui->actionUndo->setDisabled(false);
      ui->actionRedo->setDisabled(false);
      ui->actionChecksum_calculate->setDisabled(false);
      ui->actionGoto_address->setDisabled(false);
      ui->actionCompare_files->setDisabled(false);
      if ((currentChipType == 0) || (currentChipType > 2)) ui->actionChip_info->setDisabled(false);
      if (currentChipType == 0) ui->actionSecurity_registers->setDisabled(false);
      ui->actionStop->setDisabled(true);

      ui->pushButton->blockSignals(false);
      ui->pushButton_2->blockSignals(false);
      ui->pushButton_3->blockSignals(false);

      ui->comboBox_type->setDisabled(false);
      ui->comboBox_man->setDisabled(false);
      ui->comboBox_name->setDisabled(false);
      ui->comboBox_size->setDisabled(false);
      ui->comboBox_page->setDisabled(false);
      ui->comboBox_block->setDisabled(false);
      ui->comboBox_vcc->setDisabled(false);
      ui->comboBox_addr4bit->setDisabled(false);
      ui->comboBox_i2cSpeed->setDisabled(false);

      hexEdit->blockSignals(false);
      timer->start();
}

void MainWindow::on_actionStop_triggered()
{
  //ch341a_spi_shutdown();
  isHalted = true;
  QMessageBox::about(this, tr("Stop"), tr("Operation aborted!"));
  ui->pushButton->setStyleSheet(grnKeyStyle);
  ui->checkBox->setStyleSheet("");
  ui->checkBox_2->setStyleSheet("");
  ui->checkBox_3->setStyleSheet("");
  ui->pushButton_3->setStyleSheet(grnKeyStyle);
  ui->statusMessage->setText("");
  return;
}

void MainWindow::on_pushButton_4_clicked()
{
    //info form showing
    DialogInfo* infoDialog = new DialogInfo();
    infoDialog->show();
    if ((currentChipType == 0) && (ui->comboBox_vcc->currentIndex() == 1)) infoDialog->setChip(2); //NOR_FLASH 1.8
    if ((currentChipType == 0) && (ui->comboBox_vcc->currentIndex() == 2)) infoDialog->setChip(3); //NOR FLASH 3.3
    if ((currentChipType == 1) && (ui->comboBox_vcc->currentIndex() == 1)) infoDialog->setChip(1); //24xxx 3.3
    if ((currentChipType == 2) && (ui->comboBox_vcc->currentIndex() == 1)) infoDialog->setChip(4); //93xxx 3.3
    if ((currentChipType == 3) && (ui->comboBox_vcc->currentIndex() == 1)) infoDialog->setChip(2); //25xxx 3.3
    if ((currentChipType == 4) && (ui->comboBox_vcc->currentIndex() == 1)) infoDialog->setChip(2); //95xxx 3.3
    if ((currentChipType == 5) && (ui->comboBox_vcc->currentIndex() == 1)) infoDialog->setChip(5); //45xxx 3.3
}

void MainWindow::on_actionChip_info_triggered()
{
     timer->stop();
     if (currentChipType == 0)
     {
        DialogSFDP* sfdpDialog = new DialogSFDP();
        connect(sfdpDialog, SIGNAL(closeRequestHasArrived()), this, SLOT(closeSFDP()));
        sfdpDialog->show();
     }
     if (currentChipType > 2)
     {
         DialogSR* srDialog = new DialogSR();
         connect(srDialog, SIGNAL(closeRequestHasArrived()), this, SLOT(closeSR()));
         srDialog->show();
         srDialog->setChipType(currentChipType);
     }

}

void MainWindow::progInit()
{
    int index2;
    QString datFileNameMain = QDir::homePath() + "/.local/share/imsprog/IMSProg.Dat";
    QString datFileNameReserve = "/usr/share/imsprog/IMSProg.Dat";
    QString currentDatFilePath = "";
    //opening chip database file
    ui->statusMessage->setText(tr("Opening DAT file"));

    if (QFileInfo(datFileNameMain).exists()) currentDatFilePath = datFileNameMain;
    else if (QFileInfo(datFileNameReserve).exists()) currentDatFilePath = datFileNameReserve;

    QFile datfile(currentDatFilePath);
    QByteArray dataChips;
    if (!datfile.open(QIODevice::ReadOnly))
    {
        QMessageBox::about(this, tr("Error"), tr("Error loading chip database file!"));
        return;
    }
    dataChips = datfile.readAll();
    datfile.close();
    //parsing dat file
    ui->statusMessage->setText(tr("Parsing DAT file"));
    //parsing qbytearray
    char txtBuf[0x30];
    int i, j, recNo, dataPoz, dataSize, delay;
    uint32_t chipSize;
    uint16_t blockSize;
    unsigned char tmpBuf;
    dataPoz = 0;
    recNo = 0;
    QStringList verticalHeader;
    dataSize = dataChips.length();
    while (dataPoz < dataSize)
    {
        for (j=0; j<0x30; j++)
             {
                 txtBuf[j] = 0;
             }
        j = 0;
             while ((j < 0x10) && (dataChips[recNo * 0x44 + j] != ',')) // ASCII data reading
             {
                 txtBuf[j] = dataChips[recNo * 0x44 + j];
                 j++;
             }
             if (txtBuf[1] == 0x00) break;
             chips[recNo].chipTypeTxt = QByteArray::fromRawData(txtBuf, 0x30);
         for (i=0; i<0x30; i++)
             {
                 txtBuf[i] = 0;
             }
         j++;
         i = 0;
         while ((i < 0x20) && (dataChips[recNo * 0x44 + j] != ',')) // ASCII data reading
         {
             txtBuf[i] = dataChips[recNo * 0x44 + j];
             j++;
             i++;
         }
             chips[recNo].chipManuf = QByteArray::fromRawData(txtBuf, 0x30);
             for (i=0; i<0x30; i++)
                 {
                     txtBuf[i] = 0;
                 }
             j++;
             i = 0;
             while ((i < 0x30) && (dataChips[recNo * 0x44 + j] != '\0')) // ASCII data reading
             {
                 txtBuf[i] = dataChips[recNo * 0x44 + j];
                 j++;
                 i++;
             }
             chips[recNo].chipName = QByteArray::fromRawData(txtBuf, 0x30);
             chips[recNo].chipJedecIDMan = static_cast<uint8_t>(dataChips[recNo * 0x44 + 0x32]);
             chips[recNo].chipJedecIDDev = static_cast<uint8_t>(dataChips[recNo * 0x44 + 0x31]);
             chips[recNo].chipJedecIDCap = static_cast<uint8_t>(dataChips[recNo * 0x44 + 0x30]);
             tmpBuf = static_cast<unsigned char>(dataChips[recNo * 0x44 + 0x34]);
             chipSize = tmpBuf;
             tmpBuf = static_cast<unsigned char>(dataChips[recNo * 0x44 + 0x35]);
             chipSize = chipSize + tmpBuf * 256;
             tmpBuf = static_cast<unsigned char>(dataChips[recNo * 0x44 + 0x36]);
             chipSize = chipSize + tmpBuf * 256 * 256;
             tmpBuf = static_cast<unsigned char>(dataChips[recNo * 0x44 + 0x37]);
             chipSize = chipSize + tmpBuf * 256 * 256 * 256;
             chips[recNo].chipSize = chipSize;
             tmpBuf = static_cast<unsigned char>(dataChips[recNo * 0x44 + 0x38]);
             blockSize = tmpBuf;
             tmpBuf = static_cast<unsigned char>(dataChips[recNo * 0x44 + 0x39]);
             blockSize = blockSize + tmpBuf * 256;
             chips[recNo].sectorSize = blockSize;
             tmpBuf = static_cast<unsigned char>(dataChips[recNo * 0x44 + 0x3a]);
             chips[recNo].chipTypeHex = tmpBuf;
             tmpBuf = static_cast<unsigned char>(dataChips[recNo * 0x44 + 0x3b]);
             chips[recNo].algorithmCode = tmpBuf;
             tmpBuf = static_cast<unsigned char>(dataChips[recNo * 0x44 + 0x3c]);
             delay = tmpBuf;
             tmpBuf = static_cast<unsigned char>(dataChips[recNo * 0x44 + 0x3d]);
             delay = delay + tmpBuf * 256;
             chips[recNo].delay = delay;
             tmpBuf = static_cast<unsigned char>(dataChips[recNo * 0x44 + 0x3e]);
             chips[recNo].addr4bit = tmpBuf;
             tmpBuf = static_cast<unsigned char>(dataChips[recNo * 0x44 + 0x40]);
             chips[recNo].blockSize = tmpBuf * 1024;
             tmpBuf = static_cast<unsigned char>(dataChips[recNo * 0x44 + 0x42]);
             chips[recNo].eepromPages = tmpBuf;
             tmpBuf = static_cast<unsigned char>(dataChips[recNo * 0x44 + 0x43]);
             if (tmpBuf == 0x00) chips[recNo].chipVCC = "3.3 V";
             if (tmpBuf == 0x01) chips[recNo].chipVCC = "1.8 V";
             if (tmpBuf == 0x02) chips[recNo].chipVCC = "5.0 V";
             dataPoz = dataPoz + 0x44; //next record
             verticalHeader.append(QString::number(recNo));
             recNo++;
    }
    max_rec = recNo;
    //ui->comboBox_man->addItem("");
    for (i = 0; i<max_rec; i++)
    {
        //replacing items to combobox Manufacture
        index2 = ui->comboBox_man->findText(chips[i].chipManuf);
                    if ((index2 == -1) && (chips[i].chipTypeHex ==0)) ui->comboBox_man->addItem(chips[i].chipManuf);
    }
     ui->comboBox_man->setCurrentIndex(0);
     ui->statusMessage->setText("");
     currentChipType = 0;
     ui->comboBox_type->setCurrentIndex(0);
}

void MainWindow::slotTimerAlarm()
{
        statusCH341 = ch341a_spi_init();
        ch341StatusFlashing();
        ch341a_spi_shutdown();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
   QMainWindow::resizeEvent(event);
   hexEdit->setGeometry(0,0,ui->frame->width(),ui->frame->height());
}

void MainWindow::closeSFDP()
{
   timer->start();
}

void MainWindow::closeSR()
{
   timer->start();
}

void MainWindow::on_actionGoto_address_triggered()
{
    //HExEditor --> goto address
    DialogSetAddr* gotoAddrDialog = new DialogSetAddr();
    gotoAddrDialog->show();
    connect(gotoAddrDialog, SIGNAL(sendAddr3(qint64)), this, SLOT(receiveAddr3(qint64)));

}

void MainWindow::receiveAddr3(qint64 gotoAddr)
{
    hexEdit->setCursorPosition(gotoAddr * 2);
    hexEdit->ensureVisible();
}

void MainWindow::on_actionSecurity_registers_triggered()
{
    if (currentChipSize == 0)
    {
        QMessageBox::about(this, tr("Error"), tr("Before working with the security registers, click the 'Detect' button"));
        return;
    }
    if (currentAlgorithm > 0)
    {
        timer->stop();
        DialogSecurity* securityDialog = new DialogSecurity();
        connect(securityDialog, SIGNAL(closeRequestHasArrived()), this, SLOT(closeSR()));
        securityDialog->setAlgorithm(currentAlgorithm);
        securityDialog->setPath(lastDirectory);
        securityDialog->show();
    }
    else QMessageBox::about(this, tr("Error"), tr("There are no security registers in this chip or the current version of IMSProg does not support this algorithm."));

}

void MainWindow::on_actionExport_to_Intel_HEX_triggered()
{
    int addr = 0, hi_addr =0;
         QString result = "";
         chipData = hexEdit->data();
         int currSize = chipData.size();
         uint8_t i, counter = 0;
         int ostatok = 0;
         ui->progressBar->setRange(0,chipData.size());
         lastDirectory.replace(".bin", ".hex");
         ui->statusMessage->setText(tr("Saving file"));
         fileName = QFileDialog::getSaveFileName(this,
                                     QString(tr("Save file")),
                                     lastDirectory,
                                     "Intel HEX Images (*.hex *.HEX);;All files (*.*)");         
         QFileInfo info(fileName);
         ui->statusMessage->setText("");
         lastDirectory = info.filePath();
         if (QString::compare(info.suffix(), "hex", Qt::CaseInsensitive)) fileName = fileName + ".hex";
         ui->statusMessage->setText(tr("Current file: ") + info.fileName());
         QFile file(fileName);
         QTextStream stream(&file);
         if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
         {

             return;
         }
         stream.seek(file.size());
         while ((addr + hi_addr * 0x10000) < (chipData.size()))
         {

             if (addr < currSize - 0x20) ostatok = 0x20;
             else  ostatok =currSize - addr;

                result.append(":");
                result.append(bytePrint(static_cast<unsigned char>(ostatok)));              //number of bytes per string
                result.append(bytePrint(static_cast<unsigned char>((addr & 0xff00) >> 8))); //one address byte
                result.append(bytePrint(static_cast<unsigned char>(addr & 0xff)));          //zero address byte
                result.append("00");                                                        //type 00
                counter = counter + static_cast<unsigned char>(ostatok) + uint8_t((addr & 0xff00) >> 8) + uint8_t(addr & 0xff) ;
                for (i = 0; i < ostatok; i++)
                {
                    result.append(bytePrint(static_cast<unsigned char>((chipData[hi_addr * 256 * 256 + addr + i]))));
                    counter = counter + static_cast<uint8_t>((chipData[hi_addr * 256 * 256 + addr + i]));
                }
             result.append(bytePrint(0xff - counter + 1));
             counter = 0;
             stream << result << "\n";
             result.clear();

             addr = addr + 0x20;
             if (addr >= 0x10000)                                                            //command 04 - setting the high address
             {
                 hi_addr ++;
                 addr = addr - 0x10000;
                 result.append(":02000004");
                 result.append(bytePrint(uint8_t((hi_addr & 0xff00) >> 8)));
                 result.append(bytePrint(uint8_t(hi_addr & 0xff)));
                 counter = 0x06 + uint8_t((hi_addr & 0xff00) >> 8) + uint8_t(hi_addr & 0xff);
                 result.append(bytePrint(0xff - counter + 1));
                 counter = 0;
                 stream << result << "\n";
                 result.clear();
                 ui->progressBar->setValue(hi_addr * 256 * 256);
             }
         }
         result = ":00000001FF\n";                                                          //end string
         stream << result;
         file.close();
         fileName.clear();
         ui->progressBar->setValue(0);
}

void MainWindow::on_actionImport_from_Intel_HEX_triggered()
{
    chipData = hexEdit->data();
    int chipSize = int(currentChipSize);
    uint_fast32_t lineLen, lo_addr, hi_addr, command, i;
    unsigned char currByte;
    uint8_t counter, checkSUM;
    QString currStr ="", strVal = "";
    ui->statusMessage->setText(tr("Opening file"));
    fileName = QFileDialog::getOpenFileName(this,
                                QString(tr("Open file")),
                                lastDirectory,
                                "Intel HEX Images (*.hex *.HEX);;All files (*.*)");
    QFileInfo info(fileName);
    ui->statusMessage->setText(tr("Current file: ") + info.fileName());
    lastDirectory = info.filePath();
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {

        return;
    }
    hi_addr = 0;
    lo_addr = 0;
    ui->progressBar->setRange(0, chipSize);
    while (!file.atEnd())
    {
        currStr = file.readLine();
        counter = 0;
        //parsing string
        if (currStr[0] != ':')
        {
            QMessageBox::about(this, tr("Error"), tr("Not valid HEX format!"));
            file.close();
            fileName.clear();
            hexEdit->setData(chipData);
            ui->progressBar->setValue(0);
            ui->crcEdit->setText(getCRC32(chipData));
            return;
        }
        strVal = currStr.mid(1,2); //Length of data in current string
        lineLen = hexToInt(strVal);
        counter = counter + static_cast<unsigned char>(lineLen);

        strVal.clear();            //low address
        strVal = currStr.mid(3,4);
        lo_addr = hexToInt(strVal);
        counter = counter + static_cast<unsigned char>((lo_addr) >> 8) + static_cast<unsigned char>(lo_addr & 0x00ff);
        strVal.clear();            //command
        strVal = currStr.mid(7,2);
        command = hexToInt(strVal);
        counter = counter + static_cast<unsigned char>(command);

        if (command == 0) //reading bytes from current string
        {
            for (i = 0; i < lineLen; i++)
            {

                strVal.clear();            //get current byte of string
                strVal = currStr.mid(int(i) * 2 + 9, 2);
                currByte = static_cast<unsigned char>(hexToInt(strVal));
                // Checking valid address
                if (hi_addr * 256 * 256 + lo_addr + i > static_cast<unsigned long>(chipSize))
                {
                    QMessageBox::about(this, tr("Error"), tr("The address is larger than the size of the chip!"));
                    file.close();
                    fileName.clear();
                    hexEdit->setData(chipData);
                    ui->progressBar->setValue(0);
                    ui->crcEdit->setText(getCRC32(chipData));
                    return;
                }

                chipData.data()[hi_addr * 256 * 256 + lo_addr + i] = char(currByte);
                counter = counter + static_cast<unsigned char>(hexToInt(strVal));

            }
                counter = 255 - counter + 1;
                checkSUM = static_cast<unsigned char>(hexToInt( currStr.mid(int(i) * 2 + 9, 2)));

                if (counter != checkSUM)
                {
                    QMessageBox::about(this, tr("Error"), tr("Checksum error!"));
                    file.close();
                    fileName.clear();
                    hexEdit->setData(chipData);
                    ui->progressBar->setValue(0);
                    ui->crcEdit->setText(getCRC32(chipData));
                    return;
                }
        }
        if (command == 4) //Changing the high address
        {
            strVal.clear();            //low address
            strVal = currStr.mid(9,4);
            hi_addr = hexToInt(strVal);
            counter = counter + static_cast<unsigned char>((hi_addr) >> 8) + static_cast<unsigned char>(hi_addr & 0x00ff);
            counter = 255 - counter + 1;
            checkSUM = static_cast<unsigned char>(hexToInt( currStr.mid(13, 2)));
            ui->progressBar->setValue(int(hi_addr * 256 * 256));
            if (counter != checkSUM)
            {
                QMessageBox::about(this, tr("Error"), tr("Checksum error!"));
                return;
            }
        }


    }
    file.close();
    fileName.clear();
    hexEdit->setData(chipData);
    ui->progressBar->setValue(0);
    ui->crcEdit->setText(getCRC32(chipData));
}

void MainWindow::on_actionFill_test_image_triggered()
{
    int fileSize, addrSize, txtSize, i, j, curPos, hiDigit;
    char k;
    fileSize = chipData.size();
    ui->progressBar->setValue(0);
    ui->progressBar->setRange(0, fileSize);
    addrSize = 0;
    j = fileSize;
    hiDigit = 1;
    while (j > 1)
    {
        j = j / 16;
        addrSize ++;
        hiDigit = hiDigit * 16;
    }
    char digits[16];
    txtSize = 16 - addrSize - 4;
    curPos = 0;
    chipData.resize(0);
           k = 0x40;
           while (curPos < fileSize)
           {
               //String
              chipData.append('<');
              chipData.append('0');
              chipData.append('x');


              //calculate digits
              i = hiDigit / 16;
              for (j=addrSize - 1; j>=0; j--)
              {
                  digits[j] = (curPos / i) % 16;
                  i = i / 16;
              }
              for (j = addrSize -1; j >=0; j--)
              {
                 if (digits[j] < 10) digits[j] = digits[j] + 0x30;
                 else digits[j] = digits[j] + 0x37;
                 chipData.append(digits[j]);
              }

              chipData.append('>');
              for (i = 0; i < txtSize; i++)
              {
                  chipData.append(k);
                  k ++;
                  if (k > 0x7e) k = 0x40;
              }
              curPos = curPos + 16;
              if (curPos % 512 == 0) ui->progressBar->setValue(curPos);
           }
    hexEdit->setData(chipData);
    ui->crcEdit->setText(getCRC32(chipData));
    ui->progressBar->setValue(0);
}

void MainWindow::on_actionCompare_files_triggered()
{
    DialogCompare* compDialog = new DialogCompare();
    compDialog->show();
    compDialog->showArrays(&chipData, &oldChipData, &newFileName, &oldFileName);
}

void MainWindow::preparingToCompare(bool type)
{
    //For comparing function
    // type = 0 - file reading
    // type = 1 - chip reading
    if (filled == 0) oldChipData = hexEdit->data();
    if (type == 0)
    {
        if (numberOfReads > 0)
        {
           //oldFileName = ui->comboBox_name->currentText();
           numberOfReads = 0;
        }
        newFileName = fileName;
    }
    else
    {
        newFileName = ui->comboBox_name->currentText();
    }
}
