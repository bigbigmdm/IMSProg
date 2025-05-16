/*
 * Copyright (C) 2024 - 2025 Mikhail Medvedev <e-ink-reader@yandex.ru>
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
#ifndef DIALOGCOMPARE_H
#define DIALOGCOMPARE_H

#include <QDialog>
#include <QResizeEvent>
#include "qhexedit.h"
#include "hexutility.h"

namespace Ui {
class DialogCompare;
}

class DialogCompare : public QDialog
{
    Q_OBJECT

private slots:
    void handleScroll1();
    void handleScroll2();

    void on_checkBox_stateChanged(int arg1);

public:
    explicit DialogCompare(QWidget *parent = nullptr);
    void showArrays(QByteArray *array1, QByteArray *array2, QString *name1, QString *name2);
    ~DialogCompare();

private:
    Ui::DialogCompare *ui;
    void resizeEvent(QResizeEvent* event);
    QHexEdit *hexEdit1, *hexEdit2;
    QByteArray data1, data2;
    //QByteArray arr1, arr2;
};

#endif // DIALOGCOMPARE_H
