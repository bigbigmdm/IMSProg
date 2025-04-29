/*
* QHexEdit is a Hex Editor Widget for the Qt Framework
* Copyright (C) 2010-2025 Winfried Simon
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, see
* https://www.gnu.org/licenses/
*/
#include "color_manager.h"

#include <QApplication>
#include <QStyleFactory>
#include <QtGui>
#include <QtCore>

ColoredArea::ColoredArea() 
{
    _posStart = 0;
    _posEnd = 0;
    _fontColor = QPen(QColor::fromRgba(qRgba(0, 0, 0, 0)));
    _areaStyle = QColor::fromRgba(qRgba(0, 0, 0, 0));
}

ColoredArea::ColoredArea(QPen pen, QBrush background)
{
    _posStart = 0;
    _posEnd = 0;
    _fontColor = pen;
    _areaStyle = background;
}

ColoredArea::ColoredArea(qint64 posStart, qint64 posEnd, QPen pen, QBrush background)
{
    _posStart = posStart;
    _posEnd = posEnd;
    _fontColor = pen;
    _areaStyle = background;
}

QPen ColoredArea::fontPen()
{
    return _fontColor;
}

QColor ColoredArea::fontColor()
{
    return _fontColor.color();
}

void ColoredArea::setFontColor(QColor color)
{
    _fontColor = QPen(color);
}

QColor ColoredArea::areaColor()
{
    return _areaStyle.color();
}

QBrush ColoredArea::areaStyle()
{
    return _areaStyle;
}

void ColoredArea::setAreaColor(QColor color)
{
    _areaStyle.setColor(color);
}

void ColoredArea::setAreaStyle(QBrush backround)
{
    _areaStyle = backround;
}

qint64 ColoredArea::posStart()
{
    return _posStart;
};

qint64 ColoredArea::posEnd()
{
    return _posEnd;
};

void ColoredArea::setRange(qint64 posStart, qint64 posEnd)
{
    _posStart = posStart;
    _posEnd = posEnd;
};

void ColoredArea::clear() 
{
    _posStart = 0;
    _posEnd = 0;
}


/********************************************* */

ColorManager::ColorManager()
{
    QPalette palette = qApp->palette();
    setPalette(palette);
};

void ColorManager::setPalette(const QPalette &palette)
{
    _selection = ColoredArea(palette.highlightedText().color(), palette.highlight());
    _highlighting = ColoredArea(QColor::fromRgb(0, 0, 0), QColor(0xff, 0xff, 0x99));
    _address = ColoredArea(palette.windowText().color(), palette.alternateBase());
    _hex = ColoredArea(palette.windowText().color(), palette.base());
    _ascii = ColoredArea(palette.windowText().color(), palette.alternateBase());
}

// read only, copy of relevant ColoredArea is returned: you can't change anything
ColoredArea ColorManager::markedArea(qint64 pos, Area area, Chunks *chunks)
{
    // prio 1 selection
    if (pos >= _selection.posStart() && pos < _selection.posEnd())
    {
        return _selection;
    }
    // prio 2 highlighting (changed data)
    if (chunks->dataChanged(pos))
    {
        return _highlighting;
    }
    // prio 3 user defined areas
    foreach (ColoredArea area, _userAreas)
    {
        if (pos >= area.posStart() && pos < area.posEnd())
        {
            return area;
        }
    }
    // nothing found -> standard colors
    return this->notMarked(area);
};

ColoredArea& ColorManager::notMarked(Area area)
{
    switch (area) {
        case Area::Address:
            return _address;
            break;
        case Area::Ascii:
            return _ascii;
            break;
        case Area::Hex:
            return _hex;
            break;
    }
    return _hex; // should never happen
}

ColoredArea& ColorManager::selection()
{
    return _selection;
};

ColoredArea& ColorManager::highlighting()
{
    return _highlighting;
};

void ColorManager::addUserArea(qint64 posStart, qint64 posEnd, QColor fontColor, QBrush areaStyle)
{
    ColoredArea userArea = ColoredArea(posStart, posEnd, fontColor, areaStyle);
    _userAreas.append(userArea);
}

void ColorManager::clearUserAreas()
{
    _userAreas.clear();
}