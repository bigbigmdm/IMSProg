/*
 * Copyright (C) 2025 Mikhail Medvedev <e-ink-reader@yandex.ru>
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
#ifndef DIALOGSECURITY_H
#define DIALOGSECURITY_H

#include <QDialog>
#include "qhexedit.h"
extern "C" {
#include "ch341a_spi.h"
#include "spi_controller.h"
}

namespace Ui {
class DialogSecurity;
}

class DialogSecurity : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSecurity(QWidget *parent = nullptr);
    void setAlgorithm(uint8_t currentAlg);
    void setPath(QString lastPath);
    void closeEvent(QCloseEvent* event);
    ~DialogSecurity();

private slots:
    void on_toolButton_read_clicked();
    void on_toolButton_write_clicked();
    void on_toolButton_erase_clicked();
    void on_toolButton_open_clicked();
    void on_toolButton_save_clicked();

signals:
    void closeRequestHasArrived();

private:
    Ui::DialogSecurity *ui;
    struct algSettings
    {
        uint8_t id;          // Algirithm number
        bool     algType;    // 0 - default with 0x48, 0x42, 0x44 commands
                             // 1 - with OTP mode and standart write/read commands
        uint8_t  regNumber;  // Number of registers by chip (1...4)
        uint8_t  size;       // size*64 = size of register bytes;
        uint16_t rg0addr;    // register 0 address / enter OTP mode
        uint16_t rg1addr;    // register 1 address / exit OTP mode
        uint16_t rg2addr;    // register 2 address
        uint16_t rg3addr;    // register 3 address
        bool     allErase;   // 1 - zero code for erasing all registers, 0 - none;
        bool     a4byte;     // 1 - 32 bit addressing using, 0 - 24 bit address using
        uint8_t  curCommand; // Current command pattern
    };
    struct securCommands
    {
       uint8_t id;        // Pattern number
       uint8_t RDSCUR;    // Read security register command
       uint8_t WRSCUR;    // Write security register command
       uint8_t ERSCUR;    // Erase cecurity register command
       uint8_t ENSO;      // Enter secured OTP mode command
       uint8_t EXSO;      // Exit secured OTP mode command
       bool    DUMRD;     // Dummy Byte on read cycle 1 - enable, 0 - disable
    };
    uint8_t curAlg;
    securCommands comPattern;
    algSettings curSettings;
    securCommands curCommands;
    QByteArray regData;
    QHexEdit *hexEdit;
    QString curPath;
};

#endif // DIALOGSECURITY_H
