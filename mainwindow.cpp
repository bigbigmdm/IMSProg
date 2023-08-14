/*
 * Copyright (C) 2023 Mikhail Medvedev <e-ink-reader@yandex.ru>
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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QErrorMessage>
#include <QDragEnterEvent>
#include <QtGui>
#include "qhexedit.h"
#include "dialogsp.h"
#include "dialogrp.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    int index2;
    max_rec = 0;
    ui->setupUi(this);
    ui->statusBar->addPermanentWidget(ui->lStatus,0);
    ui->statusBar->addPermanentWidget(ui->eStatus,0);
    ui->statusBar->addPermanentWidget(ui->jLabel,0);
    ui->statusBar->addPermanentWidget(ui->jedecEdit,0);
    ui->progressBar->setValue(0);
    ui->comboBox_name->addItems({""});
    ui->comboBox_man->addItems({""});
    ui->comboBox_vcc->addItems({ " ", "3.3 V", "1.8 V", "5.0 V"});

    ui->comboBox_type->addItem("SPI_FLASH", 0);
    ui->comboBox_type->addItem("24_EEPROM", 1);
    ui->comboBox_type->addItem("93_EEPROM", 2);

    ui->comboBox_addr4bit->addItem("No", 0);
    ui->comboBox_addr4bit->addItem("Yes", 1);

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
    // connect and status check
    statusCH341 = ch341a_spi_init();
    ch341StatusFlashing();
    chipData.resize(256);
    for (int i=0; i < 256; i++)
    {
        chipData[i] = char(0xff);
    }
    ch341a_spi_shutdown();
    hexEdit = new QHexEdit(ui->frame);
    hexEdit->setGeometry(0,0,ui->frame->width(),ui->frame->height());
    hexEdit->setData(chipData);
    //opening chip database file
    ui->statusBar->showMessage("Opening DAT file");
    QFile datfile("SNANDer_GUI.Dat");
    QByteArray dataChips;
    if (!datfile.open(QIODevice::ReadOnly))
    {
        QMessageBox::about(this, "Error", "Error loading chip database file!");
        return;
    }
    dataChips = datfile.readAll();
    datfile.close();
    //parsing dat file
    ui->statusBar->showMessage("Parsing DAT file");
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

}

MainWindow::~MainWindow()
{
    delete ui;
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

void MainWindow::on_pushButton_clicked()
{
  //Reading data from chip
  int res = 0;
  if (currentChipType!=1) statusCH341 = ch341a_spi_init();
  else statusCH341 = ch341a_init_i2c();
  if (statusCH341 == 0)
  {
    if (((currentNumBlocks > 0) && (currentBlockSize >0) && (currentChipType == 0)) || ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 1)) || ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 2)))
    {

       if (currentChipType == 1)
       {
           config_stream(2);
           currentBlockSize = 128;
           //currentBlockSize = currentPageSize;
           currentNumBlocks = currentChipSize / currentBlockSize;
       }
       if (currentChipType == 2)
       {
           //org = (currentAlgorithm & 0xF0) / 16;
           config_stream(1);
           res = mw_gpio_init();
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
       ui->pushButton->setStyleSheet("QPushButton{color:#fff;background-color:#f66;border-radius: 20px;border: 2px solid #094065;border-radius:8px;font-weight:600;}");
       ui->statusBar->showMessage("Reading data from " + ui->comboBox_name->currentText());
       for (k = 0; k < currentNumBlocks; k++)
       {
          if (currentChipType == 0) res = snor_read_param(buf,curBlock * currentBlockSize, currentBlockSize, currentBlockSize, currentAddr4bit);
          if (currentChipType == 1)
          {
            qDebug() <<"buf-"<<curBlock * currentBlockSize<<"-"<<currentBlockSize<<"-"<<currentChipSize<<"-"<<currentPageSize<<"-"<<currentAlgorithm;
            //res = ch341readEEPROM_param(buf, curBlock * currentBlockSize, currentBlockSize, currentChipSize, currentBlockSize, 1);
            res = ch341readEEPROM_param(buf, curBlock * currentBlockSize, currentBlockSize, currentChipSize, currentPageSize, currentAlgorithm);//currentAlgorithm);
            qDebug() << res;
            if (res==0) res = 1;
          }
          if (currentChipType == 2)
          {
             res = Read_EEPROM_3wire_param(buf, static_cast<int>(curBlock * currentBlockSize), static_cast<int>(currentBlockSize), static_cast<int>(currentChipSize), currentAlgorithm);
             if (res==0) res = 1;
          }
          // if res=-1 - error, stop
          if (statusCH341 != 0)
            {
                QMessageBox::about(this, "Error", "Programmer CH341a is not connected!");
                break;
            }
          if (res == 0)
            {
               QMessageBox::about(this, "Error", "Error reading block " + QString::number(curBlock));
               break;
            }
         for (j = 0; j < currentBlockSize; j++)
            {
                  chipData[addr + j] = char(buf[addr + j - k * currentBlockSize]);
            }
          addr = addr + currentBlockSize;
          curBlock++;
          if (curBlock * currentBlockSize < 413300) hexEdit->setData(chipData); //show buffer in hehedit while chip data is being loaded
          ui->progressBar->setValue(static_cast<int>(curBlock));
       }
    }
    else
    {
       //Not correct Number fnd size of blocks
       if (currentChipType == 0) QMessageBox::about(this, "Error", "Before reading from chip please press 'Detect' button.");
       if (currentChipType == 1) QMessageBox::about(this, "Error", "Please select the chip parameters.");
    }
    hexEdit->setData(chipData);
    ui->statusBar->showMessage("");
    ui->progressBar->setValue(0);
    ui->pushButton->setStyleSheet("QPushButton{color:#fff;background-color:rgb(120, 183, 140);border-radius: 20px;border: 2px solid #094065;border-radius:8px;font-weight:600;}");
    ch341a_spi_shutdown();
  }
  else
  {
      ch341StatusFlashing();
      QMessageBox::about(this, "Error", "Programmer CH341a is not connected!");
  }
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

void MainWindow::on_pushButton_2_clicked()
{
    //searching the connected chip in database
    statusCH341 = ch341a_spi_init();
    ch341StatusFlashing();
    if (statusCH341 != 0)
      {
        QMessageBox::about(this, "Error", "Programmer CH341a is not connected!");
        return;
      }

    ui->pushButton_2->setStyleSheet("QPushButton {color:#fff;background-color:#f66;border-radius: 20px;border: 2px solid #094065;border-radius:8px;font-weight:600;}");
    int i, index;
    // print JEDEC info
    //struct JEDEC listJEDEC;
    //ui->sizeEdit->setText(QString::number(ch341SpiCapacity()));
    //listJEDEC = GetJedecId();
    unsigned char bufid[5]={0xff,0xff,0xff,0xff,0xff};
    snor_read_devid(bufid, 5);
    if ((bufid[0] == 0xff) && (bufid[1] == 0xff) && (bufid[2] == 0xff))
    {
        QMessageBox::about(this, "Error", "The chip is not connect or missing!");
        ui->pushButton_2->setStyleSheet("QPushButton{color:#fff;background-color:rgb(120, 183, 140);border-radius: 20px;border: 2px solid #094065;border-radius:8px;font-weight:600;}");
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

            ui->pushButton_2->setStyleSheet("QPushButton{color:#fff;background-color:rgb(120, 183, 140);border-radius: 20px;border: 2px solid #094065;border-radius:8px;font-weight:600;}");
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
    for (uint32_t i=0; i < currentChipSize; i++)
    {
        chipData[i] = char(0xff);
    }
    hexEdit->setData(chipData);
    }
    if ((currentChipSize !=0) && (currentPageSize!=0)  && (currentChipType == 1))
    {
    currentNumBlocks = currentChipSize / currentPageSize;
    chipData.resize(static_cast<int>(currentChipSize));
    for (uint32_t i=0; i < currentChipSize; i++)
    {
        chipData[i] = char(0xff);
    }
    hexEdit->setData(chipData);
    }
    ui->pushButton_2->setStyleSheet("QPushButton{color:#fff;background-color:rgb(120, 183, 140);border-radius: 20px;border: 2px solid #094065;border-radius:8px;font-weight:600;}");
    ch341a_spi_shutdown();
}

void MainWindow::on_comboBox_size_currentIndexChanged(int index)
{
    //qDebug() <<"size="<< ui->comboBox_size->currentData().toInt() << " block_size=" << ui->comboBox_page->currentData().toInt();
    currentChipSize = ui->comboBox_size->currentData().toUInt();
    currentBlockSize = ui->comboBox_block->currentData().toUInt();
    currentPageSize = ui->comboBox_page->currentData().toUInt();
    currentAddr4bit = ui->comboBox_addr4bit->currentData().toUInt();
    if ((currentChipSize !=0) && (currentBlockSize!=0) && (currentChipType == 0))
    {
        currentNumBlocks = currentChipSize / currentBlockSize;
        qDebug()<<"blocks="<<currentNumBlocks;
        chipData.resize(static_cast<int>(currentChipSize));
        for (uint32_t i=0; i < currentChipSize; i++)
        {
            chipData[i] = char(0xff);
        }
        hexEdit->setData(chipData);
    }
    if ((currentChipSize !=0) && (currentPageSize!=0)  && (currentChipType > 0))
    {
    currentNumBlocks = currentChipSize / currentPageSize;
    chipData.resize(static_cast<int>(currentChipSize));
    for (uint32_t i=0; i < currentChipSize; i++)
    {
        chipData[i] = char(0xff);
    }
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
        //qDebug()<<"blocks="<<currentNumBlocks;
        chipData.resize(static_cast<int>(currentChipSize));
        for (uint32_t i=0; i < currentChipSize; i++)
        {
            chipData[i] = char(0xff);
        }
        hexEdit->setData(chipData);
    }
    if ((currentChipSize !=0) && (currentPageSize!=0)  && (currentChipType > 0))
    {
    currentNumBlocks = currentChipSize / currentPageSize;
    chipData.resize(static_cast<int>(currentChipSize));
    for (uint32_t i=0; i < currentChipSize; i++)
    {
        chipData[i] = char(0xff);
    }
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

    ui->statusBar->showMessage("Saving file");
    fileName = QFileDialog::getSaveFileName(this,
                                QString::fromUtf8("Save file"),
                                QDir::currentPath(),
                                "Data Images (*.bin);;All files (*.*)");
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::about(this, "Error", "Error saving file!");
        return;
    }
    file.write(hexEdit->data());
    file.close();
}

void MainWindow::on_actionErase_triggered()
{
    statusCH341 = ch341a_spi_init();
    ch341StatusFlashing();
    if (statusCH341 != 0)
      {
        QMessageBox::about(this, "Error", "Programmer CH341a is not connected!");
        return;
      }
    ui->statusBar->showMessage("Erasing the " + ui->comboBox_name->currentText());
    ui->checkBox->setStyleSheet("QCheckBox{font-weight:600;}");
    ui->centralWidget->repaint();
    ui->progressBar->setRange(0, 100);
    if (currentChipType == 0)
    {
       ui->progressBar->setValue(50);
       //int snor_erase_param(unsigned long offs, unsigned long len, unsigned int sector_size, unsigned int n_sectors);
       snor_erase_param(0, 65536, 65536, 1);
       sleep(1);
    }
    if (currentChipType == 2)
    {
        config_stream(1);
        mw_gpio_init();
        ui->progressBar->setValue(50);
        mw_eeprom_erase(0, currentChipSize);
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
        ui->progressBar->setRange(0, static_cast<int>(currentNumBlocks));
        for (k = 0; k < currentBlockSize; k++)
        {
            buf[k] = 0xff;
        }
        for (curBlock = 0; curBlock < currentNumBlocks; curBlock++)
        {
            res = ch341writeEEPROM_param(buf, curBlock * 128, 128, currentPageSize, currentAlgorithm);
            ui->progressBar->setValue( static_cast<int>(curBlock));
            if (res != 0)
              {
                QMessageBox::about(this, "Error", "Error erasing sector " + QString::number(curBlock));
                ch341a_spi_shutdown();
                return;
              }
        }

    }
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
    ui->statusBar->showMessage("Opening file");
    fileName = QFileDialog::getOpenFileName(this,
                                QString::fromUtf8("Open file"),
                                QDir::currentPath(),
                                "Data Images (*.bin);;All files (*.*)");
    ui->statusBar->showMessage("Current file: " + fileName);
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {

        return;
    }
    chipData = file.readAll();
    hexEdit->setData(chipData);
    file.close();
    ui->statusBar->showMessage("");
}

void MainWindow::on_actionWrite_triggered()
{
    //Writting data to chip
    int res = 0;
    if (((currentNumBlocks > 0) && (currentBlockSize >0) && (currentChipType == 0)) || ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 1)) || ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 2)))
        {
        if (currentChipType!=1) statusCH341 = ch341a_spi_init();
        else statusCH341 = ch341a_init_i2c();
        if (currentChipType == 1)
        {
            config_stream(2);
            currentBlockSize = 128;
            //currentBlockSize = currentPageSize;
            currentNumBlocks = currentChipSize / currentBlockSize;
        }
        if (currentChipType == 2)
        {
            //org = (currentAlgorithm & 0xF0) / 16;
            config_stream(1);
            res = mw_gpio_init();
            currentBlockSize = currentPageSize;
            currentNumBlocks = currentChipSize / currentBlockSize;
        }
        ch341StatusFlashing();
    uint32_t addr = 0;
    uint32_t curBlock = 0;    
    uint32_t j, k;
    ui->statusBar->showMessage("Writing data to " + ui->comboBox_name->currentText());
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
         //int32_t ch341SpiWrite(uint8_t *buf, uint32_t add, uint32_t len)
         //res = ch341SpiWrite(buf, curBlock * currentBlockSize, currentBlockSize);
         //res = snor_read_param(buf,curBlock * currentBlockSize, currentBlockSize, currentBlockSize, currentAddr4bit);
         if (currentChipType == 0) res =  snor_write_param(buf, addr, currentBlockSize, currentBlockSize, currentAddr4bit);
         if (currentChipType == 1)
         {
              qDebug() <<"buf-"<<curBlock * currentBlockSize<<"-"<<currentBlockSize<<"-"<<currentChipSize<<"-"<<currentPageSize<<"-1";
              //res = ch341readEEPROM_param(buf, curBlock * currentBlockSize, currentBlockSize, currentChipSize, currentBlockSize, 1);
              //ch341writeEEPROM_param(buf, 0, 128, 256, 8, 1);
              res = ch341writeEEPROM_param(buf, curBlock * 128, 128, currentPageSize, currentAlgorithm);
              qDebug() << res;
              if (res==0) res = 1;
         }
         if (currentChipType == 2)
         {
               res = Write_EEPROM_3wire_param(buf, static_cast<int>(curBlock * currentBlockSize), static_cast<int>(currentBlockSize), static_cast<int>(currentChipSize), currentAlgorithm);
               if (res==0) res = 1;
         }
         // if res=-1 - error, stop
         if (statusCH341 != 0)
           {
             QMessageBox::about(this, "Error", "Programmer CH341a is not connected!");
             break;
           }
         if (res == 0)
           {
             QMessageBox::about(this, "Error", "Error writing sector " + QString::number(curBlock));
             break;
           }
         addr = addr + currentBlockSize;
         curBlock++;
         ui->progressBar->setValue( static_cast<int>(curBlock));

      }
    }
    else
    {
    //Not correct Number fnd size of blocks
     QMessageBox::about(this, "Error", "Before reading from chip please press 'Detect' button.");
    }
    ui->progressBar->setValue(0);
    ui->checkBox_2->setStyleSheet("");
    ui->statusBar->showMessage("");
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
           if (txt.compare(chips[i].chipManuf)==0)
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
           qDebug()<<"blocks="<<currentNumBlocks;
           chipData.resize(static_cast<int>(currentChipSize));
           for (uint32_t i=0; i < currentChipSize; i++)
           {
               chipData[i] = char(0xff);
           }
           hexEdit->setData(chipData);
       }
       if ((currentChipSize !=0) && (currentPageSize!=0)  && (currentChipType > 0))
       {
           currentNumBlocks = currentChipSize / currentPageSize;
           chipData.resize(static_cast<int>(currentChipSize));
           for (uint32_t i=0; i < currentChipSize; i++)
           {
               chipData[i] = char(0xff);
           }
           hexEdit->setData(chipData);
       }

    }
}

void MainWindow::on_actionVerify_triggered()
{
    //Reading and veryfying data from chip
    int res =0;
    if (currentChipType!=1) statusCH341 = ch341a_spi_init();
    else statusCH341 = ch341a_init_i2c();
    if (statusCH341 == 0)
    {
       if (((currentNumBlocks > 0) && (currentBlockSize >0) && (currentChipType == 0)) || ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 1)) || ((currentNumBlocks > 0) && (currentPageSize >0) && (currentChipType == 2)))
           {
               if (currentChipType == 1)
               {
                 config_stream(2);
                 currentBlockSize = 128;
                 currentNumBlocks = currentChipSize / currentBlockSize;
               }
               if (currentChipType == 2)
               {
                  //org = (currentAlgorithm & 0xF0) / 16;
                  config_stream(1);
                  res = mw_gpio_init();
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
               ui->statusBar->showMessage("Veryfing data from " + ui->comboBox_name->currentText());
               for (k = 0; k < currentNumBlocks; k++)
               {
                   //int32_t ch341SpiRead(uint8_t *buf, uint32_t add, uint32_t len);
                   // res = ch341SpiRead(buf, curBlock * currentBlockSize, currentBlockSize);
                    if (currentChipType == 0) res = snor_read_param(buf,curBlock * currentBlockSize, currentBlockSize, currentBlockSize, currentAddr4bit);
                    if (currentChipType == 1)
                    {
                       qDebug() <<"buf-"<<curBlock * currentBlockSize<<"-"<<currentBlockSize<<"-"<<currentChipSize<<"-"<<currentBlockSize<<"-1";
                       //res = ch341readEEPROM_param(buf, curBlock * currentBlockSize, currentBlockSize, currentChipSize, currentBlockSize, 1);
                       res = ch341readEEPROM_param(buf, curBlock * currentBlockSize, currentBlockSize, currentChipSize, currentPageSize, currentAlgorithm);
                       qDebug() << res;
                       if (res == 0) res = 1;
                     }
                    if (currentChipType == 2)
                    {
                        res = Read_EEPROM_3wire_param(buf, static_cast<int>(curBlock * currentBlockSize), static_cast<int>(currentBlockSize), static_cast<int>(currentChipSize), currentAlgorithm);
                        if (res == 0) res = 1;
                    }
                    // if res=-1 - error, stop
                    if (statusCH341 != 0)
                    {
                       QMessageBox::about(this, "Error", "Programmer CH341a is not connected!");
                       break;
                    }
                    if (res == 0)
                    {
                        QMessageBox::about(this, "Error", "Error reading block " + QString::number(curBlock));
                        break;
                    }
                    for (j = 0; j < currentBlockSize; j++)
                    {
                      if (chipData[addr + j] != char(buf[addr + j - k * currentBlockSize]))
                          {
                            //error compare
                            QMessageBox::about(this, "Error", "Error comparing data!\nAddress:   " + hexiAddr(addr + j) + "\nBuffer: " + bytePrint( static_cast<unsigned char>(chipData[addr + j])) + "    Chip: " + bytePrint(buf[addr + j - k * currentBlockSize]));
                            ui->statusBar->showMessage("");
                            ui->checkBox_3->setStyleSheet("");
                            ch341a_spi_shutdown();
                            return;
                           }
                     }
                     addr = addr + currentBlockSize;
                     curBlock++;
                     ui->progressBar->setValue(static_cast<int>(curBlock));
                 }
             }
             else
             {
                //Not correct Number fnd size of blocks
               if (currentChipType == 0) QMessageBox::about(this, "Error", "Before reading from chip please press 'Detect' button.");
               if (currentChipType == 1) QMessageBox::about(this, "Error", "Please select the chip parameters.");

             }
             ui->statusBar->showMessage("");
             ui->progressBar->setValue(0);
             ui->checkBox_3->setStyleSheet("");
             ch341a_spi_shutdown();
    }
      else
      {
          ch341StatusFlashing();
          QMessageBox::about(this, "Error", "Programmer CH341a is not connected!");
      }
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

void MainWindow::on_pushButton_3_clicked()
{
    ui->pushButton_3->setStyleSheet("QPushButton{color:#fff;background-color:#f66;border-radius: 20px;border: 2px solid #094065;border-radius:8px;font-weight:600;}");
    if (ui->checkBox->isChecked()) MainWindow::on_actionErase_triggered();
    if (ui->checkBox_2->isChecked()) MainWindow::on_actionWrite_triggered();
    if (ui->checkBox_3->isChecked()) MainWindow::on_actionVerify_triggered();
    ui->pushButton_3->setStyleSheet("QPushButton{color:#fff;background-color:rgb(120, 183, 140);border-radius: 20px;border: 2px solid #094065;border-radius:8px;font-weight:600;}");
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
            QMessageBox::about(this, "Error", "The end address must be greater than the starting addres.");
            return;
        }
        blockLen = blockEndAddr - blockStartAddr + 1;
    }
    else blockLen = hexToInt(addressData.mid(e + 1, t - e - 2));
    //qDebug() << blockStartAddr << " " << blockEndAddr << " " << blockLen;
    block.resize(static_cast<int>(blockLen));
    chipData = hexEdit->data();
    for (ee = 0; ee < blockLen; ee++)
    {
        block[ee] = chipData[ee + blockStartAddr];
    }
    ui->statusBar->showMessage("Saving block");
    fileName = QFileDialog::getSaveFileName(this,
                                QString::fromUtf8("Save block"),
                                QDir::currentPath(),
                                "Data Images (*.bin);;All files (*.*)");
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::about(this, "Error", "Error saving file!");
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
    ui->statusBar->showMessage("Opening block");
    fileName = QFileDialog::getOpenFileName(this,
                                QString::fromUtf8("Open block"),
                                QDir::currentPath(),
                                "Data Images (*.bin);;All files (*.*)");
    ui->statusBar->showMessage("Current file: " + fileName);
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
        QMessageBox::about(this, "Error", "The end address out of image size!");
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
        ui->eStatus->setText("Connected");
        ui->eStatus -> setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 5px;color:#000;background:#9f0;font-weight:600;border-style:inset;}");
    }
    else
    {
        ui->eStatus->setText("Not connected");
        ui->eStatus -> setStyleSheet("QLineEdit {border: 2px solid gray;border-radius: 5px;color:#fff;background:#f00;font-weight:600;border-style:inset;}");
    }
}

void MainWindow::on_pushButton_4_clicked()
{
    //int32_t ch341writeEEPROM_param(uint8_t *buffer, uint32_t offset, uint32_t bytesum, uint32_t ic_size, uint32_t block_size, uint8_t algorithm)
    org = 1;
    int res = 0, start_addr=0;
    uint32_t j;
    uint8_t alg = 0;
    statusCH341 = ch341a_init_i2c();
    config_stream(1);
    res = mw_gpio_init();
     qDebug()<<"init="<<res;
    i2c_init(); //crash int mw_eeprom_read(unsigned char *buf, unsigned long from, unsigned long len)
    ch341StatusFlashing();
    uint8_t *buf;
    buf = (uint8_t *)malloc(256);
    for (j = 0; j < 256; j++)
       {
             buf[j] = 0xff;

       }
    //res = mw_eeprom_read(buf, 0, 128);
    //int Read_EEPROM_3wire_param(unsigned char *buffer, int start_addr, int block_size, int size_eeprom, uint8_t algorithm)
    start_addr = 16;
    alg = 0x07;
   res = Read_EEPROM_3wire_param(buf,start_addr, 16,128, alg);
    for (j = 0; j < 256; j++)
       {
             chipData[j+start_addr] = buf[j];
             qDebug() << buf[j];
       }



    qDebug() << "res="<< res;
                        //buf,start_addr,len,ic_size,block_size,algorithm
    //ch341writeEEPROM_param(uint8_t *buffer, uint32_t offset, uint32_t bytesum, uint32_t ic_size, uint32_t block_size, uint8_t algorithm)




    hexEdit->setData(chipData);

    ch341a_spi_shutdown();
    qDebug()<<"sh";
}

void MainWindow::on_comboBox_type_currentIndexChanged(int index)
{
    int i, index2;
    ui->comboBox_man->clear();
    ui->comboBox_name->clear();
    ui->comboBox_man->addItem("");
    ui->comboBox_name->addItem("");
    currentChipType = static_cast<uint8_t>(index);
    qDebug() << "ic_type=" << currentChipType;
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
     }
     if (index == 0)
     {
         ui->pushButton_2->show();
         ui->comboBox_block->show();
         ui->comboBox_addr4bit->show();
         ui->label_8->show();
         ui->label_9->show();
     }
}

void MainWindow::on_actionAbout_triggered()
{
    DialogAbout* aboutDialog = new DialogAbout();
    aboutDialog->show();
}
