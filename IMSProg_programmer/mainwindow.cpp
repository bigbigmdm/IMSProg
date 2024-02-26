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

 ui->actionStop->setDisabled(true);
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
 ui->comboBox_page->addItem("512", 512);

 ui->comboBox_size->addItem(" ", 0);
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
 ui->comboBox_size->addItem("1 M", 1024 * 1024);
 ui->comboBox_size->addItem("2 M", 2048 * 1024);
 ui->comboBox_size->addItem("4 M", 4096 * 1024);
 ui->comboBox_size->addItem("8 M", 8192 * 1024);
 ui->comboBox_size->addItem("16 M", 16384 * 1024);
 ui->comboBox_size->addItem("32 M", 32768 * 1024);
 ui->comboBox_size->addItem("64 M", 65536 * 1024);

 ui->comboBox_block->addItem(" ", 0);
 ui->comboBox_block->addItem("64 K", 64 * 1024);

 currentChipSize = 0;
 currentNumBlocks = 0;
 currentBlockSize = 0;
 currentPageSize = 0;
 currentAlgorithm = 0;
 currentChipType = 0;
 blockStartAddr = 0;
 blockLen = 0;
 currentAddr4bit = 0;
 cmdStarted = false;
 // connect and status check
 statusCH341 = ch341a_spi_init();
 ch341StatusFlashing();
 chipData.reserve(64 * 1024 *1024 + 2048);
 chipData.resize(256);
 chipData.fill(char(0xff));
 ch341a_spi_shutdown();
 hexEdit = new QHexEdit(ui->frame);
 hexEdit->setGeometry(0,0,ui->frame->width(),ui->frame->height());
 hexEdit->setData(chipData);
 hexEdit->setHexCaps(true);
 defaultTextColor = ui->label->palette().color(QPalette::Text);
 hexEdit->setAsciiFontColor(defaultTextColor);
 hexEdit->setAddressFontColor(defaultTextColor);
 hexEdit->setHexFontColor(defaultTextColor);
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
  int res = 0;
  statusCH341 = ch341a_init(currentChipType);
  if (statusCH341 == 0)
  {
    ui->crcEdit->setText("");
    if (((currentNumBlocks > 0) && (currentBlockSize >0) && (currentChipType == 0)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 1)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 2)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 3)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 4)))
    {
       doNotDisturb();
       if (currentChipType == 1)
       {
           currentBlockSize = 128;
           currentNumBlocks = currentChipSize / currentBlockSize;
       }
       if ((currentChipType == 2) ||(currentChipType == 3) || (currentChipType == 4))
       {
           currentBlockSize = currentPageSize;
           currentNumBlocks = currentChipSize / currentBlockSize;
       }

       ch341StatusFlashing();
       uint32_t addr = 0;
       uint32_t curBlock = 0;
       uint32_t j, k;

       //progerssbar settings
       ui->progressBar->setRange(0, static_cast<int>(currentNumBlocks));
       ui->progressBar->setValue(0);
       //uint8_t buf[currentBlockSize];
       uint8_t *buf;
       buf = (uint8_t *)malloc(currentBlockSize);
       ui->pushButton->setStyleSheet(redKeyStyle);
       ui->statusBar->showMessage(tr("Reading data from ") + ui->comboBox_name->currentText());
       for (k = 0; k < currentNumBlocks; k++)
       {
           switch (currentChipType)
              {
              case 0:
                 //SPI
                 res = snor_read_param(buf,curBlock * currentBlockSize, currentBlockSize, currentBlockSize, currentAddr4bit);
              break;
              case 1:
                 //I2C
               res = ch341readEEPROM_param(buf, curBlock * currentBlockSize, currentBlockSize, currentChipSize, currentPageSize, currentAlgorithm);//currentAlgorithm);
               if (res==0) res = 1;
              break;
              case 2:
                 //MicroWire
               res = Read_EEPROM_3wire_param(buf, static_cast<int>(curBlock * currentBlockSize), static_cast<int>(currentBlockSize), static_cast<int>(currentChipSize), currentAlgorithm);
               if (res==0) res = 1;
              break;
              case 3:
                 //25xxx
              case 4:
                 //95xxx
                 res = s95_read_param(buf,curBlock * currentBlockSize, currentBlockSize, currentBlockSize, currentAlgorithm);
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
               doNotDisturbCancel();
               return;
            }
         for (j = 0; j < currentBlockSize; j++)
            {
                  chipData[addr + j] = char(buf[addr + j - k * currentBlockSize]);
            }
          addr = addr + currentBlockSize;
          curBlock++;
          if (curBlock * currentBlockSize < 413300) hexEdit->setData(chipData); //show buffer in hehedit while chip data is being loaded
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
    ui->statusBar->showMessage("");
    ui->progressBar->setValue(0);
    ui->pushButton->setStyleSheet(grnKeyStyle);
    ui->crcEdit->setText(getCRC32());
  }
  else
  {
      ch341StatusFlashing();
      QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
  }
  ch341a_spi_shutdown();
  doNotDisturbCancel();
}

void MainWindow::on_pushButton_2_clicked()
{
    timer->stop();
    //searching the connected chip in database
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
    if ((bufid[0] == 0xff) && (bufid[1] == 0xff) && (bufid[2] == 0xff))
    {
        QMessageBox::about(this, tr("Error"), tr("The chip is not connect or missing!"));
        ui->pushButton_2->setStyleSheet(grnKeyStyle);
        ch341a_spi_shutdown();
        return;
    }
    ui->jedecEdit->setText(bytePrint(bufid[0]) + " " + bytePrint(bufid[1]) + " " + bytePrint(bufid[2]));
    for (i = 0; i< max_rec; i++)
    {
        if ((bufid[0] == chips[i].chipJedecIDMan) && (bufid[1] == chips[i].chipJedecIDDev) && (bufid[2] == chips[i].chipJedecIDCap))
        {            
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
    currentChipSize = ui->comboBox_size->currentData().toUInt();
    currentBlockSize = ui->comboBox_block->currentData().toUInt();
    currentPageSize = ui->comboBox_page->currentData().toUInt();
    currentAddr4bit = ui->comboBox_addr4bit->currentData().toUInt();
    if ((currentChipSize !=0) && (currentBlockSize!=0)  && (currentChipType == 0))
    {
    currentNumBlocks = currentChipSize / currentBlockSize;
    chipData.resize(static_cast<int>(currentChipSize));
    chipData.fill(char(0xff));
    hexEdit->setData(chipData);
    }
    if ((currentChipSize !=0) && (currentPageSize!=0)  && (currentChipType == 1))
    {
    currentNumBlocks = currentChipSize / currentPageSize;
    chipData.resize(static_cast<int>(currentChipSize));
    chipData.fill(char(0xff));
    hexEdit->setData(chipData);
    }
    ui->pushButton_2->setStyleSheet(grnKeyStyle);
    ui->crcEdit->setText(getCRC32());
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
        chipData.resize(static_cast<int>(currentChipSize));
        chipData.fill(char(0xff));
        hexEdit->setData(chipData);
    }
    if ((currentChipSize !=0) && (currentPageSize!=0)  && (currentChipType > 0))
    {
    currentNumBlocks = currentChipSize / currentPageSize;
    chipData.resize(static_cast<int>(currentChipSize));
    chipData.fill(char(0xff));
    hexEdit->setData(chipData);
    }
    index = index + 0;
}

void MainWindow::on_comboBox_page_currentIndexChanged(int index)
{
    currentChipSize = ui->comboBox_size->currentData().toUInt();
    currentBlockSize = ui->comboBox_block->currentData().toUInt();
    currentAddr4bit = ui->comboBox_addr4bit->currentData().toUInt();
    if ((currentChipSize !=0) && (currentBlockSize!=0) && (currentChipType ==0))
    {
        currentNumBlocks = currentChipSize / currentBlockSize;
        chipData.resize(static_cast<int>(currentChipSize));
        chipData.fill(char(0xff));
        hexEdit->setData(chipData);
    }
    if ((currentChipSize !=0) && (currentPageSize!=0)  && (currentChipType > 0))
    {
    currentNumBlocks = currentChipSize / currentPageSize;
    chipData.resize(static_cast<int>(currentChipSize));
    chipData.fill(char(0xff));
    hexEdit->setData(chipData);
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

    ui->statusBar->showMessage(tr("Saving file"));
    fileName = QFileDialog::getSaveFileName(this,
                                QString(tr("Save file")),
                                lastDirectory,
                                "Data Images (*.bin *.BIN);;All files (*.*)");
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
}

void MainWindow::on_actionErase_triggered()
{
    //statusCH341 = ch341a_spi_init();
    statusCH341 = ch341a_init(currentChipType);
    ch341StatusFlashing();
    if (statusCH341 != 0)
      {
        QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
        return;
      }
    ui->statusBar->showMessage(tr("Erasing the ") + ui->comboBox_name->currentText());
    ui->checkBox->setStyleSheet("QCheckBox{font-weight:600;}");
    ui->centralWidget->repaint();
    ui->progressBar->setRange(0, 100);
    doNotDisturb();
    if (currentChipType == 0)
    {
       ui->progressBar->setValue(50);
       full_erase_chip();
       sleep(1);
    }
    if ((currentChipType == 4) || ((currentChipType == 3) && ((currentAlgorithm & 0x20) == 0)))
    {
        uint32_t curBlock = 0;
        uint32_t k;
        int res = 0;
        currentBlockSize = currentPageSize;
        currentNumBlocks = currentChipSize / currentBlockSize;
        uint8_t *buf;
        buf = (uint8_t *)malloc(currentBlockSize);
        config_stream(2);
        if (isHalted)
        {
            isHalted = false;
            ch341a_spi_shutdown();
            doNotDisturbCancel();
            return;
        }
        ui->progressBar->setRange(0, static_cast<int>(currentNumBlocks));
        for (k = 0; k < currentBlockSize; k++)
        {
            buf[k] = 0xff;
        }
        for (curBlock = 0; curBlock < currentNumBlocks; curBlock++)
        {
            //res = ch341writeEEPROM_param(buf, curBlock * 128, 128, currentPageSize, currentAlgorithm);
            res =  s95_write_param(buf, curBlock * currentBlockSize, currentBlockSize, currentBlockSize, currentAlgorithm);
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
        currentBlockSize = 128;
        currentNumBlocks = currentChipSize / currentBlockSize;
        uint8_t *buf;
        buf = (uint8_t *)malloc(currentBlockSize);
        config_stream(2);
        if (isHalted)
        {
            isHalted = false;
            ch341a_spi_shutdown();
            doNotDisturbCancel();
            return;
        }
        ui->progressBar->setRange(0, static_cast<int>(currentNumBlocks));
        for (k = 0; k < currentBlockSize; k++)
        {
            buf[k] = 0xff;
        }
        for (curBlock = 0; curBlock < currentNumBlocks; curBlock++)
        {
            res = ch341writeEEPROM_param(buf, curBlock * 128, 128, currentPageSize, currentAlgorithm);
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
    doNotDisturbCancel();
    ui->checkBox->setStyleSheet("");
    ui->statusBar->showMessage("");
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
    ui->statusBar->showMessage(tr("Opening file"));
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
    ui->statusBar->showMessage(tr("Current file: ") + info.fileName());
    lastDirectory = info.filePath();
    // if ChipSze = 0 (Chip is not selected) IMSProg using at hexeditor only. chipsize -> hexedit.data
    // if ChipSize < FileSize - showing error message
    // if Filesize <= ChipSize - filling fileArray to hexedit.Data, the end of the array chipData remains filled 0xff
    QFile file(fileName);
    if ((info.size() > currentChipSize) && (currentChipSize != 0))
    {
      QMessageBox::about(this, tr("Error"), tr("The file size exceeds the chip size. Please select another chip or file or use `Save part` to split the file."));
      return;
    }
    if (!file.open(QIODevice::ReadOnly))
    {

        return;
    }
    buf.resize(static_cast<int>(info.size()));
    buf = file.readAll();
    if (currentChipSize == 0)
    {
        chipData.resize(static_cast<int>(info.size()));
    }

    chipData.replace(0, static_cast<int>(info.size()), buf);
    hexEdit->setData(chipData);

    file.close();
    //ui->statusBar->showMessage("");
    ui->crcEdit->setText(getCRC32());
}

void MainWindow::on_actionExtract_from_ASUS_CAP_triggered()
{
    QByteArray buf;
    ui->statusBar->showMessage(tr("Opening file"));
    fileName = QFileDialog::getOpenFileName(this,
                                QString(tr("Open file")),
                                lastDirectory,
                                "ASUS Data Images (*.cap *.CAP);;All files (*.*)");
    QFileInfo info(fileName);
    ui->statusBar->showMessage(tr("Current file: ") + info.fileName());
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
    ui->crcEdit->setText(getCRC32());
}

void MainWindow::on_actionWrite_triggered()
{
    //Writting data to chip
    int res = 0;
    statusCH341 = ch341a_init(currentChipType);
    if (statusCH341 == 0)
    {
    if (((currentNumBlocks > 0) && (currentBlockSize >0) && (currentChipType == 0)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 1)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 2)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 3)) ||
         ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 4)))
        {
        doNotDisturb();
        if (currentChipType == 1)
        {
            currentBlockSize = 128;
            currentNumBlocks = currentChipSize / currentBlockSize;
        }
        if ((currentChipType == 2) || (currentChipType == 3) || (currentChipType == 4))
        {
            currentBlockSize = currentPageSize;
            currentNumBlocks = currentChipSize / currentBlockSize;
        }
        ch341StatusFlashing();
    uint32_t addr = 0;
    uint32_t curBlock = 0;    
    uint32_t j, k;
    ui->statusBar->showMessage(tr("Writing data to ") + ui->comboBox_name->currentText());
    //progerssbar settings
    ui->progressBar->setRange(0, static_cast<int>(currentNumBlocks));
    ui->checkBox_2->setStyleSheet("QCheckBox{font-weight:800;}");
    chipData = hexEdit->data();
    //uint8_t buf[currentBlockSize];
    uint8_t *buf;
    buf = (uint8_t *)malloc(currentBlockSize);
    for (k = 0; k < currentNumBlocks; k++)
      {

         for (j = 0; j < currentBlockSize; j++)
            {
               buf[addr + j - k * currentBlockSize] =  static_cast<uint8_t>(chipData[addr + j]) ;
            }
         switch (currentChipType)
                       {
                       case 0:
                          //SPI
                          res =  snor_write_param(buf, addr, currentBlockSize, currentBlockSize, currentAddr4bit);
                       break;
                       case 1:
                          //I2C
                          res = ch341writeEEPROM_param(buf, curBlock * 128, 128, currentPageSize, currentAlgorithm);
                          if (res==0) res = 1;
                       break;
                       case 2:
                          //MicroWire
                          res = Write_EEPROM_3wire_param(buf, static_cast<int>(curBlock * currentBlockSize), static_cast<int>(currentBlockSize), static_cast<int>(currentChipSize), currentAlgorithm);
                          if (res==0) res = 1;
                       break;
                       case 3:
                          //25xxx
                       case 4:
                          //M95xx
                          res =  s95_write_param(buf, addr, currentBlockSize, currentBlockSize, currentAlgorithm);                         
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
         addr = addr + currentBlockSize;
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
    ui->statusBar->showMessage("");    
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
        ui->statusBar->showMessage("");
   }
 index = index + 0;
}

void MainWindow::on_comboBox_name_currentIndexChanged(const QString &arg1)
{
    int i, index;
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

       if ((currentChipSize !=0) && (currentBlockSize!=0) && (currentChipType == 0))
       {
           currentNumBlocks = currentChipSize / currentBlockSize;
           chipData.resize(static_cast<int>(currentChipSize));
           chipData.fill(char(0xff));
           hexEdit->setData(chipData);
       }
       if ((currentChipSize !=0) && (currentPageSize!=0)  && (currentChipType > 0))
       {
           currentNumBlocks = currentChipSize / currentPageSize;
           chipData.resize(static_cast<int>(currentChipSize));
            chipData.fill(char(0xff));
           hexEdit->setData(chipData);
       }

    }
}

void MainWindow::on_actionVerify_triggered()
{
    //Reading and veryfying data from chip
    int res =0;
    statusCH341 = ch341a_init(currentChipType);
    if (statusCH341 == 0)
    {
       if (((currentNumBlocks > 0) && (currentBlockSize >0) && (currentChipType == 0)) ||
            ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 1)) ||
            ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 2)) ||
            ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 3)) ||
            ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 4)))
           {
               ui->crcEdit->setText("");
               doNotDisturb();
               if (currentChipType == 1)
               {
                 currentBlockSize = 128;
                 currentNumBlocks = currentChipSize / currentBlockSize;
               }
               if ((currentChipType == 2) || (currentChipType == 3) || (currentChipType == 4))
               {
                   currentBlockSize = currentPageSize;
                   currentNumBlocks = currentChipSize / currentBlockSize;
               }
               ch341StatusFlashing();
               uint32_t addr = 0;
               uint32_t curBlock = 0;
               uint32_t j, k;               
               //progerssbar settings
               ui->progressBar->setRange(0, static_cast<int>(currentNumBlocks));
               ui->progressBar->setValue(0);
               //uint8_t buf[currentBlockSize];
               uint8_t *buf;
               buf = (uint8_t *)malloc(currentBlockSize);
               chipData = hexEdit->data();
               ui->checkBox_3->setStyleSheet("QCheckBox{font-weight:800;}");
               ui->statusBar->showMessage(tr("Veryfing data from ") + ui->comboBox_name->currentText());
               for (k = 0; k < currentNumBlocks; k++)
               {
                   switch (currentChipType)
                      {
                      case 0:
                         //SPI
                         res = snor_read_param(buf,curBlock * currentBlockSize, currentBlockSize, currentBlockSize, currentAddr4bit);
                      break;
                      case 1:
                         //I2C
                       res = ch341readEEPROM_param(buf, curBlock * currentBlockSize, currentBlockSize, currentChipSize, currentPageSize, currentAlgorithm);//currentAlgorithm);
                       if (res==0) res = 1;
                      break;
                      case 2:
                         //MicroWire
                       res = Read_EEPROM_3wire_param(buf, static_cast<int>(curBlock * currentBlockSize), static_cast<int>(currentBlockSize), static_cast<int>(currentChipSize), currentAlgorithm);
                       if (res==0) res = 1;
                      break;
                      case 3:
                         //25xxx
                      case 4:
                         //95xxx
                         res = s95_read_param(buf,curBlock * currentBlockSize, currentBlockSize, currentBlockSize, currentAlgorithm);
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
                        doNotDisturbCancel();
                        return;
                    }
                    for (j = 0; j < currentBlockSize; j++)
                    {
                      if (chipData[addr + j] != char(buf[addr + j - k * currentBlockSize]))
                          {
                            //error compare
                            QMessageBox::about(this, tr("Error"), tr("Error comparing data!\nAddress:   ") + hexiAddr(addr + j) + tr("\nBuffer: ") + bytePrint( static_cast<unsigned char>(chipData[addr + j])) + tr("    Chip: ") + bytePrint(buf[addr + j - k * currentBlockSize]));
                            ui->statusBar->showMessage("");
                            ui->checkBox_3->setStyleSheet("");
                            ch341a_spi_shutdown();
                            doNotDisturbCancel();
                            return;
                           }
                     }
                     addr = addr + currentBlockSize;
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
             ui->statusBar->showMessage("");
             ui->progressBar->setValue(0);
             ui->checkBox_3->setStyleSheet("");
             ui->crcEdit->setText(getCRC32());
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
    ui->statusBar->showMessage(tr("Saving block"));
    fileName = QFileDialog::getSaveFileName(this,
                                QString(tr("Save block")),
                                lastDirectory,
                                "Data Images (*.bin *.BIN);;All files (*.*)");
    QFileInfo info(fileName);
    lastDirectory = info.filePath();
    if (QString::compare(info.suffix(), "bin", Qt::CaseInsensitive)) fileName = fileName + ".bin";
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::about(this, tr("Error"), tr("Error saving file!"));
        return;
    }
    file.write(block);
    file.close();
}

void MainWindow::receiveAddr2(QString addressData)
{
    uint32_t ee;
    QString endType;
    blockStartAddr = 0;
    blockLen = 0;
    blockStartAddr = hexToInt(addressData);
    ui->statusBar->showMessage(tr("Opening block"));
    fileName = QFileDialog::getOpenFileName(this,
                                QString(tr("Open block")),
                                lastDirectory,
                                "Data Images (*.bin *.BIN);;All files (*.*)");
    ui->statusBar->showMessage(tr("Current file: ") + fileName);
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
    ui->statusBar->showMessage("");
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
    for (i = 0; i<max_rec; i++)
    {
        //replacing items to combobox Manufacture
        index2 = ui->comboBox_man->findText(chips[i].chipManuf);
                    if (( index2 == -1 ) && (chips[i].chipTypeHex == currentChipType)) ui->comboBox_man->addItem(chips[i].chipManuf);
    }
     ui->comboBox_man->setCurrentIndex(0);
     ui->statusBar->showMessage("");
     ui->comboBox_man->setCurrentIndex(0);
     ui->comboBox_vcc->setCurrentIndex(0);
     ui->comboBox_name->setCurrentIndex(0);
     ui->comboBox_page->setCurrentIndex(0);
     ui->comboBox_block->setCurrentIndex(0);
     ui->comboBox_size->setCurrentIndex(0);
     ui->comboBox_addr4bit->setCurrentIndex(0);
     if (index > 0)
     {
         ui->pushButton_2->hide();
         ui->comboBox_block->hide();
         ui->comboBox_addr4bit->hide();
         ui->label_8->hide();
         ui->label_9->hide();
         //QMenu::actionAt(on_actionDetect_triggered()).setDisabled;
         //ui->menuBar->actionAt(QPoint(0,0))->setDisabled(true);
         ui->actionDetect->setDisabled(true);
         ui->actionChip_info->setDisabled(true);

     }
     if (index == 0)
     {
         ui->pushButton_2->show();
         ui->comboBox_block->show();
         ui->comboBox_addr4bit->show();
         ui->label_8->show();
         ui->label_9->show();
         ui->actionDetect->setEnabled(true);
         ui->actionChip_info->setEnabled(true);
     }
}

void MainWindow::on_comboBox_addr4bit_currentIndexChanged(int index)
{
   currentAddr4bit = ui->comboBox_addr4bit->currentData().toUInt();
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
    ui->crcEdit->setText(getCRC32());
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
   ui->actionChip_info->setDisabled(true);
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
      if (currentChipType == 0) ui->actionChip_info->setDisabled(false);
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
  ui->statusBar->showMessage("");
  return;
}
void MainWindow::on_pushButton_4_clicked()
{
    //info form showing
    DialogInfo* infoDialog = new DialogInfo();
    infoDialog->show();
    if ((currentChipType == 0) && (ui->comboBox_vcc->currentIndex() == 1)) infoDialog->setChip(2);
    if ((currentChipType == 0) && (ui->comboBox_vcc->currentIndex() == 2)) infoDialog->setChip(3);
    if ((currentChipType == 1) && (ui->comboBox_vcc->currentIndex() == 1)) infoDialog->setChip(1);
    if ((currentChipType == 2) && (ui->comboBox_vcc->currentIndex() == 1)) infoDialog->setChip(4);
    if ((currentChipType == 3) && (ui->comboBox_vcc->currentIndex() == 1)) infoDialog->setChip(2);
    if ((currentChipType == 4) && (ui->comboBox_vcc->currentIndex() == 1)) infoDialog->setChip(2);
}

void MainWindow::on_actionChip_info_triggered()
{
     timer->stop();
     DialogSFDP* sfdpDialog = new DialogSFDP();
     connect(sfdpDialog, SIGNAL(closeRequestHasArrived()), this, SLOT(closeSFDP()));
     sfdpDialog->show();


}

void MainWindow::progInit()
{
    int index2;
    QString datFileNameMain = QDir::homePath() + "/.local/share/imsprog/IMSProg.Dat";
    QString datFileNameReserve = "/usr/share/imsprog/IMSProg.Dat";
    QString currentDatFilePath = "";
    //opening chip database file
    ui->statusBar->showMessage(tr("Opening DAT file"));

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
    ui->statusBar->showMessage(tr("Parsing DAT file"));
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
     ui->statusBar->showMessage("");
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
//*****************************************************
//       HEX ULTLITY by Mikhail Medvedev
//*****************************************************
uint32_t MainWindow::hexToInt(QString str)
{
    unsigned char c;
    uint32_t len = static_cast<uint32_t>(str.length());
    QByteArray bstr = str.toLocal8Bit();
    if ((len > 0) && (len < 8))
    {
        uint32_t i, j = 1;
        uint32_t  addr = 0;
        for (i = len; i >0; i--)
        {
           c = static_cast<unsigned char>(bstr[i-1]);
           if ((c >= 0x30) && (c <=0x39)) addr =  addr + (c - 0x30) * j;
           if ((c >= 0x41) && (c <= 0x46)) addr = addr + (c - 0x37) * j;
           if ((c >= 0x61) && (c <= 0x66)) addr = addr + (c - 0x57) * j;
        j = j * 16;
        }
        return addr;
    }
    else return 0;
}

QString MainWindow::hexiAddr(uint32_t add)
{
 QString rez = "";
 uint8_t A,B,C,D;
 D = 0xFF & add;
 add = add >> 8;
 C = 0xFF & add;
 add = add >> 8;
 B = 0xFF & add;
 add = add >> 8;
 A = 0xFF & add;
 rez = bytePrint(A) + bytePrint(B) + bytePrint(C) + bytePrint(D);
 return rez;
}

QString MainWindow::bytePrint(unsigned char z)
{
    unsigned char s;
    s = z / 16;
    if (s > 0x9) s = s + 0x37;
    else s = s + 0x30;
    z = z % 16;
    if (z > 0x9) z = z + 0x37;
    else z = z + 0x30;
    return QString(s) + QString(z);
}

QString MainWindow::sizeConvert(int a)
{
    QString rez;
    rez = "0";
    if (a < 1024) rez = QString::number(a) + " B";
    else if ((a < 1024 * 1024)) rez = QString::number(a/1024) + " K";
    else rez = QString::number(a/1024/1024) + " M";
    return rez;
}

QString MainWindow::getCRC32()
{
    chipData = hexEdit->data();
    uint32_t size, i;
    uint_least32_t crc = 0xFFFFFFFF;
    const uint_least32_t Crc32Table[256] = {
        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
        0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
        0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
        0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
        0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
        0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
        0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
        0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
        0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
        0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
        0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
        0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
        0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
        0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
        0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
        0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
        0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
        0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
        0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
        0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
        0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
        0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
        0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
        0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
        0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
        0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
        0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
        0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
        0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
        0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
        0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
        0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
        0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
        0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
        0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
        0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
        0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
        0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
        0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
        0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
        0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
        0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
        0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
        0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
        0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
        0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
        0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
        0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
        0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
        0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
        0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
        0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
        0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
        0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
        0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
        0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
        0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
        0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
        0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
        0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
        0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
        0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
        0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
        0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
    };
    size = static_cast<uint32_t>(chipData.length());
    i = 0;
    while (size--)
            {
              crc = (crc >> 8) ^ Crc32Table[(crc ^ static_cast<uint_least32_t>(chipData[i])) & 0xFF];
              i++;
            }
        return hexiAddr(crc ^ 0xFFFFFFFF);
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
         ui->statusBar->showMessage(tr("Saving file"));
         fileName = QFileDialog::getSaveFileName(this,
                                     QString(tr("Save file")),
                                     lastDirectory,
                                     "Intel HEX Images (*.hex *.HEX);;All files (*.*)");         
         QFileInfo info(fileName);
         lastDirectory = info.filePath();
         if (QString::compare(info.suffix(), "hex", Qt::CaseInsensitive)) fileName = fileName + ".hex";
         ui->statusBar->showMessage(tr("Current file: ") + info.fileName());
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
    ui->statusBar->showMessage(tr("Opening file"));
    fileName = QFileDialog::getOpenFileName(this,
                                QString(tr("Open file")),
                                lastDirectory,
                                "Intel HEX Images (*.hex *.HEX);;All files (*.*)");
    QFileInfo info(fileName);
    ui->statusBar->showMessage(tr("Current file: ") + info.fileName());
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
            ui->crcEdit->setText(getCRC32());
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
                    ui->crcEdit->setText(getCRC32());
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
                    ui->crcEdit->setText(getCRC32());
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
    ui->crcEdit->setText(getCRC32());
}
