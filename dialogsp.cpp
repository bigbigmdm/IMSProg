/*
 * Copyright (C) 2023 Mikhail Medvedev <e-ink-reader@yandex.ru>
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
#include "dialogsp.h"
#include "ui_dialogsp.h"
#include "mainwindow.h"
#include <QValidator>
#include <QRegExp>
#include <QDebug>
#include <QString>
DialogSP::DialogSP(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSP)
{
    ui->setupUi(this);
    ui->comboBox_end->addItem("End address", 0);
    ui->comboBox_end->addItem("Lenght", 1);
    QRegExp reHex( "[A-Fa-f0-9]{1,8}" );
    QRegExpValidator *validator = new QRegExpValidator(reHex, this);
    ui->lineEditStart->setValidator(validator);
    ui->lineEditEnd->setValidator(validator);
}

DialogSP::~DialogSP()
{
    delete ui;
}

void DialogSP::on_pushButton_clicked()
{

    if (ui->comboBox_end->currentData() == 0) addrData = QString(ui->lineEditStart->text() + "-" + ui->lineEditEnd->text() + "*");
    else addrData = QString(ui->lineEditStart->text() + "-" + ui->lineEditEnd->text() + "#");
    emit sendAddr(addrData);
    DialogSP::close();
}
