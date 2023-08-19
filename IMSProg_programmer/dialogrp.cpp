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
#include "dialogrp.h"
#include "ui_dialogrp.h"
#include "mainwindow.h"
#include <QValidator>
#include <QRegExp>
#include <QDebug>
#include <QString>
DialogRP::DialogRP(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRP)
{
    ui->setupUi(this);
    QRegExp reHex( "[A-Fa-f0-9]{1,8}" );
    QRegExpValidator *validator = new QRegExpValidator(reHex, this);
    ui->lineEditStart->setValidator(validator);
}

DialogRP::~DialogRP()
{
    delete ui;
}

void DialogRP::on_pushButton_clicked()
{

    addrData = QString(ui->lineEditStart->text());
    emit sendAddr2(addrData);
    DialogRP::close();
}
