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
#ifndef DIALOGSR_H
#define DIALOGSR_H

#include <QDialog>
#include <QMessageBox>
extern "C" {
#include "ch341a_spi.h"
#include "spi_controller.h"
}
namespace Ui {
class DialogSR;
}

class DialogSR : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSR(QWidget *parent = nullptr);
    void setChipType(const uint chipType);
    void closeEvent(QCloseEvent* event);
    uint currentChipType;
    ~DialogSR();

private slots:
    void on_pushButton_read_clicked();
    void on_pushButton_write_clicked();

signals:
    void closeRequestHasArrived();

private:
    Ui::DialogSR *ui;
    void setLineEditFilter();
    bool regReaded;
};

#endif // DIALOGSR_H
