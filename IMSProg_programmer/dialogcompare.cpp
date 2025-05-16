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
#include "dialogcompare.h"
#include "ui_dialogcompare.h"
#include <QDebug>
#include <QWidget>
#include <QByteArray>
#include <QScrollBar>
#include <stddef.h>
#include <stdint.h>
#include "qhexedit.h"
#include "hexutility.h"

DialogCompare::DialogCompare(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCompare)
{
    ui->setupUi(this);
    QFont heFont;
    //heFont = QFont("Monospace", 10);
    heFont = QFont("DejaVu Sans Mono", 10);
    hexEdit1 = new QHexEdit(ui->frame_1);
    hexEdit2 = new QHexEdit(ui->frame_2);
    hexEdit1->setHexCaps(true);
    hexEdit2->setHexCaps(true);
    hexEdit1->setFont(heFont);
    hexEdit2->setFont(heFont);
    hexEdit1->setGeometry(0,0,ui->frame_1->width(),ui->frame_1->height());
    hexEdit2->setGeometry(0,0,ui->frame_2->width(),ui->frame_2->height());
    hexEdit1->setReadOnly(true);
    hexEdit2->setReadOnly(true);
    ui->checkBox->setChecked(false);
    QFontMetrics fm(hexEdit1->fontMetrics());
    int pixelsHigh =fm.horizontalAdvance("F");
    DialogCompare::resize(pixelsHigh * 122, DialogCompare::height());
    hexEdit1->setAsciiArea(false);
    hexEdit2->setAsciiArea(false);
    //ui->checkBox->setChecked(true);

    //Synchronise scrollbars
    connect(hexEdit1->verticalScrollBar(), &QScrollBar::valueChanged, this, &DialogCompare::handleScroll1);
    connect(hexEdit2->verticalScrollBar(), &QScrollBar::valueChanged, this, &DialogCompare::handleScroll2);


}

void DialogCompare::handleScroll2()
{
    long posStart = 0;

    posStart = hexEdit2->_bPosFirst;
    hexEdit1->setCursorPosition(posStart *2);
    hexEdit2->setCursorPosition(posStart *2);
    hexEdit1->verticalScrollBar()->setValue(hexEdit2->verticalScrollBar()->value());
}


void DialogCompare::handleScroll1()
{
  long posStart = 0, posEnd = 0;
  int i;

  posStart = hexEdit1->_bPosFirst;
  posEnd = hexEdit1->_bPosLast + 16;

  hexEdit1->clearUserAreas();
  hexEdit2->clearUserAreas();
  hexEdit2->setCursorPosition(posStart *2);
  hexEdit1->setCursorPosition(posStart *2);
  hexEdit2->verticalScrollBar()->setValue(hexEdit1->verticalScrollBar()->value());

  for (i = static_cast<int>(posStart); i <= posEnd; i++)
  {
      if (data1[i] != data2[i])
      {
          hexEdit1->addUserArea(i, i + 1, QColor(0, 0, 0, 255), QColor(255, 0, 0, 60));
          hexEdit2->addUserArea(i, i + 1, QColor(0, 0, 0, 255), QColor(255, 0, 0, 60));
      }
  }
}

DialogCompare::~DialogCompare()
{
    delete ui;
}

void DialogCompare::showArrays(QByteArray *array1, QByteArray *array2, QString *name1, QString *name2)
{
    data1 = *array1;
    data2 = *array2;

    hexEdit1->setData(data1);
    hexEdit2->setData(data2);

    ui->label_n1->setText(tr("Name: "));
    ui->label_n2->setText(tr("Name: "));
    ui->lineEdit_n1->setText(*name1);
    ui->lineEdit_n2->setText(*name2);
    ui->lineEdit_c1->setText(getCRC32(*array1));
    ui->lineEdit_c2->setText(getCRC32(*array2));

    handleScroll1();
}

void DialogCompare::resizeEvent(QResizeEvent* event)
{
   QWidget::resizeEvent(event);
   hexEdit1->setGeometry(0,0,ui->frame_1->width(),ui->frame_1->height());
   hexEdit2->setGeometry(0,0,ui->frame_2->width(),ui->frame_2->height());
   handleScroll1();
}



void DialogCompare::on_checkBox_stateChanged(int arg1)
{
    QFontMetrics fm(hexEdit1->fontMetrics());
    int pixelsHigh =fm.horizontalAdvance("F");
    if (arg1 == 0)
    {
        hexEdit1->setAsciiArea(false);
        hexEdit2->setAsciiArea(false);
        DialogCompare::resize(pixelsHigh * 122, DialogCompare::height());
    }
    else
    {
        hexEdit1->setAsciiArea(true);
        hexEdit2->setAsciiArea(true);
        DialogCompare::resize(pixelsHigh * 153, DialogCompare::height());
    }
}
