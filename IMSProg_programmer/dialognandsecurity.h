/*
 * Copyright (C) 2024 - 2026 Mikhail Medvedev <e-ink-reader@yandex.ru>
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
#ifndef DIALOGNANDSECURITY_H
#define DIALOGNANDSECURITY_H
#include "qhexedit.h"
#include <QDialog>

namespace Ui {
class DialogNandSecurity;
}

class DialogNandSecurity : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNandSecurity(QWidget *parent = nullptr);
    void setAlgorithm(unsigned int currentAlg);
    void setSectorSize(uint32_t sectorSize);
    void setPath(QString lastPath);
    void setDeviceType(const uint8_t pType);
    void closeEvent(QCloseEvent* event);
    ~DialogNandSecurity();

private slots:
    void on_toolButton_read_clicked();
    void on_toolButton_write_clicked();
    void on_toolButton_open_clicked();
    void on_toolButton_save_clicked();

signals:
    void closeRequestHasArrived();

private:
    Ui::DialogNandSecurity *ui;
    QString curPath;
    uint32_t currentSector;
    uint8_t startSector, endSector;
    QByteArray regData;
    QHexEdit *hexEdit;
    uint8_t programmerType;
    QString programmerName;
    struct algSettings
    {
        uint8_t id;          // Algorithm number
        uint8_t  secStart;  // Start security sector number
        uint8_t  secEnd;    // End security sector number
    };
};

#endif // DIALOGNANDSECURITY_H
