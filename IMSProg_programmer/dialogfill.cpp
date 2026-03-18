#include "dialogfill.h"
#include "ui_dialogfill.h"
#include "mainwindow.h"
#include <QValidator>
#include <QRegExp>
#include <QDebug>
#include <QString>

DialogFill::DialogFill(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogFill)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window| Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    QRegExp reHex( "[A-Fa-f0-9]{1,8}" );
    QRegExp codeHex( "[A-Fa-f0-9]{1,2}" );
    QRegExpValidator *validator = new QRegExpValidator(reHex, this);
    QRegExpValidator *valCode = new QRegExpValidator(codeHex, this);
    ui->lineEditStart->setValidator(validator);
    ui->lineEditEnd->setValidator(validator);
    ui->lineEditCode->setValidator(valCode);
    ui->comboBox_end->addItem(tr("End address"), 0);
    ui->comboBox_end->addItem(tr("Length"), 1);
}

DialogFill::~DialogFill()
{
    delete ui;
}

void DialogFill::on_pushButton_clicked()
{
    QString typeOfEnd;
    if (ui->comboBox_end->currentData() == 0) typeOfEnd = "0";
    else typeOfEnd = "1";
    addrData = ui->lineEditStart->text() + "-" + ui->lineEditEnd->text() + "-" + ui->lineEditCode->text() + "-" + typeOfEnd;
//    if (ui->comboBox_end->currentData() == 0) addrData = QString(ui->lineEditStart->text() + "-" + ui->lineEditEnd->text() + "*");  //+code
//    else addrData = QString(ui->lineEditStart->text() + "-" + ui->lineEditEnd->text() + "#");

    emit sendAddr4(addrData);
    DialogFill::close();
}
