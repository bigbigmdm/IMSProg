/*
 * This file is part of the EZP_Chip_Editor project.
 *
 * Copyright (C) 2023 Mikhail Medvedev (e-ink-reader@yandex.ru)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * verbose functionality forked from https://github.com/bigbigmdm/EZP2019-EZP2025_chip_data_editor
 *
 */
#include "delegates.h"
#include <QApplication>
#include <QComboBox>


    chTypeDelegate::chTypeDelegate(QObject *parent): QItemDelegate(parent) { }
    QWidget *chTypeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QComboBox *editor = new QComboBox(parent);
        editor->addItem("SPI_FLASH");
        editor->addItem("25_EEPROM");
        editor->addItem("93_EEPROM");
        editor->addItem("24_EEPROM");
        editor->addItem("95_EEPROM");
        return editor;
    }
    void chTypeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QString text = index.model()->data(index, Qt::EditRole).toString();
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        int tindex = comboBox->findText(text);
        comboBox->setCurrentIndex(tindex);
    }
    void chTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        QString text = comboBox->currentText();
        model->setData(index, text, Qt::EditRole);
    }
    void chTypeDelegate::updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        editor->setGeometry(option.rect);
    }

    chVCCDelegate::chVCCDelegate(QObject *parent): QItemDelegate(parent) { }
    QWidget *chVCCDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QComboBox *editor = new QComboBox(parent);
        editor->addItem("5.0 V");
        editor->addItem("3.3 V");
        editor->addItem("1.8 V");
        return editor;
    }
    void chVCCDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QString text = index.model()->data(index, Qt::EditRole).toString();
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        int tindex = comboBox->findText(text);
        comboBox->setCurrentIndex(tindex);
    }
    void chVCCDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        QString text = comboBox->currentText();
        model->setData(index, text, Qt::EditRole);
    }
    void chVCCDelegate::updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        editor->setGeometry(option.rect);
    }
    chPagesDelegate::chPagesDelegate(QObject *parent): QItemDelegate(parent) { }
    QWidget *chPagesDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QComboBox *editor = new QComboBox(parent);
        editor->addItem("0x00");
        editor->addItem("0x01");
        editor->addItem("0x02");
        editor->addItem("0x04");
        return editor;
    }
    void chPagesDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QString text = index.model()->data(index, Qt::EditRole).toString();
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        int tindex = comboBox->findText(text);
        comboBox->setCurrentIndex(tindex);
    }
    void chPagesDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        QString text = comboBox->currentText();
        model->setData(index, text, Qt::EditRole);
    }
    void chPagesDelegate::updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        editor->setGeometry(option.rect);
    }
    chBlSizeDelegate::chBlSizeDelegate(QObject *parent): QItemDelegate(parent) { }
    QWidget *chBlSizeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QComboBox *editor = new QComboBox(parent);
        editor->addItem("8");
        editor->addItem("16");
        editor->addItem("32");
        editor->addItem("64");
        editor->addItem("128");
        editor->addItem("256");
        return editor;
    }
    void chBlSizeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QString text = index.model()->data(index, Qt::EditRole).toString();
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        int tindex = comboBox->findText(text);
        comboBox->setCurrentIndex(tindex);
    }
    void chBlSizeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        QString text = comboBox->currentText();
        model->setData(index, text, Qt::EditRole);
    }
    void chBlSizeDelegate::updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        editor->setGeometry(option.rect);
    }
