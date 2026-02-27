/*
 * Copyright (C) 2023 - 2026 Mikhail Medvedev <e-ink-reader@yandex.ru>
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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QTime>
#include <QTimer>
#include <QSettings>
#include <QResizeEvent>
#include <unistd.h>
#include "qhexedit.h"
#include "dialogsp.h"
#include "dialogabout.h"
#include "dialoginfo.h"
#include "dialogsfdp.h"
#include "dialognandsr.h"
#include "dialogsr.h"
#include "dialogcompare.h"
#include "searchdialog.h"
#include "hexutility.h"
extern "C" {
#include "bitbang_microwire.h"
#include "ch341a_gpio.h"
#include "ch341a_i2c.h"
#include "ch341a_spi.h"
#include "ch347.h"
#include "flashcmd_api.h"
#include "i2c_eeprom_api.h"
#include "mw_eeprom_api.h"
#include "res.h"
#include "snorcmd_api.h"
#include "spi_controller.h"
#include "spi_eeprom.h"
#include "spi_eeprom_api.h"
#include "timer.h"
#include "types.h"
#include "ch347eeprom.h"
}


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:

    void receiveAddr(QString);
    void receiveAddr2(QString);
    void receiveAddr3(qint64);
    void closeSFDP();
    void closeSR();
    void closeNandSR();
    void receiveNandStatus(uint8_t);

private slots:
    void progInit();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_comboBox_size_currentIndexChanged(int index);
    void on_comboBox_page_currentIndexChanged(int index);
    void on_actionDetect_triggered();
    void on_actionSave_triggered();
    void on_actionErase_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionOpen_triggered();
    void on_actionWrite_triggered();
    void on_actionRead_triggered();
    void on_actionExit_triggered();
    void on_comboBox_man_currentIndexChanged(int index);
    void on_comboBox_name_currentIndexChanged(const QString &arg1);
    void on_actionVerify_triggered();
    void on_pushButton_3_clicked();
    void on_actionSave_Part_triggered();
    void on_actionLoad_Part_triggered();
    void on_actionFind_Replace_triggered();
    void on_comboBox_type_currentIndexChanged(int index);
    void on_actionAbout_triggered();
    void on_actionChecksum_calculate_triggered();
    void on_actionEdit_chips_Database_triggered();
    void doNotDisturb();
    void doNotDisturbCancel();
    void on_actionStop_triggered();
    void on_pushButton_4_clicked();
    void on_actionChip_info_triggered();
    void on_comboBox_addr4bit_currentIndexChanged(int index);
    void on_actionExport_to_Intel_HEX_triggered();
    void on_actionImport_from_Intel_HEX_triggered();
    void on_actionExtract_from_ASUS_CAP_triggered();
    void resizeEvent(QResizeEvent* event);
    void slotTimerAlarm();
    void on_actionGoto_address_triggered();
    void on_comboBox_i2cSpeed_currentIndexChanged(int index);
    void on_actionSecurity_registers_triggered();
    void on_actionFill_test_image_triggered();
    void preparingToCompare(bool type);
    void on_actionCompare_files_triggered();
    void on_comboBox_ECC_currentIndexChanged(int index);
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionBad_block_management_triggered();
    void on_actionCH341A_B_v1_2_triggered();
    void on_actionCH341A_v1_7_triggered();
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent* event);
    void on_actionCH347T_triggered();

private:
    Ui::MainWindow *ui;
    QString grnKeyStyle, redKeyStyle;
    QString lastDirectory;
    int statusCH341;
    QByteArray chipData, oldChipData;
    uint32_t currentChipSize, currentNumBlocks, currentBlockSize, currentPageSize, currentECCsize;
    uint8_t currentAlgorithm, currentChipType, currentI2CBusSpeed;
    unsigned int currentAddr4bit;
    bool isHalted;
    bool filled;
    uint8_t numberOfReads;
    QTimer *timer;
    QVector <QString> chType = {"SPI_FLASH","25_EEPROM","93_EEPROM","24_EEPROM","95_EEPROM"};
    struct chip_data {
        QString chipManuf;
        QString chipTypeTxt;
        QString chipName;
        uint8_t chipJedecIDMan;
        uint8_t chipJedecIDDev;
        uint8_t chipJedecIDCap;
        uint32_t chipSize;
        uint16_t sectorSize;
        uint8_t chipTypeHex;
        uint8_t algorithmCode;
        int delay;
        uint8_t addr4bit;
        uint32_t blockSize;
        uint8_t eepromPages;
        QString chipVCC;
    };
    chip_data chips[2000];
    int max_rec;
    QString fileName, oldFileName, newFileName;
    bool cmdStarted;
    QHexEdit *hexEdit;
    void ch341StatusFlashing();
    QByteArray block;
    uint32_t blockStartAddr, blockLen;
    uint8_t nandSettings;
    uint8_t current_programmer;
};

#endif // MAINWINDOW_H
