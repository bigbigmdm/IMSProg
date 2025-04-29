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
#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>

#include "qhexedit.h"
#include <algorithm>


// ********************************************************************** Constructor, destructor

QHexEdit::QHexEdit(QWidget *parent) : QAbstractScrollArea(parent)
    , _addressArea(true)
    , _addressWidth(4)
    , _asciiArea(true)
    , _bytesPerLine(16)
    , _hexCharsInLine(47)
    , _highlighting(true)
    , _overwriteMode(true)
    , _readOnly(false)
    , _hexCaps(false)
    , _dynamicBytesPerLine(false)
    , _editAreaIsAscii(false)
    , _chunks(new Chunks(this))
    , _cursorPosition(0)
    , _lastEventSize(0)
    , _undoStack(new UndoStack(_chunks, this))
    , _colorManager(new ColorManager())
{
#ifdef Q_OS_WIN32
    setFont(QFont("Courier", 10));
#else
    setFont(QFont("Monospace", 10));
#endif
    connect(&_cursorTimer, SIGNAL(timeout()), this, SLOT(updateCursor()));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(adjust()));
    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(adjust()));
    connect(_undoStack, SIGNAL(indexChanged(int)), this, SLOT(dataChangedPrivate(int)));

    _cursorTimer.setInterval(500);
    _cursorTimer.start();

    setAddressWidth(4);
    setAddressArea(true);
    setAsciiArea(true);
    setOverwriteMode(true);
    setHighlighting(true);
    setReadOnly(false);

    init();
}

QHexEdit::~QHexEdit()
{
}

// ********************************************************************** Properties

void QHexEdit::setAddressArea(bool addressArea)
{
    _addressArea = addressArea;
    adjust();
    setCursorPosition(_cursorPosition);
    viewport()->update();
}

bool QHexEdit::addressArea()
{
    return _addressArea;
}

void QHexEdit::setAddressOffset(qint64 addressOffset)
{
    _addressOffset = addressOffset;
    adjust();
    setCursorPosition(_cursorPosition);
    viewport()->update();
}

qint64 QHexEdit::addressOffset()
{
    return _addressOffset;
}

void QHexEdit::setAddressWidth(int addressWidth)
{
    _addressWidth = addressWidth;
    adjust();
    setCursorPosition(_cursorPosition);
    viewport()->update();
}

int QHexEdit::addressWidth()
{
    qint64 size = _chunks->size();
    int n = 1;
    if (size > Q_INT64_C(0x100000000)){ n += 8; size /= Q_INT64_C(0x100000000);}
    if (size > 0x10000){ n += 4; size /= 0x10000;}
    if (size > 0x100){ n += 2; size /= 0x100;}
    if (size > 0x10){ n += 1;}

    if (n > _addressWidth)
        return n;
    else
        return _addressWidth;
}

void QHexEdit::setAsciiArea(bool asciiArea)
{
    if (!asciiArea)
        _editAreaIsAscii = false;
    _asciiArea = asciiArea;
    adjust();
    setCursorPosition(_cursorPosition);
    viewport()->update();
}

bool QHexEdit::asciiArea()
{
    return _asciiArea;
}

void QHexEdit::setBytesPerLine(int count)
{
    _bytesPerLine = count;
    _hexCharsInLine = count * 3 - 1;

    adjust();
    setCursorPosition(_cursorPosition);
    viewport()->update();
}

int QHexEdit::bytesPerLine()
{
    return _bytesPerLine;
}

void QHexEdit::setCursorPosition(qint64 position)
{
    // 1. delete old cursor
    _blink = false;
    viewport()->update(_cursorRect);

    // 2. Check, if cursor in range?
    if (position > (_chunks->size() * 2 - 1))
        position = _chunks->size() * 2  - (_overwriteMode ? 1 : 0);

    if (position < 0)
        position = 0;

    // 3. Calc new position of cursor
    _bPosCurrent = position / 2;
    _pxCursorY = ((position / 2 - _bPosFirst) / _bytesPerLine + 1) * _pxCharHeight;
    int x = (position % (2 * _bytesPerLine));
    if (_editAreaIsAscii)
    {
        _pxCursorX = x / 2 * _pxCharWidth + _pxPosAsciiX;
        _cursorPosition = position & 0xFFFFFFFFFFFFFFFE;
    } else {
        _pxCursorX = (((x / 2) * 3) + (x % 2)) * _pxCharWidth + _pxPosHexX;
        _cursorPosition = position;
    }

    int pxOfsX = horizontalScrollBar()->value();
    if (_readOnly)
        _cursorRect = QRect(
            _pxCursorX - pxOfsX, 
            _pxCursorY - _pxCharHeight + _pxSelectionSub, 
            _pxCharWidth, 
            _pxCharHeight
        );
    else 
        if (_overwriteMode)
            _cursorRect = QRect(
                _pxCursorX - pxOfsX, 
                _pxCursorY + _pxCursorWidth, 
                _pxCharWidth, 
                _pxCursorWidth
            );
        else
            _cursorRect = QRect(
                _pxCursorX - pxOfsX, 
                _pxCursorY - _pxCharHeight + _pxSelectionSub, 
                _pxCursorWidth, 
                _pxCharHeight
            );

    // 4. Immediately draw new cursor
    _blink = true;
    viewport()->update(_cursorRect);
    emit currentAddressChanged(_bPosCurrent);
}

qint64 QHexEdit::cursorPosition(QPoint pos)
{
    // Calc cursor position depending on a graphical position
    qint64 result = -1;
    int posX = pos.x() + horizontalScrollBar()->value();
    int posY = pos.y() - 3;
    if ((posX >= _pxPosHexX) && (posX < (_pxPosHexX + (1 + _hexCharsInLine) * _pxCharWidth)))
    {
        _editAreaIsAscii = false;
        int x = (posX - _pxPosHexX) / _pxCharWidth;
        x = (x / 3) * 2 + x % 3;
        int y = (posY / _pxCharHeight) * 2 * _bytesPerLine;
        result = _bPosFirst * 2 + x + y;
    }
    else
        if (_asciiArea && (posX >= _pxPosAsciiX) && (posX < (_pxPosAsciiX + (1 + _bytesPerLine) * _pxCharWidth)))
        {
            _editAreaIsAscii = true;
            int x = 2 * (posX - _pxPosAsciiX) / _pxCharWidth;
            int y = (posY / _pxCharHeight) * 2 * _bytesPerLine;
            result = _bPosFirst * 2 + x + y;
        }
    return result;
}

qint64 QHexEdit::cursorPosition()
{
    return _cursorPosition;
}

void QHexEdit::setData(const QByteArray &ba)
{
    _data = ba;
    _bData.setData(_data);
    setData(_bData);
}

QByteArray QHexEdit::data()
{
    return _chunks->data(0, -1);
}

void QHexEdit::setHighlighting(bool highlighting)
{
    _highlighting = highlighting;
    viewport()->update();
}

bool QHexEdit::highlighting()
{
    return _highlighting;
}

void QHexEdit::setHighlightingColor(const QColor &color)
{
    _colorManager->highlighting().setAreaColor(color);
    viewport()->update();
}

QColor QHexEdit::highlightingColor()
{
    return _colorManager->highlighting().areaColor();
}

void QHexEdit::setOverwriteMode(bool overwriteMode)
{
    _overwriteMode = overwriteMode;
    emit overwriteModeChanged(overwriteMode);
}

bool QHexEdit::overwriteMode()
{
    return _overwriteMode;
}

bool QHexEdit::isReadOnly()
{
    return _readOnly;
}

void QHexEdit::setReadOnly(bool readOnly)
{
    _readOnly = readOnly;
}

void QHexEdit::setHexCaps(const bool isCaps)
{
    _hexCaps = isCaps;
    viewport()->update();
}

bool QHexEdit::hexCaps()
{
    return _hexCaps;
}

void QHexEdit::setDynamicBytesPerLine(const bool isDynamic)
{
    _dynamicBytesPerLine = isDynamic;
    resizeEvent(NULL);
}

bool QHexEdit::dynamicBytesPerLine()
{
    return _dynamicBytesPerLine;
}

// ********************************************************************** Access to data of qhexedit
bool QHexEdit::setData(QIODevice &iODevice)
{
    bool ok = _chunks->setIODevice(iODevice);
    init();
    return ok;
}

QByteArray QHexEdit::dataAt(qint64 pos, qint64 count)
{
    return _chunks->data(pos, count);
}

bool QHexEdit::write(QIODevice &iODevice, qint64 pos, qint64 count)
{
    return _chunks->write(iODevice, pos, count);
}

// ********************************************************************** Char handling
void QHexEdit::insert(qint64 index, char ch)
{
    _undoStack->insert(index, ch);
    refresh();
}

void QHexEdit::remove(qint64 index, qint64 len)
{
    _undoStack->removeAt(index, len);
    refresh();
}

void QHexEdit::replace(qint64 index, char ch)
{
    _undoStack->overwrite(index, ch);
    refresh();
}

// ********************************************************************** ByteArray handling
void QHexEdit::insert(qint64 pos, const QByteArray &ba)
{
    _undoStack->insert(pos, ba);
    refresh();
}

void QHexEdit::replace(qint64 pos, qint64 len, const QByteArray &ba)
{
    _undoStack->overwrite(pos, len, ba);
    refresh();
}

// ********************************************************************** User marking areas
void QHexEdit::addUserArea(qint64 posStart, qint64 posEnd, QColor fontColor, QBrush areaStyle)
{
    _colorManager->addUserArea(posStart, posEnd, fontColor, areaStyle);
    refresh();
}

void QHexEdit::clearUserAreas()
{
    _colorManager->clearUserAreas();
    refresh();
}

// ********************************************************************** Utility functions
void QHexEdit::ensureVisible()
{
    if (_cursorPosition < (_bPosFirst * 2))
        verticalScrollBar()->setValue((int)(_cursorPosition / 2 / _bytesPerLine));
    if (_cursorPosition > ((_bPosFirst + (_rowsShown - 1)*_bytesPerLine) * 2))
        verticalScrollBar()->setValue((int)(_cursorPosition / 2 / _bytesPerLine) - _rowsShown + 1);
    if (_pxCursorX < horizontalScrollBar()->value())
        horizontalScrollBar()->setValue(_pxCursorX);
    if ((_pxCursorX + _pxCharWidth) > (horizontalScrollBar()->value() + viewport()->width()))
        horizontalScrollBar()->setValue(_pxCursorX + _pxCharWidth - viewport()->width());
    viewport()->update();
}

qint64 QHexEdit::indexOf(const QByteArray &ba, qint64 from)
{
    qint64 pos = _chunks->indexOf(ba, from);
    if (pos > -1)
    {
        qint64 curPos = pos*2;
        setCursorPosition(curPos + ba.length()*2);
        resetSelection(curPos);
        setSelection(curPos + ba.length()*2);
        ensureVisible();
    }
    return pos;
}

bool QHexEdit::isModified()
{
    return _modified;
}

qint64 QHexEdit::lastIndexOf(const QByteArray &ba, qint64 from)
{
    qint64 pos = _chunks->lastIndexOf(ba, from);
    if (pos > -1)
    {
        qint64 curPos = pos*2;
        setCursorPosition(curPos - 1);
        resetSelection(curPos);
        setSelection(curPos + ba.length()*2);
        ensureVisible();
    }
    return pos;
}

void QHexEdit::redo()
{
    _undoStack->redo();
    setCursorPosition(_chunks->pos()*(_editAreaIsAscii ? 1 : 2));
    refresh();
}

QString QHexEdit::selectionToReadableString()
{
    QByteArray ba = _chunks->data(getSelectionBegin(), getSelectionEnd() - getSelectionBegin());
    return toReadable(ba);
}

QString QHexEdit::selectedData()
{
    QByteArray ba = _chunks->data(getSelectionBegin(), getSelectionEnd() - getSelectionBegin()).toHex();
    return ba;
}

void QHexEdit::setFont(const QFont &font)
{
    QFont theFont(font);
    theFont.setStyleHint(QFont::Monospace);
    QWidget::setFont(theFont);
    QFontMetrics metrics = fontMetrics();
#if QT_VERSION > QT_VERSION_CHECK(5, 11, 0)
    _pxCharWidth = metrics.horizontalAdvance(QLatin1Char('2'));
#else
    _pxCharWidth = metrics.width(QLatin1Char('2'));
#endif
    _pxCharHeight = metrics.height();
    _pxAreaMargin = _pxCharWidth / 2;
    _pxCursorWidth = _pxCharHeight / 7;
    _pxSelectionSub = _pxCharHeight / 5;
    viewport()->update();
}

QString QHexEdit::toReadableString()
{
    QByteArray ba = _chunks->data();
    return toReadable(ba);
}

void QHexEdit::undo()
{
    _undoStack->undo();
    setCursorPosition(_chunks->pos()*(_editAreaIsAscii ? 1 : 2));
    refresh();
}

// ********************************************************************** Handle events
void QHexEdit::keyPressEvent(QKeyEvent *event)
{
    // Cursor movements
    if (event->matches(QKeySequence::MoveToNextChar))
    {
        qint64 pos = _cursorPosition + 1;
        if (_editAreaIsAscii)
            pos += 1;
        setCursorPosition(pos);
        resetSelection(pos);
    }
    if (event->matches(QKeySequence::MoveToPreviousChar))
    {
        qint64 pos = _cursorPosition - 1;
        if (_editAreaIsAscii)
            pos -= 1;
        setCursorPosition(pos);
        resetSelection(pos);
    }
    if (event->matches(QKeySequence::MoveToEndOfLine))
    {
        qint64 pos = _cursorPosition - (_cursorPosition % (2 * _bytesPerLine)) + (2 * _bytesPerLine) - 1;
        setCursorPosition(pos);
        resetSelection(_cursorPosition);
    }
    if (event->matches(QKeySequence::MoveToStartOfLine))
    {
        qint64 pos = _cursorPosition - (_cursorPosition % (2 * _bytesPerLine));
        setCursorPosition(pos);
        resetSelection(_cursorPosition);
    }
    if (event->matches(QKeySequence::MoveToPreviousLine))
    {
        setCursorPosition(_cursorPosition - (2 * _bytesPerLine));
        resetSelection(_cursorPosition);
    }
    if (event->matches(QKeySequence::MoveToNextLine))
    {
        setCursorPosition(_cursorPosition + (2 * _bytesPerLine));
        resetSelection(_cursorPosition);
    }
    if (event->matches(QKeySequence::MoveToNextPage))
    {
        setCursorPosition(_cursorPosition + (((_rowsShown - 1) * 2 * _bytesPerLine)));
        resetSelection(_cursorPosition);
    }
    if (event->matches(QKeySequence::MoveToPreviousPage))
    {
        setCursorPosition(_cursorPosition - (((_rowsShown - 1) * 2 * _bytesPerLine)));
        resetSelection(_cursorPosition);
    }
    if (event->matches(QKeySequence::MoveToEndOfDocument))
    {
        setCursorPosition(_chunks->size() * 2 );
        resetSelection(_cursorPosition);
    }
    if (event->matches(QKeySequence::MoveToStartOfDocument))
    {
        setCursorPosition(0);
        resetSelection(_cursorPosition);
    }

    // Select commands
    if (event->matches(QKeySequence::SelectAll))
    {
        resetSelection(0);
        setSelection(2 * _chunks->size() + 1);
    }
    if (event->matches(QKeySequence::SelectNextChar))
    {
        qint64 pos = _cursorPosition + 1;
        if (_editAreaIsAscii)
            pos += 1;
        setCursorPosition(pos);
        setSelection(pos);
    }
    if (event->matches(QKeySequence::SelectPreviousChar))
    {
        qint64 pos = _cursorPosition - 1;
        if (_editAreaIsAscii)
            pos -= 1;
        setSelection(pos);
        setCursorPosition(pos);
    }
    if (event->matches(QKeySequence::SelectEndOfLine))
    {
        qint64 pos = _cursorPosition - (_cursorPosition % (2 * _bytesPerLine)) + (2 * _bytesPerLine) - 1;
        setCursorPosition(pos);
        setSelection(pos);
    }
    if (event->matches(QKeySequence::SelectStartOfLine))
    {
        qint64 pos = _cursorPosition - (_cursorPosition % (2 * _bytesPerLine));
        setCursorPosition(pos);
        setSelection(pos);
    }
    if (event->matches(QKeySequence::SelectPreviousLine))
    {
        qint64 pos = _cursorPosition - (2 * _bytesPerLine);
        setCursorPosition(pos);
        setSelection(pos);
    }
    if (event->matches(QKeySequence::SelectNextLine))
    {
        qint64 pos = _cursorPosition + (2 * _bytesPerLine);
        setCursorPosition(pos);
        setSelection(pos);
    }
    if (event->matches(QKeySequence::SelectNextPage))
    {
        qint64 pos = _cursorPosition + (((viewport()->height() / _pxCharHeight) - 1) * 2 * _bytesPerLine);
        setCursorPosition(pos);
        setSelection(pos);
    }
    if (event->matches(QKeySequence::SelectPreviousPage))
    {
        qint64 pos = _cursorPosition - (((viewport()->height() / _pxCharHeight) - 1) * 2 * _bytesPerLine);
        setCursorPosition(pos);
        setSelection(pos);
    }
    if (event->matches(QKeySequence::SelectEndOfDocument))
    {
        qint64 pos = _chunks->size() * 2;
        setCursorPosition(pos);
        setSelection(pos);
    }
    if (event->matches(QKeySequence::SelectStartOfDocument))
    {
        qint64 pos = 0;
        setCursorPosition(pos);
        setSelection(pos);
    }

    // Edit Commands
    if (!_readOnly)
    {
        /* Cut */
        if (event->matches(QKeySequence::Cut))
        {
            QByteArray ba = _chunks->data(getSelectionBegin(), getSelectionEnd() - getSelectionBegin()).toHex();
            for (qint64 idx = 32; idx < ba.size(); idx +=33)
                ba.insert(idx, "\n");
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(ba);
            if (_overwriteMode)
            {
                qint64 len = getSelectionEnd() - getSelectionBegin();
                replace(getSelectionBegin(), (int)len, QByteArray((int)len, char(0)));
            }
            else
            {
                remove(getSelectionBegin(), getSelectionEnd() - getSelectionBegin());
            }
            setCursorPosition(2 * getSelectionBegin());
            resetSelection(2 * getSelectionBegin());
        } else

        /* Paste */
        if (event->matches(QKeySequence::Paste))
        {
            QClipboard *clipboard = QApplication::clipboard();
            QByteArray ba = QByteArray().fromHex(clipboard->text().toLatin1());
            if (_overwriteMode)
            {
                ba = ba.left(std::min<qint64>(ba.size(), (_chunks->size() - _bPosCurrent)));
                replace(_bPosCurrent, ba.size(), ba);
            }
            else
                insert(_bPosCurrent, ba);
            setCursorPosition(_cursorPosition + 2 * ba.size());
            resetSelection(getSelectionBegin());
        } else

        /* Delete char */
        if (event->matches(QKeySequence::Delete))
        {
            if (getSelectionBegin() != getSelectionEnd())
            {
                _bPosCurrent = getSelectionBegin();
                if (_overwriteMode)
                {
                    QByteArray ba = QByteArray(getSelectionEnd() - getSelectionBegin(), char(0));
                    replace(_bPosCurrent, ba.size(), ba);
                }
                else
                {
                    remove(_bPosCurrent, getSelectionEnd() - getSelectionBegin());
                }
            }
            else
            {
                if (_overwriteMode)
                    replace(_bPosCurrent, char(0));
                else
                    remove(_bPosCurrent, 1);
            }
            setCursorPosition(2 * _bPosCurrent);
            resetSelection(2 * _bPosCurrent);
        } else

        /* Backspace */
        if ((event->key() == Qt::Key_Backspace) && (event->modifiers() == Qt::NoModifier))
        {
            if (getSelectionBegin() != getSelectionEnd())
            {
                _bPosCurrent = getSelectionBegin();
                setCursorPosition(2 * _bPosCurrent);
                if (_overwriteMode)
                {
                    QByteArray ba = QByteArray(getSelectionEnd() - getSelectionBegin(), char(0));
                    replace(_bPosCurrent, ba.size(), ba);
                }
                else
                {
                    remove(_bPosCurrent, getSelectionEnd() - getSelectionBegin());
                }
                resetSelection(2 * _bPosCurrent);
            }
            else
            {
                bool behindLastByte = false;
                if ((_cursorPosition / 2) == _chunks->size())
                    behindLastByte = true;

                _bPosCurrent -= 1;
                if (_overwriteMode)
                    replace(_bPosCurrent, char(0));
                else
                    remove(_bPosCurrent, 1);

                if (!behindLastByte)
                    _bPosCurrent -= 1;

                setCursorPosition(2 * _bPosCurrent);
                resetSelection(2 * _bPosCurrent);
            }
        } else

        /* undo */
        if (event->matches(QKeySequence::Undo))
        {
            undo();
        } else

        /* redo */
        if (event->matches(QKeySequence::Redo))
        {
            redo();
        } else

        if ((QApplication::keyboardModifiers() == Qt::NoModifier) ||
            (QApplication::keyboardModifiers() == Qt::KeypadModifier) ||
            (QApplication::keyboardModifiers() == Qt::ShiftModifier) ||
            (QApplication::keyboardModifiers() == (Qt::AltModifier | Qt::ControlModifier)) ||
            (QApplication::keyboardModifiers() == Qt::GroupSwitchModifier))
        {
            /* Hex and ascii input */
            int key = 0;
            QString text = event->text();
            if (!text.isEmpty())
            {
                if (_editAreaIsAscii)
                    key = (uchar)text.at(0).toLatin1();
                else
                    key = int(text.at(0).toLower().toLatin1());
            }

            if ((((key >= '0' && key <= '9') || (key >= 'a' && key <= 'f')) && _editAreaIsAscii == false)
                || (key >= ' ' && _editAreaIsAscii))
            {
                if (getSelectionBegin() != getSelectionEnd())
                {
                    if (_overwriteMode)
                    {
                        qint64 len = getSelectionEnd() - getSelectionBegin();
                        replace(getSelectionBegin(), (int)len, QByteArray((int)len, char(0)));
                    } else
                    {
                        remove(getSelectionBegin(), getSelectionEnd() - getSelectionBegin());
                        _bPosCurrent = getSelectionBegin();
                    }
                    setCursorPosition(2 * _bPosCurrent);
                    resetSelection(2 * _bPosCurrent);
                }

                // If insert mode, then insert a byte
                if (_overwriteMode == false)
                    if ((_cursorPosition % 2) == 0)
                        insert(_bPosCurrent, char(0));

                // Change content
                if (_chunks->size() > 0)
                {
                    char ch = key;
                    if (!_editAreaIsAscii){
                        QByteArray hexValue = _chunks->data(_bPosCurrent, 1).toHex();
                        if ((_cursorPosition % 2) == 0)
                            hexValue[0] = key;
                        else
                            hexValue[1] = key;
                        ch = QByteArray().fromHex(hexValue)[0];
                    }
                    replace(_bPosCurrent, ch);
                    if (_editAreaIsAscii)
                        setCursorPosition(_cursorPosition + 2);
                    else
                        setCursorPosition(_cursorPosition + 1);
                    resetSelection(_cursorPosition);
                }
            }
        }


    }

    /* Copy */
    if (event->matches(QKeySequence::Copy))
    {
        QByteArray ba = _chunks->data(getSelectionBegin(), getSelectionEnd() - getSelectionBegin()).toHex();
        for (qint64 idx = 32; idx < ba.size(); idx +=33)
            ba.insert(idx, "\n");
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(ba);
    }

    // Switch between insert/overwrite mode
    if ((event->key() == Qt::Key_Insert) && (event->modifiers() == Qt::NoModifier))
    {
        setOverwriteMode(!overwriteMode());
        setCursorPosition(_cursorPosition);
    }

    // switch from hex to ascii edit
    if (event->key() == Qt::Key_Tab && !_editAreaIsAscii){
        _editAreaIsAscii = true;
        setCursorPosition(_cursorPosition);
    }

    // switch from ascii to hex edit
    if (event->key() == Qt::Key_Backtab  && _editAreaIsAscii){
        _editAreaIsAscii = false;
        setCursorPosition(_cursorPosition);
    }

    refresh();
}

void QHexEdit::mouseMoveEvent(QMouseEvent * event)
{
    _blink = false;
    viewport()->update();
    qint64 actPos = cursorPosition(event->pos());
    if (actPos >= 0)
    {
        setCursorPosition(actPos);
        setSelection(actPos);
    }
}

void QHexEdit::mousePressEvent(QMouseEvent * event)
{
    _blink = false;
    viewport()->update();
    qint64 cPos = cursorPosition(event->pos());
    if (cPos >= 0)
    {
        if (event->button() != Qt::RightButton)
            resetSelection(cPos);
        setCursorPosition(cPos);
    }
}

void QHexEdit::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    int pxOfsX = horizontalScrollBar()->value();
    int pxPosAsciiX2 = _pxPosAsciiX  - pxOfsX;

    if (event->rect() != _cursorRect)
    {
        int pxPosStartY = _pxCharHeight;

        // *** prepare background
        painter.fillRect(
            event->rect(), 
            viewport()->palette().color(QPalette::Base)
        );
        if (_addressArea)
            painter.fillRect(
                QRect(
                    -pxOfsX, 
                    event->rect().top(), 
                    _pxCharWidth * (_addrDigits + 1), 
                    height()
                ), 
                _colorManager->notMarked(Area::Address).areaStyle()
            );
        if (_asciiArea)
        {
            painter.fillRect(
                QRect(
                    pxPosAsciiX2 - _pxAreaMargin, 
                    event->rect().top(),  
                    _pxCharWidth * _bytesPerLine  + 2 * _pxAreaMargin, 
                    height()
                ),
                _colorManager->notMarked(Area::Ascii).areaStyle()
            );
        }

        // *** paint central areas
        for (int row = 0, pxPosY = pxPosStartY; row <= _rowsShown; row++, pxPosY +=_pxCharHeight)
        {
            QByteArray hex;
            qint64 bPosLine = row * _bytesPerLine;
            int pxPosX = _pxPosHexX  - pxOfsX;
            int pxPosAsciiX2 = _pxPosAsciiX  - pxOfsX;

            // *** address info
            if (_addressArea && row * _bytesPerLine < _dataShown.size()) 
            {
                QString address = QString("%1").arg(_bPosFirst + row*_bytesPerLine + _addressOffset, _addrDigits, 16, QChar('0'));
                ColoredArea addressArea = _colorManager->notMarked(Area::Address);

                painter.setPen(addressArea.fontPen());
                painter.drawText(_pxPosAdrX - pxOfsX, pxPosY, hexCaps() ? address.toUpper() : address);
            }

            for (int colIdx = 0; ((bPosLine + colIdx) < _dataShown.size() && (colIdx < _bytesPerLine)); colIdx++)
            {
                qint64 posBa = _bPosFirst + bPosLine + colIdx;

                // *** hex values
                ColoredArea hexArea = _colorManager->markedArea(posBa, Area::Hex, _chunks);
                painter.setPen(hexArea.fontPen());
                QRect rect;
                if (colIdx == 0)
                    rect = QRect(
                        pxPosX, 
                        pxPosY - _pxCharHeight + _pxSelectionSub, 
                        2*_pxCharWidth, 
                        _pxCharHeight
                    );
                else
                    rect = QRect(
                        pxPosX - _pxCharWidth, 
                        pxPosY - _pxCharHeight + _pxSelectionSub, 
                        3*_pxCharWidth, 
                        _pxCharHeight
                    );
                painter.fillRect(rect, hexArea.areaStyle());
                hex = _hexDataShown.mid((bPosLine + colIdx) * 2, 2);
                painter.drawText(pxPosX, pxPosY, hexCaps()?hex.toUpper():hex);
                pxPosX += 3*_pxCharWidth;

                // *** ascii values
                if (_asciiArea)
                {
                    ColoredArea asciiArea = _colorManager->markedArea(posBa, Area::Ascii, _chunks);
                    painter.setPen(asciiArea.fontPen());

                    int ch = (uchar)_dataShown.at(bPosLine + colIdx);
                    if ( ch < ' ' || ch > '~' )
                        ch = '.';
                    rect.setRect(pxPosAsciiX2, pxPosY - _pxCharHeight + _pxSelectionSub, _pxCharWidth, _pxCharHeight);
                    painter.fillRect(rect, asciiArea.areaStyle());
                    painter.drawText(pxPosAsciiX2, pxPosY, QChar(ch));
                    pxPosAsciiX2 += _pxCharWidth;
                }
            }
        }
    }

    // *** paint cursor, _cursorPosition counts in 2, _bPosFirst counts in 1
    int hexPos = _cursorPosition - 2 * _bPosFirst;

    // due to scrolling the cursor can go out of the currently displayed data
    if ((hexPos >= 0) && (hexPos <= _hexDataShown.size()))
    {
        ColoredArea curArea;
        if (_editAreaIsAscii)
            curArea = _colorManager->markedArea(hexPos / 2, Area::Ascii, _chunks);
        else
            curArea = _colorManager->markedArea(hexPos / 2, Area::Hex, _chunks);

        if (_blink && hasFocus())
        {
            if (_readOnly)
                painter.fillRect(_cursorRect, curArea.fontColor());
            else
                painter.fillRect(_cursorRect, curArea.fontColor());
        }

        // We have to repaint the current char because the curser destroys the char
        painter.setPen(curArea.fontColor());
        if (_editAreaIsAscii)
        {
            // every 2 hex there is 1 ascii
            int ch = (uchar)_dataShown.at(hexPos / 2);
            if (ch < ' ' || ch > '~')
                ch = '.';

            painter.drawText(_pxCursorX - pxOfsX, _pxCursorY, QChar(ch));
        }
        else
        {
            QByteArray txt = _hexDataShown.mid(hexPos, 1);
            if (_hexCaps)
                txt = txt.toUpper();
            painter.drawText(_pxCursorX - pxOfsX, _pxCursorY, txt);
        }
    }

    // *** emit event, if size has changed
    if (_lastEventSize != _chunks->size())
    {
        _lastEventSize = _chunks->size();
        emit currentSizeChanged(_lastEventSize);
    }
}

void QHexEdit::resizeEvent(QResizeEvent *)
{
    if (_dynamicBytesPerLine)
    {
        int pxFixGaps = 0;
        if (_addressArea)
            pxFixGaps = addressWidth() * _pxCharWidth + 2 * _pxAreaMargin;
        pxFixGaps += 2 * _pxAreaMargin;
        if (_asciiArea)
            pxFixGaps += 2 * _pxAreaMargin;

        // +1 because the last hex value do not have space. so it is effective one char more
        int charWidth = (viewport()->width() - pxFixGaps ) / _pxCharWidth + 1;

        // 2 hex alfa-digits 1 space 1 ascii per byte = 4; if ascii is disabled then 3
        // to prevent devision by zero use the min value 1
        setBytesPerLine(std::max(charWidth / (_asciiArea ? 4 : 3), 1));
    }
    adjust();
}

bool QHexEdit::focusNextPrevChild(bool next)
{
    if (_addressArea)
    {
        if ( (next && _editAreaIsAscii) || (!next && !_editAreaIsAscii ))
            return QWidget::focusNextPrevChild(next);
        else
            return false;
    }
    else
    {
        return QWidget::focusNextPrevChild(next);
    }
}

// ********************************************************************** Handle selections
void QHexEdit::resetSelection()
{
    _bSelectionBegin = 0;
    _bSelectionEnd = 0;
    _colorManager->selection().setRange(_bSelectionBegin, _bSelectionEnd);
}

void QHexEdit::resetSelection(qint64 pos)
{
    pos = pos / 2 ;
    if (pos < 0)
        pos = 0;
    if (pos > _chunks->size())
        pos = _chunks->size();

    _bSelectionBegin = pos;
    _bSelectionEnd = pos;
    _colorManager->selection().setRange(_bSelectionBegin, _bSelectionEnd);
}

void QHexEdit::setSelection(qint64 pos)
{
    pos = pos / 2;
    if (pos < 0)
        pos = 0;
    if (pos > _chunks->size())
        pos = _chunks->size();

    if (pos > _bSelectionBegin)
    {
        _bSelectionEnd = pos;
    }
    else
    {
        _bSelectionBegin = pos;
    }
    _colorManager->selection().setRange(_bSelectionBegin, _bSelectionEnd);
}

qint64 QHexEdit::getSelectionBegin()
{
    return _bSelectionBegin;
}

qint64 QHexEdit::getSelectionEnd()
{
    return _bSelectionEnd;
}

// ********************************************************************** Private utility functions
bool QHexEdit::event(QEvent *event)
{
    if (event->type() == QEvent::QEvent::PaletteChange)
    {
        QPalette palette = this->palette();
        _colorManager->setPalette(palette);
    }
    return QAbstractScrollArea::event(event);
}

void QHexEdit::init()
{
    _undoStack->clear();
    setAddressOffset(0);
    resetSelection(0);
    setCursorPosition(0);
    verticalScrollBar()->setValue(0);
    _modified = false;
}

void QHexEdit::adjust()
{
    // recalc Graphics
    if (_addressArea)
    {
        _addrDigits = addressWidth();
        _pxPosHexX = _pxAreaMargin + _addrDigits*_pxCharWidth + 2 * _pxAreaMargin;
    }
    else
        _pxPosHexX = _pxAreaMargin;
    _pxPosAdrX = _pxAreaMargin;
    _pxPosAsciiX = _pxPosHexX + _hexCharsInLine * _pxCharWidth + 2 * _pxAreaMargin;

    // set horizontalScrollBar()
    int pxWidth = _pxPosAsciiX;
    if (_asciiArea)
        pxWidth += _bytesPerLine*_pxCharWidth;
    horizontalScrollBar()->setRange(0, pxWidth - viewport()->width());
    horizontalScrollBar()->setPageStep(viewport()->width());

    // set verticalScrollbar()
    _rowsShown = ((viewport()->height()-4)/_pxCharHeight);
    int lineCount = (int)(_chunks->size() / (qint64)_bytesPerLine) + 1;
    verticalScrollBar()->setRange(0, lineCount - _rowsShown);
    verticalScrollBar()->setPageStep(_rowsShown);

    int value = verticalScrollBar()->value();
    _bPosFirst = (qint64)value * _bytesPerLine;
    _bPosLast = _bPosFirst + (qint64)(_rowsShown * _bytesPerLine) - 1;
    if (_bPosLast >= _chunks->size())
        _bPosLast = _chunks->size() - 1;
    readBuffers();
    setCursorPosition(_cursorPosition);
}

void QHexEdit::dataChangedPrivate(int)
{
    _modified = _undoStack->index() != 0;
    adjust();
    emit dataChanged();
}

void QHexEdit::refresh()
{
    ensureVisible();
    readBuffers();
}

void QHexEdit::readBuffers()
{
    _dataShown = _chunks->data(_bPosFirst, _bPosLast - _bPosFirst + _bytesPerLine + 1, &_markedShown);
    _hexDataShown = QByteArray(_dataShown.toHex());
}

QString QHexEdit::toReadable(const QByteArray &ba)
{
    QString result;

    for (int i=0; i < ba.size(); i += 16)
    {
        QString addrStr = QString("%1").arg(_addressOffset + i, addressWidth(), 16, QChar('0'));
        QString hexStr;
        QString ascStr;
        for (int j=0; j<16; j++)
        {
            if ((i + j) < ba.size())
            {
                hexStr.append(" ").append(ba.mid(i+j, 1).toHex());
                char ch = ba[i + j];
                if ((ch < 0x20) || (ch > 0x7e))
                        ch = '.';
                ascStr.append(QChar(ch));
            }
        }
        result += addrStr + " " + QString("%1").arg(hexStr, -48) + "  " + QString("%1").arg(ascStr, -17) + "\n";
    }
    return result;
}

void QHexEdit::updateCursor()
{
    if (_blink)
        _blink = false;
    else
        _blink = true;
    viewport()->update(_cursorRect);
}
