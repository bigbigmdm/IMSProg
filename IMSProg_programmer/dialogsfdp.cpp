#include "dialogsfdp.h"
#include "ui_dialogsfdp.h"

DialogSFDP::DialogSFDP(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSFDP)
{
    ui->setupUi(this);
}

DialogSFDP::~DialogSFDP()
{
    delete ui;
}

void DialogSFDP::on_pushButton_clicked()
{
    int stCH341 = 0;
    uint64_t sfdpSize = 0;
    uint32_t sfdpBlockSize = 0;
    bool sfdpSupport = false;
    unsigned char i, imax, twoAreaAddress=0xff, manufAreaAddress=0xff, twoAreaLen = 0xff;
    uint8_t *sfdpBuf;
    sfdpBuf = (uint8_t *)malloc(256);
    QString regData = "", VCCmin = "", VCCmax = "", speeds = "Single", addrTxt="";
    int retval = 0;
    stCH341 = ch341a_spi_init();
    ui->lineEdit_vcc_max->setText("");
    ui->lineEdit_vcc_min->setText("");
    ui->lineEdit_block->setText("");
    ui->lineEdit_size->setText("");
    ui->lineEdit_speeds->setText("");
    ui->lineEdit_otp->setText("");
    ui->label_9->setText("<html><head/><body><p>Legend:<br>** - Basic area</p><p>** - Extended area<br>** - Manufacture area </p></body></html>");
    if (stCH341 == 0)
    {
    //Transfer to ch341 0x5a
          SPI_CONTROLLER_Chip_Select_Low();
          SPI_CONTROLLER_Write_One_Byte(0x5a);
          SPI_CONTROLLER_Write_One_Byte(0x00);
          SPI_CONTROLLER_Write_One_Byte(0x00);
          //SPI_CONTROLLER_Write_One_Byte(i);
          SPI_CONTROLLER_Write_One_Byte(0x00);
          SPI_CONTROLLER_Write_One_Byte(0x00);
          retval = SPI_CONTROLLER_Read_NByte(sfdpBuf,256,SPI_CONTROLLER_SPEED_SINGLE);
          SPI_CONTROLLER_Chip_Select_High();
          if (retval)
          {
             QMessageBox::about(this, tr("Error"), tr("Error reading register!"));
             return;
          }

      //Analyse-parsing
      if ((sfdpBuf[0] == 0x53) && (sfdpBuf[1] == 0x46) && (sfdpBuf[2] == 0x44) && (sfdpBuf[3] == 0x50))
         {
             ui->lineEdit_sfdp->setText("Yes");
             sfdpSupport = true;
         }
      else
         {
             ui->lineEdit_sfdp->setText("No");
             sfdpSupport = false;
         }
      if (sfdpSupport)
         {
            if ((sfdpBuf[0x80] == 0x53) && (sfdpBuf[0x81] == 0x46) && (sfdpBuf[0x82] == 0x44) && (sfdpBuf[0x83] == 0x50)) imax = 127;
            else imax = 254;
            twoAreaAddress = sfdpBuf[0x0c];
            twoAreaLen = sfdpBuf[0x0b] * 4;
            manufAreaAddress = sfdpBuf[0x1c];
            if (manufAreaAddress != 0xff)
            {
               if ((sfdpBuf[manufAreaAddress] != 0xff) && (sfdpBuf[manufAreaAddress + 1] != 0xff))
               {
                   VCCmax = bP(sfdpBuf[manufAreaAddress + 1]) + bP(sfdpBuf[manufAreaAddress]);
                   VCCmin = bP(sfdpBuf[manufAreaAddress + 3]) + bP(sfdpBuf[manufAreaAddress + 2]);
                   VCCmax.insert(1, ".");
                   VCCmin.insert(1, ".");
                   ui->lineEdit_vcc_max->setText(VCCmax);
                   ui->lineEdit_vcc_min->setText(VCCmin);
               }
               if (sfdpBuf[manufAreaAddress + 9] != 0xff)
               {
                  if ((sfdpBuf[manufAreaAddress + 9] & 0x08) != 0) ui->lineEdit_otp->setText("Yes");
                  if ((sfdpBuf[manufAreaAddress + 9] & 0x08) == 0) ui->lineEdit_otp->setText("No");
               }
               else ui->lineEdit_otp->setText("");

            }
            else
            {
                ui->lineEdit_vcc_max->setText("");
                ui->lineEdit_vcc_min->setText("");
            }
            sfdpSize =( sfdpBuf[twoAreaAddress + 4] + sfdpBuf [twoAreaAddress +5] * 256 + sfdpBuf[twoAreaAddress +6] * 256 * 256 + sfdpBuf[twoAreaAddress + 7] * 256 * 256 * 256 + 1) /8 /1024 ;
            ui->lineEdit_size->setText(QString::number(sfdpSize) + " K");
            if (sfdpBuf[twoAreaAddress + 0x20] != 0xff)
            {
                sfdpBlockSize = (1 << sfdpBuf[twoAreaAddress + 0x20]) / 1024;
                ui->lineEdit_block->setText(QString::number(sfdpBlockSize) + " K");
            }
            else
            {
                ui->lineEdit_block->setText("");
            }
            if (sfdpBuf[twoAreaAddress + 0x0f] == 0xbb) speeds = speeds + "/Dual";
            if (sfdpBuf[twoAreaAddress + 0x09] == 0xeb) speeds = speeds + "/Quad";
            ui->lineEdit_speeds->setText(speeds);
            ui->label_9->setText(tr("<html><head/><body><p>Legend:</p><p>00 - Basic area<br><span style=\" background:#f77;\">") + bP(twoAreaAddress) + tr("</span> - Extended area<br><span style=\" background:#7f7;\">") + bP(manufAreaAddress) + tr("</span> - Manufacture area </p></body></html>"));
            //HEXDUMP
            regData = tr("<html><head/><body><p> Hex SFDP register data:\n");
            addrTxt = tr("<html><head/><body><p>Addr:<br>");
            for (i=0; i<232;i=i+16)
            {
                addrTxt = addrTxt + "0" + bP(i) + "><br>";
            }
            addrTxt = addrTxt + "0F0></p></body></html>";
            ui->label_10->setText(addrTxt);
            for (i=0;i<=imax;i++)
            {
               if (i % 16 == 0) regData = regData +  "<br> ";
               if (i == 0x0c) regData = regData + "<span style=\" background:#f77;\">";
               if (i == 0x0d) regData = regData + "</span>";
               if (i == 0x1c) regData = regData + "<span style=\" background:#7f7;\">";
               if (i == 0x1d) regData = regData + "</span>";
               if (i == twoAreaAddress) regData = regData + "<span style=\" background:#f77;\">";
               if (i == twoAreaAddress + twoAreaLen) regData = regData + "</span>";
               if (i == manufAreaAddress) regData = regData + "<span style=\" background:#7f7;\">";
               if (i == manufAreaAddress + 0x0f) regData = regData + "</span>";
               regData = regData + bP(sfdpBuf[i]) + " ";
            }
            regData = regData + "</p></body></html>";
            ui->label->setText(regData);

        }
      //READING STATUS REGISTERS
      SPI_CONTROLLER_Chip_Select_Low();
      SPI_CONTROLLER_Write_One_Byte(0x05);
      retval = SPI_CONTROLLER_Read_NByte(sfdpBuf,2,SPI_CONTROLLER_SPEED_SINGLE);
      SPI_CONTROLLER_Chip_Select_High();
      if (retval)
      {
         QMessageBox::about(this, tr("Error"), tr("Error reading register!"));
         return;
      }
      ui->lineEdit_sr07->setText(QString::number(((sfdpBuf[0] & 128) >> 7)));
      ui->lineEdit_sr06->setText(QString::number(((sfdpBuf[0] & 64) >> 6)));
      ui->lineEdit_sr05->setText(QString::number(((sfdpBuf[0] & 32) >> 5)));
      ui->lineEdit_sr04->setText(QString::number(((sfdpBuf[0] & 16) >> 4)));
      ui->lineEdit_sr03->setText(QString::number(((sfdpBuf[0] & 8) >> 3)));
      ui->lineEdit_sr02->setText(QString::number(((sfdpBuf[0] & 4) >> 2)));
      ui->lineEdit_sr01->setText(QString::number(((sfdpBuf[0] & 2) >> 1)));
      ui->lineEdit_sr00->setText(QString::number((sfdpBuf[0] & 1)));

      ui->lineEdit_sr17->setText(QString::number(((sfdpBuf[1] & 128) >> 7)));
      ui->lineEdit_sr16->setText(QString::number(((sfdpBuf[1] & 64) >> 6)));
      ui->lineEdit_sr15->setText(QString::number(((sfdpBuf[1] & 32) >> 5)));
      ui->lineEdit_sr14->setText(QString::number(((sfdpBuf[1] & 16) >> 4)));
      ui->lineEdit_sr13->setText(QString::number(((sfdpBuf[1] & 8) >> 3)));
      ui->lineEdit_sr12->setText(QString::number(((sfdpBuf[1] & 4) >> 2)));
      ui->lineEdit_sr11->setText(QString::number(((sfdpBuf[1] & 2) >> 1)));
      ui->lineEdit_sr10->setText(QString::number((sfdpBuf[1] & 1)));
      //ui->lineEdit_sr1->setText(bP(sfdpBuf[1]));

      SPI_CONTROLLER_Chip_Select_Low();
      SPI_CONTROLLER_Write_One_Byte(0x9f);
      retval = SPI_CONTROLLER_Read_NByte(sfdpBuf,3,SPI_CONTROLLER_SPEED_SINGLE);
      SPI_CONTROLLER_Chip_Select_High();
      ch341a_spi_shutdown();
      if (retval)
      {
         QMessageBox::about(this, tr("Error"), tr("Error reading JEDEC ID!"));
         return;
      }
      ui->lineEdit_jedec0->setText(bP(sfdpBuf[0]));
      ui->lineEdit_jedec1->setText(bP(sfdpBuf[1]));
      ui->lineEdit_jedec2->setText(bP(sfdpBuf[2]));
    }

    else QMessageBox::about(this, tr("Error"), tr("Programmer CH341a is not connected!"));

}
QString DialogSFDP::bP(unsigned char z)
{
    unsigned char s;
    s = z / 16;
    if (s > 0x9) s = s + 0x37;
    else s = s + 0x30;
    z = z % 16;
    if (z > 0x9) z = z + 0x37;
    else z = z + 0x30;
    return QString(s) + QString(z);
}

void DialogSFDP::on_pushButton_2_clicked()
{
    DialogSFDP::close();
}
