#include "dialognandsecurity.h"
#include "ui_dialognandsecurity.h"
#include "mainwindow.h"
#include <QDebug>
#include <QByteArray>
#include <QFileInfo>
#include "qhexedit.h"

DialogNandSecurity::DialogNandSecurity(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNandSecurity)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window| Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
}

DialogNandSecurity::~DialogNandSecurity()
{
    delete ui;
}

void DialogNandSecurity::on_toolButton_read_clicked()
{
    int stCH341 = 0, retval, i;
    std::shared_ptr<uint8_t[]> buf(new uint8_t[4096]);
    uint8_t curRegister = static_cast<uint8_t>(ui->comboBox_regnum->currentData().toUInt()) + startSector - 1;
qDebug()<<"curregister="<<curRegister;
    bool otp;
    //READING OTP PAGE
    stCH341 = ch341a_spi_init();


    usleep(100);
    SPI_CONTROLLER_Chip_Select_Low(); //Reading status
    SPI_CONTROLLER_Write_One_Byte(0x0f);
    SPI_CONTROLLER_Write_One_Byte(0xb0);
    retval = SPI_CONTROLLER_Read_NByte(buf.get(),1,SPI_CONTROLLER_SPEED_SINGLE);
    SPI_CONTROLLER_Chip_Select_High();
    usleep(1);
    if (stCH341 == 0)
    {


        if ((buf[0] & 0x40) == 0) //OPT Disabled ?
        {
            otp = false;
            // Enable OTP MODE

            SPI_CONTROLLER_Chip_Select_Low();  //Write enable
            SPI_CONTROLLER_Write_One_Byte(0x06);
            SPI_CONTROLLER_Chip_Select_High();
            usleep(1);

            SPI_CONTROLLER_Chip_Select_Low();
            SPI_CONTROLLER_Write_One_Byte(0x1f);
            SPI_CONTROLLER_Write_One_Byte(0xb0);
            SPI_CONTROLLER_Write_One_Byte(buf[0] | 0x40); //&bf to clear
            SPI_CONTROLLER_Chip_Select_High();
            usleep(1);
        }
        else otp = true;

        SPI_CONTROLLER_Chip_Select_Low();
        SPI_CONTROLLER_Write_One_Byte(0x13);
        SPI_CONTROLLER_Write_One_Byte(0x00);
        SPI_CONTROLLER_Write_One_Byte(0x00);
        SPI_CONTROLLER_Write_One_Byte(curRegister);
        //SPI_CONTROLLER_Write_One_Byte(1); //for test ONFI page reading
        SPI_CONTROLLER_Chip_Select_High();
        usleep(1000);
        SPI_CONTROLLER_Chip_Select_Low();
        SPI_CONTROLLER_Write_One_Byte(0x03);
        SPI_CONTROLLER_Write_One_Byte(0x00);
        SPI_CONTROLLER_Write_One_Byte(0x00);
        SPI_CONTROLLER_Write_One_Byte(0x00);
        retval = SPI_CONTROLLER_Read_NByte(buf.get(),currentSector,SPI_CONTROLLER_SPEED_SINGLE);
        SPI_CONTROLLER_Chip_Select_High();
        if (retval)
           {
              QMessageBox::about(this, tr("Error"), tr("Error reading Parameter Page!"));
              return;
           }
        else
        {
            for (i = 0; i < currentSector; i++)
            {
                regData[i] = char(buf[i]);
            }
            hexEdit->setData(regData);
        }
        if (otp == false) //OPT Disabled
        {
            // Disable OTP MODE
            SPI_CONTROLLER_Chip_Select_Low(); //Reading status
            SPI_CONTROLLER_Write_One_Byte(0x0f);
            SPI_CONTROLLER_Write_One_Byte(0xb0);
            retval = SPI_CONTROLLER_Read_NByte(buf.get(),1,SPI_CONTROLLER_SPEED_SINGLE);
            SPI_CONTROLLER_Chip_Select_High();
            usleep(1);

            SPI_CONTROLLER_Chip_Select_Low();  //Write enable
            SPI_CONTROLLER_Write_One_Byte(0x06);
            SPI_CONTROLLER_Chip_Select_High();
            usleep(1);

            SPI_CONTROLLER_Chip_Select_Low();
            SPI_CONTROLLER_Write_One_Byte(0x1f);
            SPI_CONTROLLER_Write_One_Byte(0xb0);
            SPI_CONTROLLER_Write_One_Byte(buf[0] & 0xbf);
            SPI_CONTROLLER_Chip_Select_High();
            usleep(1);
        }
        SPI_CONTROLLER_Chip_Select_Low();  //Write disable
        SPI_CONTROLLER_Write_One_Byte(0x04);
        SPI_CONTROLLER_Chip_Select_High();
        usleep(1);

        ch341a_spi_shutdown();
    }
    else QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
}

void DialogNandSecurity::on_toolButton_write_clicked()
{
    int stCH341 = 0, retval, i;
    std::shared_ptr<uint8_t[]> buf(new uint8_t[4096]);
    uint8_t curRegister = static_cast<uint8_t>(ui->comboBox_regnum->currentData().toUInt()) + startSector - 1;
    bool otp;
    //READING OTP PAGE
    stCH341 = ch341a_spi_init();

    usleep(100);
    SPI_CONTROLLER_Chip_Select_Low(); //Reading status
    SPI_CONTROLLER_Write_One_Byte(0x0f);
    SPI_CONTROLLER_Write_One_Byte(0xb0);
    retval = SPI_CONTROLLER_Read_NByte(buf.get(),1,SPI_CONTROLLER_SPEED_SINGLE);
    SPI_CONTROLLER_Chip_Select_High();
    usleep(1);
    if (stCH341 == 0)
    {

        if ((buf[0] & 0x40) == 0) //OPT Disabled ?
        {
            otp = false;
            // Enable OTP MODE

            SPI_CONTROLLER_Chip_Select_Low();  //Write enable
            SPI_CONTROLLER_Write_One_Byte(0x06);
            SPI_CONTROLLER_Chip_Select_High();
            usleep(1);

            SPI_CONTROLLER_Chip_Select_Low();
            SPI_CONTROLLER_Write_One_Byte(0x1f);
            SPI_CONTROLLER_Write_One_Byte(0xb0);
            SPI_CONTROLLER_Write_One_Byte(buf[0] | 0x40); //&bf to clear
            SPI_CONTROLLER_Chip_Select_High();
            usleep(1);
        }
        else otp = true;

        SPI_CONTROLLER_Chip_Select_Low();  //Write enable
        SPI_CONTROLLER_Write_One_Byte(0x06);
        SPI_CONTROLLER_Chip_Select_High();
        usleep(200);

        regData = hexEdit->data();
        for (i = 0; i < static_cast<int>(currentSector); i++)
        {
             buf[i] = static_cast<uint8_t>(regData[i]);
        }
        SPI_CONTROLLER_Chip_Select_Low();  //From PC to buffer
        SPI_CONTROLLER_Write_One_Byte(0x02);
        SPI_CONTROLLER_Write_One_Byte(0x00);
        SPI_CONTROLLER_Write_One_Byte(0x00);
        SPI_CONTROLLER_Write_NByte( buf.get(), currentSector, SPI_CONTROLLER_SPEED_SINGLE );
        SPI_CONTROLLER_Chip_Select_High();
        usleep(1000);
        SPI_CONTROLLER_Chip_Select_Low(); //From buffer to OTP sector
        SPI_CONTROLLER_Write_One_Byte(0x10);
        SPI_CONTROLLER_Write_One_Byte(0x00);
        SPI_CONTROLLER_Write_One_Byte(0x00);
        SPI_CONTROLLER_Write_One_Byte(curRegister);
        SPI_CONTROLLER_Chip_Select_High();
        usleep(200);

        if (otp == false) //OPT Disabled
        {
            // Disable OTP MODE
            SPI_CONTROLLER_Chip_Select_Low(); //Reading status
            SPI_CONTROLLER_Write_One_Byte(0x0f);
            SPI_CONTROLLER_Write_One_Byte(0xb0);
            retval = SPI_CONTROLLER_Read_NByte(buf.get(),1,SPI_CONTROLLER_SPEED_SINGLE);
            SPI_CONTROLLER_Chip_Select_High();
            usleep(1);

            SPI_CONTROLLER_Chip_Select_Low();  //Write enable
            SPI_CONTROLLER_Write_One_Byte(0x06);
            SPI_CONTROLLER_Chip_Select_High();
            usleep(1);

            SPI_CONTROLLER_Chip_Select_Low();
            SPI_CONTROLLER_Write_One_Byte(0x1f);
            SPI_CONTROLLER_Write_One_Byte(0xb0);
            SPI_CONTROLLER_Write_One_Byte(buf[0] & 0xbf);
            SPI_CONTROLLER_Chip_Select_High();
            usleep(1);
        }
        SPI_CONTROLLER_Chip_Select_Low();  //Write disable
        SPI_CONTROLLER_Write_One_Byte(0x04);
        SPI_CONTROLLER_Chip_Select_High();
        usleep(1);

        ch341a_spi_shutdown();
    }
    else QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));
}

void DialogNandSecurity::on_toolButton_open_clicked()
{
    QByteArray buf;
    QString fileName;

        fileName = QFileDialog::getOpenFileName(this,
                                    QString(tr("Open file")),
                                    curPath,
                                    "Data Images (*.bin *.BIN *.rom *.ROM);;All files (*.*)");

    QFileInfo info(fileName);
    curPath = info.filePath();
    QFile file(fileName);
    if (info.size() > currentSector)
    {
      QMessageBox::about(this, tr("Error"), tr("The file size exceeds the security register size."));
      return;
    }
    if (!file.open(QIODevice::ReadOnly))
    {

        return;
    }
    buf.resize(static_cast<int>(info.size()));
    buf = file.readAll();

    regData.replace(0, static_cast<int>(info.size()), buf);
    hexEdit->setData(regData);

    file.close();
    // path must be transfered to mainwindow
}

void DialogNandSecurity::on_toolButton_save_clicked()
{
    QString fileName;
    fileName = QFileDialog::getSaveFileName(this,
                                QString(tr("Save file")),
                                curPath,
                                "Data Images (*.bin *.BIN);;All files (*.*)");
    if (fileName.isEmpty()) return;
    QFileInfo info(fileName);
    curPath = info.filePath();

    if (QString::compare(info.suffix(), "bin", Qt::CaseInsensitive)) fileName = fileName + ".bin";
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::about(this, tr("Error"), tr("Error saving file!"));
        return;
    }
    file.write(hexEdit->data());
    file.close();
    // path must be transfered to mainwindow
}

void DialogNandSecurity::closeEvent(QCloseEvent* event)
{
    emit closeRequestHasArrived();
    QWidget::closeEvent(event);
}

void DialogNandSecurity::setSectorSize(uint32_t sectorSize)
{
   currentSector = sectorSize;

   regData.reserve(4096);
   regData.resize(static_cast<int>(currentSector));
   regData.fill(char(0xff));

   QFont heFont;
   heFont = QFont("Monospace", 10);
   hexEdit = new QHexEdit(ui->frame);
   hexEdit->setGeometry(0, 0, ui->frame->width(), ui->frame->height());
   hexEdit->setData(regData);
   hexEdit->setHexCaps(true);
   hexEdit->setFont(heFont);
   hexEdit->setData(regData);
}

void DialogNandSecurity::setAlgorithm(unsigned int currentAlg)
{
   uint8_t numSector;
   int i;
   algSettings algSet[] = {
   //  id Start_sect Number
       {  0,   0x02,  0x0b },
       {  1,   0x00,  0x03 },
       {  2,   0x00,  0x03 },
       {  3,   0x02,  0x1f },
       {  4,   0x02,  0x0b },
       {  5,   0x02,  0x1d },
       {  6,   0x02,  0x1f },
       {  7,   0x02,  0x1f },
       {  8,   0x02,  0x0b },
       {  9,   0x02,  0x0b },
       { 10,   0x02,  0x1f },
       { 11,   0x00,  0x03 },
       { 12,   0x02,  0x05 },
       { 13,   0x00,  0x03 },
       { 14,   0x02,  0x0b },
       { 15,   0x02,  0x3f },
       { 16,   0x02,  0x3f },
       { 17,   0x00,  0x03 },
       { 18,   0x00,  0x03 },
     };
   startSector = algSet[currentAlg].secStart;
   endSector = algSet[currentAlg].secEnd;
   numSector = endSector - startSector + 1;
   for (i = 1; i <= numSector; i++)
   {
       ui->comboBox_regnum->addItem(QString::number(i), i);
   }
}

void DialogNandSecurity::setPath(QString lastPath)
{
    curPath = lastPath;
}

