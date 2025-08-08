/*
 * Copyright (C) 2023 - 2025 Mikhail Medvedev <e-ink-reader@yandex.ru>
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
#ifndef DIALOGNANDSR_H
#define DIALOGNANDSR_H

#include <QDialog>
#include <QMessageBox>
extern "C" {
#include "ch341a_spi.h"
#include "spi_controller.h"
}

namespace Ui {
class DialogNANDSr;
}

class DialogNANDSr : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNANDSr(QWidget *parent = nullptr);
    ~DialogNANDSr();
    void setPattern(const uint pattern);
    void closeEvent(QCloseEvent* event);

signals:
    void closeRequestHasArrived();

private slots:
    void on_pushButton_read_clicked();

    void on_pushButton_write_clicked();

private:
    Ui::DialogNANDSr *ui;
    void setLineEditFilter();
    void allRegEnabled();
    void setRegDisabled(uint8_t regNumber);
    void clearAllFields();
    void setRegLabels(uint8_t regNumber, QString lt);
    uint8_t RegNumbers[5];
    bool regReaded;
};

#endif // DIALOGNANDSR_H
