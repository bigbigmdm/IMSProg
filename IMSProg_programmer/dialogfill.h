/*
 * Copyright (C) 2024 - 2026 Mikhail Medvedev <e-ink-reader@yandex.ru>
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
#ifndef DIALOGFILL_H
#define DIALOGFILL_H

#include <QDialog>
#include <QString>

namespace Ui {
class DialogFill;
}

class DialogFill : public QDialog
{
    Q_OBJECT

public:
    explicit DialogFill(QWidget *parent = nullptr);
    ~DialogFill();

private slots:
    void on_pushButton_clicked();

signals:
    void sendAddr4(QString);

private:
    Ui::DialogFill *ui;
    QString addrData;
};

#endif // DIALOGFILL_H
