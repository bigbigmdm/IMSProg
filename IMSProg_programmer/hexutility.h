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
#ifndef HEXUTILITY_H
#define HEXUTILITY_H

#include <QString>

QString getCRC32(const QByteArray &buf);
QString hexiAddr(uint32_t add);
QString printAddress(int address, int digits);
QString bytePrint(unsigned char z);
QString sizeConvert(int a);
uint32_t hexToInt(QString str);

#endif // HEXUTILITY_H
