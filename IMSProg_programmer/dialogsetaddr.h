/*
 * Copyright (C) 2024 Mikhail Medvedev <e-ink-reader@yandex.ru>
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
#ifndef DIALOGSETADDR_H
#define DIALOGSETADDR_H

#include <QDialog>

namespace Ui {
class DialogSetAddr;
}

class DialogSetAddr : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSetAddr(QWidget *parent = nullptr);
    ~DialogSetAddr();

private slots:
    void on_pushButton_clicked();

signals:
    void sendAddr3(qint64);

private:
    Ui::DialogSetAddr *ui;
};

#endif // DIALOGSETADDR_H
