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
#ifndef DIALOGBBM_H
#define DIALOGBBM_H

#include <QDialog>
#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>
#include "qhexedit.h"
extern "C" {
#include "ch341a_spi.h"
#include "spi_controller.h"
}

namespace Ui {
class DialogBBM;
}

class DialogBBM : public QDialog
{
    Q_OBJECT

public:
    explicit DialogBBM(QWidget *parent = nullptr);
    void getSectorSize(uint32_t sectorSize);
    void getBlockSize(uint32_t blockSize);
    void getTotalBlocks(uint32_t totalBlocks);
    void getSettings(uint8_t settings);
    void setDeviceType(const uint8_t pType);
    ~DialogBBM();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();

private:
    Ui::DialogBBM *ui;
    int maxBlock;
    uint32_t sectSize, blSize, totBlocks;
    uint8_t setParams;
    uint8_t programmerType;
    QString programmerName;

signals:
    void sendNandParam(uint8_t);
};

#endif // DIALOGBBM_H
