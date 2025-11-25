/*
 * Copyright (C) 2024 Mikhail Medvedev <e-ink-reader@yandex.ru>
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
#include "dialogsetaddr.h"
#include "ui_dialogsetaddr.h"
#include <QValidator>
#include <QRegularExpression>
#include <QString>

DialogSetAddr::DialogSetAddr(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSetAddr)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    QRegularExpression reHex( "[A-Fa-f0-9]{1,8}" );
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(reHex, this);
    ui->lineEditStart->setValidator(validator);
}

DialogSetAddr::~DialogSetAddr()
{
    delete ui;
}

void DialogSetAddr::on_pushButton_clicked()
{
   bool ok;
   QString str = ui->lineEditStart->text();
   qint64 inputAddr = str.toUInt(&ok, 16);
   emit sendAddr3(inputAddr);
   DialogSetAddr::close();
   //return inputAddr;
}
