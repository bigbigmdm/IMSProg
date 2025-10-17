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
#include "dialogbbm.h"
#include "ui_dialogbbm.h"
#include "mainwindow.h"
#include "hexutility.h"
#include <QTabWidget>
#include <QTableWidgetItem>
#include <QDebug>

DialogBBM::DialogBBM(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogBBM)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window| Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    ui->tableWidgetBBM->setShowGrid(true);
    ui->tableWidgetScan->setShowGrid(true);
    ui->tableWidgetScan->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetScan->verticalHeader()->setVisible(false);
    ui->tableWidgetScan->setColumnCount(4);
    ui->tableWidgetScan->setRowCount(1);
    ui->progressBar->setRange(0, 1024);
    ui->progressBar->setValue(0);
}

DialogBBM::~DialogBBM()
{
    delete ui;
}

void DialogBBM::on_pushButton_clicked()
{
    bool scanResult = false;
    int i, stCH341 = 0, retval, curSectNo, bbmCount;
    uint32_t sectInBlock;
    uint8_t tmp_hi, tmp_lo;
    QString col_block, col_start, col_end;
    stCH341 = ch341a_spi_init();
    bbmCount = 0;
    sectInBlock = blSize / sectSize;
    ui->label_scan->clear();
    ui->progressBar->setRange(0, static_cast<int>(totBlocks));
    std::shared_ptr<uint8_t[]> buf(new uint8_t[2]);
    if (stCH341 == 0)
    {
        for (i = 0; i < static_cast<int>(totBlocks); i++)
        {
            curSectNo = i * static_cast<int>( sectInBlock );

            SPI_CONTROLLER_Chip_Select_Low();
            SPI_CONTROLLER_Write_One_Byte(0x13);
            SPI_CONTROLLER_Write_One_Byte(static_cast<uint8_t>((0xff0000 & curSectNo) >> 16));
            SPI_CONTROLLER_Write_One_Byte(static_cast<uint8_t>((0x00ff00 & curSectNo) >> 8));
            SPI_CONTROLLER_Write_One_Byte(static_cast<uint8_t>(0x0000ff & curSectNo));
            SPI_CONTROLLER_Chip_Select_High();
            //usleep(1000);
            nand_wait_ready(950);
            SPI_CONTROLLER_Chip_Select_Low();
            SPI_CONTROLLER_Write_One_Byte(0x03);
            SPI_CONTROLLER_Write_One_Byte(0x08); //high address
            SPI_CONTROLLER_Write_One_Byte(0x00); //low address
            SPI_CONTROLLER_Write_One_Byte(0x00); //dymmy byte
            retval = SPI_CONTROLLER_Read_NByte(buf.get(),2,SPI_CONTROLLER_SPEED_SINGLE);
            SPI_CONTROLLER_Chip_Select_High();
            if (buf[0] != 0xff)
            {                
                scanResult = true;
                tmp_hi = static_cast<uint8_t>( i >> 8 );
                tmp_lo = i & 0xff;
                col_block = bytePrint(tmp_hi) + bytePrint(tmp_lo);
                col_start = hexiAddr( static_cast<uint32_t>(i) * sectInBlock * sectSize);
                col_end = hexiAddr(static_cast<uint32_t>(i + 1) * sectInBlock * sectSize - 1);
                ui->tableWidgetScan->insertRow(bbmCount);
                ui->tableWidgetScan->setItem(bbmCount, 0, new QTableWidgetItem(QString::number(bbmCount + 1)));
                ui->tableWidgetScan->setItem(bbmCount, 1, new QTableWidgetItem(col_block));
                ui->tableWidgetScan->setItem(bbmCount, 2, new QTableWidgetItem(col_start));
                ui->tableWidgetScan->setItem(bbmCount, 3, new QTableWidgetItem(col_end));

                bbmCount++;

            }
            ui->progressBar->setValue(i);
            if (retval)
               {
                  QMessageBox::about(this, tr("Error"), tr("Error reading chip!"));
                  return;
               }
        }
        if (scanResult) ui->label_scan->setText(tr("Corrupted blocks found:"));
        else ui->label_scan->setText(tr("All blocks in the chip are good!"));
        ui->tableWidgetScan->resizeColumnsToContents();
        ui->tableWidgetScan->removeRow(bbmCount);
        ui->progressBar->setValue(0);
        scanResult = false;
        ch341a_spi_shutdown();
    }
    else QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
}


void DialogBBM::getBlockSize(uint32_t blockSize)
{
    blSize = blockSize;
}

void DialogBBM::getSectorSize(uint32_t sectorSize)
{
    sectSize = sectorSize;
}

void DialogBBM::getTotalBlocks(uint32_t totalBlocks)
{
    totBlocks = totalBlocks;
}

void DialogBBM::getSettings(uint8_t settings)
{
    setParams = settings;
    if ((setParams & 0x0f) == 0x01) ui->radioButton_w2->setChecked(true);
    if ((setParams & 0x0f) == 0x00) ui->radioButton_w1->setChecked(true);
    if ((setParams & 0xf0) == 0x10) ui->radioButton_e2->setChecked(true);
    if ((setParams & 0xf0) == 0x00) ui->radioButton_e1->setChecked(true);
}

void DialogBBM::on_pushButton_2_clicked()
{
    int i, stCH341 = 0, retval, maxTableRows;
    uint8_t stringResult;
    QString numBlock;
    std::shared_ptr<uint8_t[]> buf(new uint8_t[256]);
    stCH341 = ch341a_spi_init();
    if (stCH341 == 0)
    {
        SPI_CONTROLLER_Chip_Select_Low();
        SPI_CONTROLLER_Write_One_Byte(0xa5);
        retval = SPI_CONTROLLER_Read_NByte(buf.get(),1,SPI_CONTROLLER_SPEED_SINGLE);
        retval = SPI_CONTROLLER_Read_NByte(buf.get(),256,SPI_CONTROLLER_SPEED_SINGLE);
        if ((buf[0] != 0xff) && (buf[1] != 0xff))
        {
            ui->tableWidgetBBM->setShowGrid(true);
            ui->tableWidgetBBM->setShowGrid(true);
            ui->tableWidgetBBM->horizontalHeader()->setStretchLastSection(true);
            ui->tableWidgetBBM->verticalHeader()->setVisible(false);
            ui->tableWidgetBBM->setColumnCount(4);
            ui->tableWidgetBBM->setRowCount(1);

            if (totBlocks == 1024) maxTableRows = 20;
            else maxTableRows = 40;
            for (i = 0; i < maxTableRows; i++)
            {
                numBlock = bytePrint(buf[i * 4] & 0x3f) + bytePrint(buf[i * 4 +1]);
                ui->tableWidgetBBM->insertRow(i);
                ui->tableWidgetBBM->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
                ui->tableWidgetBBM->setItem(i, 1, new QTableWidgetItem(numBlock));
                numBlock = bytePrint(buf[i * 4 + 2]) + bytePrint(buf[i * 4 + 3]);
                ui->tableWidgetBBM->setItem(i, 2, new QTableWidgetItem(numBlock));
                stringResult = (buf[i * 4] & 0x3f) >> 6;
                if (stringResult == 0x00) ui->tableWidgetBBM->setItem(i, 3, new QTableWidgetItem(tr("Not used")));
                if (stringResult == 0x02) ui->tableWidgetBBM->setItem(i, 3, new QTableWidgetItem(tr("Active, valid")));
                if (stringResult == 0x03) ui->tableWidgetBBM->setItem(i, 3, new QTableWidgetItem(tr("Active, not valid")));
            }
            ui->tableWidgetBBM->removeRow(maxTableRows);
            ui->tableWidgetBBM->resizeColumnsToContents();
        }
        else ui->label_bbm_result->setText(tr("BBM table is not used in this chip."));
        ch341a_spi_shutdown();
    }
    else QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
}

void DialogBBM::on_pushButton_3_clicked()
{
    setParams = 0;
    if (ui->radioButton_e1->isChecked()) setParams = setParams & 0x0f;
    if (ui->radioButton_e2->isChecked()) setParams = setParams | 0x10;
    if (ui->radioButton_w1->isChecked()) setParams = setParams & 0xf0;
    if (ui->radioButton_w2->isChecked()) setParams = setParams | 0x01;
    emit sendNandParam(setParams);
    DialogBBM::close();
}
