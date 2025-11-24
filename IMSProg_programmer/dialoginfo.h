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
#ifndef DIALOGINFO_H
#define DIALOGINFO_H

#include <QDialog>

namespace Ui {
class DialogInfo;
}

class DialogInfo : public QDialog
{
    Q_OBJECT

public:
    explicit DialogInfo(QWidget *parent = nullptr);
    ~DialogInfo();
    void setChip(const uint chipType);
    void setProgrammer(const uint8_t progType);

private slots:
    void on_pushButton_clicked();

private:
    Ui::DialogInfo *ui;

    QPixmap *pix24;
    QPixmap *pix93;
    QPixmap *pix25;
    QPixmap *pix35;
    QPixmap *pix45;
    QPixmap *pix2518;
    QPixmap *pix3518;
    QPixmap *pixnone;
    QPixmap *pix24v7;
    QPixmap *pix25v718;
    QPixmap *pix25v733;
    QPixmap *pix93v17;
    QPixmap *pix45v17;
    QPixmap *pix35v718;
    QPixmap *pix35v733;

    uint8_t currentProg;

};

#endif // DIALOGINFO_H
