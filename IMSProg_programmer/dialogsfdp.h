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
#ifndef DIALOGSFDP_H
#define DIALOGSFDP_H

#include <QDialog>
#include <QMessageBox>
#include "hexutility.h"
extern "C" {
#include "ch341a_spi.h"
#include "spi_controller.h"
}
namespace Ui {
class DialogSFDP;
}

class DialogSFDP : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSFDP(QWidget *parent = nullptr);
    ~DialogSFDP();
    void closeEvent(QCloseEvent* event);

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void r1Disable();
    void r1Enable();
    void r2Disable();
    void r2Enable();

signals:
    void closeRequestHasArrived();

private:
    Ui::DialogSFDP *ui;
    void setLineEditFilter();
    void legendPrint(QString basic, QString extended, QString manufacture);
    int numOfRegisters;
};

#endif // DIALOGSFDP_H
