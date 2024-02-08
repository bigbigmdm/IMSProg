/*
 * This file is part of the IMSProg_Editor project.
 *
 * Copyright (C) 2023-2024 Mikhail Medvedev (e-ink-reader@yandex.ru)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * verbose functionality forked from https://github.com/bigbigmdm/EZP2019-EZP2025_chip_data_editor
 *
 */
#include "ezp_chip_editor.h"
#include "ui_ezp_chip_editor.h"
#include "delegates.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    }

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_actionOpen_triggered()
{
    QString fileName, benchmarkDataFile;
    char txtBuf[0x30];
    benchmarkDataFile = "/usr/share/imsprog/IMSProg.Dat";
    QFileInfo check_benchmarkDataFile(benchmarkDataFile);
    int i, j, recNo, dataPoz, dataSize, chipSize, blockSize, delay, rowCount;
    unsigned char chipSizeCode, chipID, manCode, tmpBuf;
    defaultPath = QDir::homePath() + "/.local/share/imsprog/";
    // if ~//.local/share/imsprog/ is not exists creating this folder
    if (!QDir(defaultPath).exists()) QDir().mkdir(defaultPath);
    // If IMSProg.dat does not exist in the /.local/share/imsprog/ folder, it will be copied to that folder
    if (check_benchmarkDataFile.exists()) QFile::copy(benchmarkDataFile, defaultPath + "/IMSProg.Dat");

    ui->statusBar->showMessage(tr("Open the file"));
    fileName = QFileDialog::getOpenFileName(this,
                                QString(tr("Open the file")),
                                defaultPath,
                                "Data Images (*.Dat);;All files (*.*)");
    ui->statusBar->showMessage(tr("Current file: ") + fileName);
    QFile file(fileName);
    QByteArray data;
    if (!file.open(QIODevice::ReadOnly)) return;
    data = file.readAll();
    file.close();
    dataPoz = 0;
    recNo = 0;
    if(ui->tableView->model() != nullptr)
    {     
       rowCount = ui->tableView->model()->rowCount();
       for (i=0; i<rowCount;i++)
       {
           ui->tableView->model()->removeRow(i);
       }
        QThread::sleep(2);
        ui->tableView->reset();
    }

    dataSize = data.length();
    ui->tableView->setShowGrid(true);   
    //Заголовки столбцов
    QStringList horizontalHeader;
    //horizontalHeader.append("No");
    horizontalHeader.append(tr("Type"));
    horizontalHeader.append(tr("Manufacture"));
    horizontalHeader.append(tr("IC Name"));
    horizontalHeader.append(tr("JEDEC ID"));
    horizontalHeader.append(tr("Size"));
    horizontalHeader.append(tr("Sector\nsize"));
    horizontalHeader.append(tr("Type\nHEX"));
    horizontalHeader.append(tr("Algo-\nrithm"));
    horizontalHeader.append(tr("Delay"));
    horizontalHeader.append(tr("4 bit\naddress"));
    horizontalHeader.append(tr("Block\nsize K"));
    horizontalHeader.append(tr("EEPROM\npages"));
    horizontalHeader.append(tr("VCC"));
    model->setHorizontalHeaderLabels(horizontalHeader);

    //parsing qbytearray
    QStringList verticalHeader;
    while (dataPoz < dataSize)
    {
        for (j=0; j<0x30; j++)
             {
                 txtBuf[j] = 0;
             }
        j = 0;
             while ((j < 0x10) && (data[recNo * 0x44 + j] != ',')) // ASCII data reading
             {
                 txtBuf[j] = data[recNo * 0x44 + j];
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
         while ((i < 0x20) && (data[recNo * 0x44 + j] != ',')) // ASCII data reading
         {
             txtBuf[i] = data[recNo * 0x44 + j];
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
             while ((i < 0x30) && (data[recNo * 0x44 + j] != '\0')) // ASCII data reading
             {
                 txtBuf[i] = data[recNo * 0x44 + j];
                 j++;
                 i++;
             }
             chips[recNo].chipName = QByteArray::fromRawData(txtBuf, 0x30);
             chipSizeCode = data[recNo * 0x44 + 0x30];
             chipID = data[recNo * 0x44 + 0x31];;
             manCode = data[recNo * 0x44 + 0x32];
             chips[recNo].chipJedecID = "0x" + bytePrint(manCode) + bytePrint(chipID) + bytePrint(chipSizeCode);
             tmpBuf = data[recNo * 0x44 + 0x34];
             chipSize = tmpBuf;
             tmpBuf = data[recNo * 0x44 + 0x35];
             chipSize = chipSize + tmpBuf * 256;
             tmpBuf = data[recNo * 0x44 + 0x36];
             chipSize = chipSize + tmpBuf * 256 * 256;
             tmpBuf = data[recNo * 0x44 + 0x37];
             chipSize = chipSize + tmpBuf * 256 * 256 * 256;
             chips[recNo].chipSize = sizeConvert(chipSize);
             tmpBuf = data[recNo * 0x44 + 0x38];
             blockSize = tmpBuf;
             tmpBuf = data[recNo * 0x44 + 0x39];
             blockSize = blockSize + tmpBuf * 256;
             chips[recNo].blockSize = QString::number(blockSize);
             tmpBuf = data[recNo * 0x44 + 0x3a];
             chips[recNo].chipTypeHex = "0x" + bytePrint(tmpBuf);
             tmpBuf = data[recNo * 0x44 + 0x3b];
             chips[recNo].algorithmCode = "0x" + bytePrint(tmpBuf);
             tmpBuf = data[recNo * 0x44 + 0x3c];
             delay = tmpBuf;
             tmpBuf = data[recNo * 0x44 + 0x3d];
             delay = delay + tmpBuf * 256;
             chips[recNo].delay = QString::number(delay);
             tmpBuf = data[recNo * 0x44 + 0x3e];
             chips[recNo].extend = "0x" + bytePrint(tmpBuf);
             tmpBuf = data[recNo * 0x44 + 0x40];
             chips[recNo].eeprom = "0x" + bytePrint(tmpBuf);
             tmpBuf = data[recNo * 0x44 + 0x42];
             chips[recNo].eepromPages = "0x" + bytePrint(tmpBuf);
             tmpBuf = data[recNo * 0x44 + 0x43];
             if (tmpBuf == 0x00) chips[recNo].chipVCC = "3.3 V";
             if (tmpBuf == 0x01) chips[recNo].chipVCC = "1.8 V";
             if (tmpBuf == 0x02) chips[recNo].chipVCC = "5.0 V";
             dataPoz = dataPoz + 0x44; //next record
             verticalHeader.append(QString::number(recNo));
             item = new QStandardItem(chips[recNo].chipTypeTxt);
             model->setItem(recNo, 0, item);
             if(chips[recNo].chipTypeTxt.compare("SPI_FLASH")==0) model->item(recNo, 0)->setBackground(QBrush(QColor(0xcc, 0xff, 0xff)));
             else if(chips[recNo].chipTypeTxt.compare("24_EEPROM")==0) model->item(recNo, 0)->setBackground(QBrush(QColor(0xff, 0xff, 0xcc)));
             else if(chips[recNo].chipTypeTxt.compare("93_EEPROM")==0) model->item(recNo, 0)->setBackground(QBrush(QColor(0xff, 0xcc, 0xcc)));
             else if(chips[recNo].chipTypeTxt.compare("95_EEPROM")==0) model->item(recNo, 0)->setBackground(QBrush(QColor(0xcc, 0xff, 0xcc)));
             else if(chips[recNo].chipTypeTxt.compare("25_EEPROM")==0) model->item(recNo, 0)->setBackground(QBrush(QColor(0xff, 0xcc, 0xff)));
             item = new QStandardItem(chips[recNo].chipManuf);
             model->setItem(recNo, 1, item);
             item = new QStandardItem(chips[recNo].chipName);
             model->setItem(recNo, 2, item);
             item = new QStandardItem(chips[recNo].chipJedecID);
             model->setItem(recNo, 3, item);
             item = new QStandardItem(chips[recNo].chipSize);
             model->setItem(recNo, 4, item);
             item = new QStandardItem(chips[recNo].blockSize);
             model->setItem(recNo, 5, item);
             item = new QStandardItem(chips[recNo].chipTypeHex);
             model->setItem(recNo, 6, item);
             item = new QStandardItem(chips[recNo].algorithmCode);
             model->setItem(recNo, 7, item);
             item = new QStandardItem(chips[recNo].delay);
             model->setItem(recNo, 8, item);
             item = new QStandardItem(chips[recNo].extend);
             model->setItem(recNo, 9, item);
             item = new QStandardItem(chips[recNo].eeprom);
             model->setItem(recNo, 10, item);
             item = new QStandardItem(chips[recNo].eepromPages);
             model->setItem(recNo, 11, item);
             item = new QStandardItem(chips[recNo].chipVCC);
             model->setItem(recNo, 12, item);
             recNo++;
    }
    //String headers
    model->setVerticalHeaderLabels(verticalHeader);
    ui->tableView->setStyleSheet("QTableView { border: none;"
                                     "selection-background-color: #8EDE21;"
                                     "}");
    ui->tableView->setModel(model);
    ui->tableView->resizeRowsToContents();
    ui->tableView->resizeColumnsToContents();
   //--combobox in firsrt colunm
    chTypeDelegate* delegateType = new chTypeDelegate(this);
    ui->tableView->setItemDelegateForColumn(0,delegateType);
   //--combobox-last-colunm
    chVCCDelegate* delegateVCC = new chVCCDelegate(this);
    ui->tableView->setItemDelegateForColumn(12,delegateVCC);
    //--combobox Pages
    chPagesDelegate* delegatePages = new chPagesDelegate(this);
    ui->tableView->setItemDelegateForColumn(11,delegatePages);
    //-combobox Block Size
    chBlSizeDelegate* delegateBlSize = new chBlSizeDelegate(this);
    ui->tableView->setItemDelegateForColumn(5,delegateBlSize);
    connect(ui->tableView->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(onDataChanged(const QModelIndex&, const QModelIndex&)));

}


void MainWindow::on_actionExit_triggered()
{

    QApplication::quit();
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
 void MainWindow::onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
 {

     if(ui->tableView->model() != nullptr)
     {
     QModelIndex ind;
     QString tmpStr;
     int row;
     row = ui->tableView->currentIndex().row();
     if (row >= 0)
       {
        ind =   model->index(row, 0, QModelIndex());
        tmpStr = ui->tableView->model()->data(ind).toString();
        if(tmpStr.compare("SPI_FLASH")==0) model->item(row, 0)->setBackground(QBrush(QColor(0xcc, 0xff, 0xff)));
        else if(tmpStr.compare("24_EEPROM")==0) model->item(row, 0)->setBackground(QBrush(QColor(0xff, 0xff, 0xcc)));
        else if(tmpStr.compare("93_EEPROM")==0) model->item(row, 0)->setBackground(QBrush(QColor(0xff, 0xcc, 0xcc)));
        else if(tmpStr.compare("95_EEPROM")==0) model->item(row, 0)->setBackground(QBrush(QColor(0xcc, 0xff, 0xcc)));
        else if(tmpStr.compare("25_EEPROM")==0) model->item(row, 0)->setBackground(QBrush(QColor(0xff, 0xcc, 0xff)));
        else  model->item(row, 0)->setBackground(QBrush(QColor(0xff, 0xff, 0xff)));
       }
     }
 }

void MainWindow::on_actionSave_triggered()
{
    //creating new QBytearray to saving
    uint32_t size;
    unsigned char A,B,C,D;
    QString fileName;
    QString tmpStr;
    QModelIndex index;
    int i, j, recNo, delitel;
    QByteArray toSave;
    int rowCount = 0;
    if(ui->tableView->model() != nullptr)
    {
       ui->tableView->update();
       rowCount = ui->tableView->model()->rowCount();
       toSave.resize(0x44 * rowCount);
       toSave.fill(0x00);
       for (recNo = 0; recNo < rowCount; recNo++)
       {
         for (int j=0; j<13; j++)
         {
             index = model->index(recNo, j, QModelIndex());
             tmpStr = ui->tableView->model()->data(index).toString();
             switch (j)
             {
                case 0:
                       {
                          chips[recNo].chipTypeTxt = tmpStr;
                          break;
                       }
                case 1:
                       {
                          chips[recNo].chipManuf = tmpStr;
                          break;
                       }
                case 2:
                       {
                          chips[recNo].chipName = tmpStr;
                          break;
                       }
                case 3:
                       {
                          chips[recNo].chipJedecID = tmpStr;
                          break;
                       }
                case 4:
                       {
                          chips[recNo].chipSize = tmpStr;
                          break;
                       }
                case 5:
                       {
                          chips[recNo].blockSize = tmpStr;
                          break;
                       }
                case 6:
                       {
                          chips[recNo].chipTypeHex = tmpStr;
                          break;
                       }
                case 7:
                       {
                          chips[recNo].algorithmCode = tmpStr;
                          break;
                       }
                case 8:
                       {
                          chips[recNo].delay = tmpStr;
                          break;
                       }
                case 9:
                       {
                          chips[recNo].extend = tmpStr;
                          break;
                       }
                case 10:
                       {
                          chips[recNo].eeprom = tmpStr;
                          break;
                       }
                case 11:
                       {
                          chips[recNo].eepromPages = tmpStr;
                          break;
                       }
                case 12:
                       {
                          chips[recNo].chipVCC = tmpStr;
                          break;
                       }
             }
       }
      }

       for (recNo = 0; recNo < rowCount; recNo++)
       {
           QByteArray ba = chips[recNo].chipTypeTxt.toLocal8Bit();
           for (i = 0; i < ba.size(); i++)
           {
               toSave[recNo * 0x44 + i] = ba[i];
           }
           toSave[recNo * 0x44 + i] = ',';
           i++;
           j = i;
           ba = chips[recNo].chipManuf.toLocal8Bit();
           for (i = 0; i < ba.size(); i++)
           {
               toSave[recNo * 0x44 + j] = ba[i];
               j++;
           }
           toSave[recNo * 0x44 + j] = ',';
           j++;
           ba = chips[recNo].chipName.toLocal8Bit();
           for (i = 0; i < ba.size(); i++)
           {
               toSave[recNo * 0x44 + j] = ba[i];
               j++;
           }
           //next bytes
           tmpStr = chips[recNo].chipJedecID;
           if ((tmpStr[0] == '0') && (tmpStr[1] == 'x'))
           {
           toSave[recNo * 0x44 + 0x30] = dualDigitToByte(tmpStr, 2);
           toSave[recNo * 0x44 + 0x31] = dualDigitToByte(tmpStr, 1);
           toSave[recNo * 0x44 + 0x32] = dualDigitToByte(tmpStr, 0);
           }
           toSave[recNo * 0x44 + 0x33] = 0x00;
           tmpStr = chips[recNo].chipSize;
           if (tmpStr[tmpStr.size() - 1] == 'B') delitel = 1;
           if (tmpStr[tmpStr.size() - 1] == 'K') delitel = 1024;
           if (tmpStr[tmpStr.size() - 1] == 'M') delitel = 1024 * 1024;
           tmpStr = tmpStr.mid(0, tmpStr.size() - 2);
           size = tmpStr.toInt() * delitel;
           A = size % 256;
           B = (size / 256) % 256;
           C = (size / 256 / 256) % 256;
           D = (size / 256 / 256 / 256) % 256;
           toSave[recNo * 0x44 + 0x34] = A;
           toSave[recNo * 0x44 + 0x35] = B;
           toSave[recNo * 0x44 + 0x36] = C;
           toSave[recNo * 0x44 + 0x37] = D;
           tmpStr = chips[recNo].blockSize;
           size = tmpStr.toInt();
           A = size % 256;
           B = (size / 256) % 256;
           toSave[recNo * 0x44 + 0x38] = A;
           toSave[recNo * 0x44 + 0x39] = B;
           tmpStr = chips[recNo].chipTypeHex;
           if ((tmpStr[0] == '0') && (tmpStr[1] == 'x'))
           {
           toSave[recNo * 0x44 + 0x3A] = dualDigitToByte(tmpStr, 0);
           }
           tmpStr = chips[recNo].algorithmCode;
           if ((tmpStr[0] == '0') && (tmpStr[1] == 'x'))
           {
           toSave[recNo * 0x44 + 0x3b] = dualDigitToByte(tmpStr, 0);
           }
           tmpStr = chips[recNo].delay;
           size = tmpStr.toInt();
           A = size % 256;
           B = (size / 256) % 256;
           toSave[recNo * 0x44 + 0x3c] = A;
           toSave[recNo * 0x44 + 0x3d] = B;
           tmpStr = chips[recNo].extend;
           if ((tmpStr[0] == '0') && (tmpStr[1] == 'x'))
           {
           toSave[recNo * 0x44 + 0x3E] = dualDigitToByte(tmpStr, 0);
           }
           toSave[recNo * 0x44 + 0x3f] = 0x00;
           tmpStr = chips[recNo].eeprom;
           if ((tmpStr[0] == '0') && (tmpStr[1] == 'x'))
           {
           toSave[recNo * 0x44 + 0x40] = dualDigitToByte(tmpStr, 0);
           }
           toSave[recNo * 0x44 + 0x41] = 0x00;
           tmpStr = chips[recNo].eepromPages;
           if ((tmpStr[0] == '0') && (tmpStr[1] == 'x'))
           {
           toSave[recNo * 0x44 + 0x42] = dualDigitToByte(tmpStr, 0);
           }
           tmpStr = chips[recNo].chipVCC;
           if (tmpStr[4] == 'V')
           {
               if(tmpStr.compare("3.3 V")==0)  toSave[recNo * 0x44 + 0x43] = 0x00;
               if(tmpStr.compare("5.0 V")==0)  toSave[recNo * 0x44 + 0x43] = 0x02;
               if(tmpStr.compare("1.8 V")==0)  toSave[recNo * 0x44 + 0x43] = 0x01;
           }
       }
       // 0x44 zero bytes
          for (i = 0; i < 0x44; i++ )
          {
              toSave[recNo * 0x44 + i] = 0x00;
          }

       //Saving Qbytearray to file
       ui->statusBar->showMessage("Saving file");
       fileName = QFileDialog::getSaveFileName(this,
                                   QString::fromUtf8("Saving file"),
                                   defaultPath,
                                   "Data Images (*.Dat);;All files (*.*)");
       ui->statusBar->showMessage("Current file: " + fileName);
       QFile file(fileName);
       if (!file.open(QIODevice::WriteOnly)) return;
       file.write(toSave);
       file.close();

    }
}
unsigned char MainWindow::dualDigitToByte(QString q, int poz)
{
    unsigned char buf, rez;
   if (poz < 3)
     {
        poz = poz *2;
        buf = q[poz + 2].toLatin1();
        if ((buf >= '0') && (buf <= '9')) buf = buf - 0x30;
        if ((buf >= 'A') && (buf <= 'F')) buf = buf - 0x37;
        if ((buf >= 'a') && (buf <= 'f')) buf = buf - 0x57;
        rez = buf * 0x10;
        buf = q[poz + 3].toLatin1();
        if ((buf >= '0') && (buf <= '9')) buf = buf - 0x30;
        if ((buf >= 'A') && (buf <= 'F')) buf = buf - 0x37;
        if ((buf >= 'a') && (buf <= 'f')) buf = buf - 0x57;
        rez = rez + buf;
       return rez;
     }
   else return 0;

}

void MainWindow::on_actionAdd_string_triggered()
{
    QModelIndex indFrom, indTo;
    QString tmpStr;
    int sel, decsel;
    if(ui->tableView->model() != nullptr)
    {
       QItemSelectionModel *select = ui->tableView->selectionModel();
       select->selectedRows();
       QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();
       if (selection.count() <=0)
       {
           QMessageBox::warning(this, tr("Warning"), tr("No string selected."));
           return;
       }
       // Multiple rows can be selected
       for(int i=0; i< selection.count(); i++)
       {
           QModelIndex index = selection.at(i);
           ui->tableView->model()->insertRow(index.row() - i +1);
           ui->tableView->update();
           sel = index.row() - i +1;
           decsel = sel -1;

           for (int j=0; j<13; j++)
           {
               indFrom = model->index(sel, j, QModelIndex());
               indTo =   model->index(decsel, j, QModelIndex());
               tmpStr = ui->tableView->model()->data(indTo).toString();
               ui->tableView->model()->setData(indFrom, tmpStr);
               ui->tableView->selectRow(decsel);
           }
       ui->tableView->update();
       ui->tableView->resizeRowsToContents();
       ui->tableView->resizeColumnsToContents();
       }
    }
}


void MainWindow::on_actionDelete_string_triggered()
{
  if(ui->tableView->model() != nullptr)
  {
    QItemSelectionModel *select = ui->tableView->selectionModel();
    select->selectedRows();
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();
    if (selection.count() <=0)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No string selected."));
        return;
    }
    // Multiple rows can be selected
    for(int i=0; i< selection.count(); i++)
    {
        QModelIndex index = selection.at(i);
        ui->tableView->model()->removeRow(index.row() - i);
        ui->tableView->update();
    }
    ui->tableView->selectRow(-1);
    ui->tableView->update();
  }
}

void MainWindow::on_actionMove_up_triggered()
{
    int sel, decsel;
    chip_data tmpRec;
  if(ui->tableView->model() != nullptr)
  {
    QModelIndex indFrom, indTo;
    QString tmpStr;
    QItemSelectionModel *select = ui->tableView->selectionModel();
    select->selectedRows();
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();
    if (selection.count() <=0)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No string selected."));
        return;
    }
    for(int i=0; i< selection.count(); i++)
    {
        QModelIndex index = selection.at(i);       
        sel = index.row();
        decsel = sel -1;
        if (index.row() > 0)
        {
          for (int j=0; j<13; j++)
          {
              indFrom = model->index(sel, j, QModelIndex());
              indTo =   model->index(decsel, j, QModelIndex());
              tmpStr = ui->tableView->model()->data(indTo).toString();
              ui->tableView->model()->setData(indTo, ui->tableView->model()->data(indFrom).toString());
              ui->tableView->model()->setData(indFrom, tmpStr);

              ui->tableView->selectRow(decsel);
          }
        }

    }
  }
}

void MainWindow::on_actionMove_down_triggered()
{
  if(ui->tableView->model() != nullptr)
  {
    QModelIndex indFrom, indTo;
    QString tmpStr;
    int sel, incsel, rowCount;
    chip_data tmpRec;
    QItemSelectionModel *select = ui->tableView->selectionModel();
    select->selectedRows();
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();
    if (selection.count() <=0)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No string selected."));
        return;
    }
    for(int i=0; i< selection.count(); i++)
    {
        QModelIndex index = selection.at(i);
        sel = index.row();
        rowCount = ui->tableView->model()->rowCount();
        incsel = sel + 1;
        if (index.row() < rowCount -1)
        {
          for (int j=0; j<13; j++)
          {

              indFrom = model->index(sel, j, QModelIndex());
              indTo =   model->index(incsel, j, QModelIndex());
              tmpStr = ui->tableView->model()->data(indTo).toString();
              ui->tableView->model()->setData(indTo, ui->tableView->model()->data(indFrom).toString());
              ui->tableView->model()->setData(indFrom, tmpStr);
              ui->tableView->selectRow(incsel);
          }          
        }

    }
  }
}

void MainWindow::on_actionExport_to_CSV_triggered()
{
    QString fileName;
    QString toCSV = csvHeader;
    int j;
    if(ui->tableView->model() != nullptr)
    {
        QModelIndex indFrom;
        QItemSelectionModel *select = ui->tableView->selectionModel();
        select->selectedRows();
        QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();
        if (selection.count() <=0)
        {
            QMessageBox::warning(this, tr("Warning"), tr("No string selected."));
            return;
        }
        select->selectedRows();
        // Multiple rows can be selected
            for(int i=0; i< selection.count(); i++)
            {
                QModelIndex index = selection.at(i);
                for ( j=0; j<13; j++)
                      {
                          indFrom = model->index(index.row(), j, QModelIndex());
                          if (j != 12) toCSV = toCSV + ui->tableView->model()->data(indFrom).toString() + ";";
                          else toCSV = toCSV + ui->tableView->model()->data(indFrom).toString();
                      }
                      toCSV = toCSV + "\n";
            }
            ui->tableView->selectRow(-1);
            ui->tableView->update();

    //Saving QString to file
    ui->statusBar->showMessage(tr("Saving file"));
    fileName = QFileDialog::getSaveFileName(this,
                                QString(tr("Save file")),
                                QDir::homePath(),
                                "Data Images (*.csv);;All files (*.*)");
    ui->statusBar->showMessage(tr("Current file: ") + fileName);
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) return;
    file.write(toCSV.toUtf8());
    file.close();
    }

}

void MainWindow::on_actionExport_to_CSV_2_triggered()
{
    QString fileName;
    QString toCSV = csvHeader;
    int i,j, rowCount;
    if(ui->tableView->model() != nullptr)
    {
        QModelIndex indFrom;

       rowCount = ui->tableView->model()->rowCount();
       if (rowCount > 0)
       {
          for (i=0; i < rowCount; i++)
          {
            for ( j=0; j<13; j++)
            {
              indFrom = model->index(i, j, QModelIndex());
              if (j != 12) toCSV = toCSV + ui->tableView->model()->data(indFrom).toString() + ";";
              else toCSV = toCSV + ui->tableView->model()->data(indFrom).toString();
            }
          toCSV = toCSV + "\n";
         }

    //Saving QString to file
    ui->statusBar->showMessage("Saving file");
    fileName = QFileDialog::getSaveFileName(this,
                                QString(tr("Open the file")),
                                QDir::homePath(),
                                "Comma-Separated Values (*.csv);;All files (*.*)");
       ui->statusBar->showMessage(tr("Current file: ") + fileName);
       QFile file(fileName);
       if (!file.open(QIODevice::WriteOnly)) return;
       file.write(toCSV.toUtf8());
       file.close();
      }
  }
}

void MainWindow::on_actionImport_from_CSV_triggered()
{
    if(ui->tableView->model() != nullptr)
    {
        QString fileName, fromCSV, tmpStr, separator, chType;
        QModelIndex indFrom;
        int j = 0, recNo, curPoz = -1, fromPoz;
        ui->statusBar->showMessage(tr("Opening file"));
        fileName = QFileDialog::getOpenFileName(this,
                                    QString(tr("Open the file")),
                                    QDir::homePath(),
                                    "Comma-Separated Values (*.csv);;All files (*.*)");
        ui->statusBar->showMessage(tr("Current file: ") + fileName);
        QFile file(fileName);
        QByteArray data;
        if (!file.open(QIODevice::ReadOnly)) return;
        data = file.readAll();
        file.close();
        fromCSV = QString(data);
        curPoz = fromCSV.indexOf(csvHeader);
        if (curPoz >= 0) separator = ";";
        else
        {
          curPoz = fromCSV.indexOf(csvHeader2);
          separator = ",";
        }
        if (curPoz >= 0) //header detected
        {
           fromCSV = fromCSV.mid(curPoz + csvHeader.length(), fromCSV.length() - curPoz);
           fromPoz = 0;
           while(1)
           {
           recNo = ui->tableView->model()->rowCount();
           ui->tableView->model()->insertRow(recNo);
           for (j = 0; j < 13; j++)
           {
           if (j == 12) curPoz = fromCSV.indexOf("\n", fromPoz);
           else curPoz = fromCSV.indexOf(separator, fromPoz);
           if (curPoz >= 0)
             {
               tmpStr = fromCSV.mid(fromPoz,curPoz - fromPoz);
               fromPoz = curPoz + 1;
             }
           if (j == 0)
           {
              chType = tmpStr;
              if (!correctChipTyte(tmpStr))
              {
                  QMessageBox::warning(this, tr("Warning"),tr("Invalid CSV data file format."));
                  return;
              }
           }
           //add string, ic_type->add cell
           if (j == 12)
           {

             if(chType.compare("SPI_FLASH")==0) model->item(recNo, 0)->setBackground(QBrush(QColor(0xcc, 0xff, 0xff)));
             else if(chType.compare("24_EEPROM")==0) model->item(recNo, 0)->setBackground(QBrush(QColor(0xff, 0xff, 0xcc)));
             else if(chType.compare("93_EEPROM")==0) model->item(recNo, 0)->setBackground(QBrush(QColor(0xff, 0xcc, 0xcc)));
             else if(chType.compare("95_EEPROM")==0) model->item(recNo, 0)->setBackground(QBrush(QColor(0xcc, 0xff, 0xcc)));
             else if(chType.compare("25_EEPROM")==0) model->item(recNo, 0)->setBackground(QBrush(QColor(0xff, 0xcc, 0xff)));
             else  model->item(recNo, 0)->setBackground(QBrush(QColor(0xff, 0xff, 0xff)));

           }

           indFrom = model->index(recNo, j, QModelIndex());
           ui->tableView->model()->setData(indFrom, tmpStr);
           if (tmpStr.length() + fromPoz >= fromCSV.length())
           {
               ui->tableView->resizeRowsToContents();
               ui->tableView->resizeColumnsToContents();
               return;
           }


        }
       }
      }
      else
      {
            QMessageBox::warning(this, tr("Warning"), tr("Invalid CSV header file format."));
            return;
      }
   }
}
bool MainWindow::correctChipTyte(QString str)
{
    bool ansver = false;
    for (QString elem : chType)
    {
      if(str.compare(elem)==0)
          {
          ansver = true;
          return ansver;
          }
    }
    return ansver;

}
