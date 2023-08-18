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
#ifndef DELEGATES_H
#define DELEGATES_H

#include <QStyledItemDelegate>
#include <QItemDelegate>
#include <QComboBox>

class chTypeDelegate : public QItemDelegate
{
Q_OBJECT
public:
  explicit chTypeDelegate(QObject *parent);
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const;
  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const;
  void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:

public slots:
//virtual ~chTypeDelegate() {}
};

class chVCCDelegate : public QItemDelegate
{
Q_OBJECT
public:
  explicit chVCCDelegate(QObject *parent);
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const;
  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const;
  void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:

public slots:
//virtual ~chVCCDelegate() {}
};
class chPagesDelegate : public QItemDelegate
{
Q_OBJECT
public:
  explicit chPagesDelegate(QObject *parent);
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const;
  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const;
  void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:

public slots:
//virtual ~chPagesDelegate() {}
};
class chBlSizeDelegate : public QItemDelegate
{
Q_OBJECT
public:
  explicit chBlSizeDelegate(QObject *parent);
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const;
  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const;
  void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:

public slots:
//virtual ~chBlSizeDelegate() {}
};
#endif // DELEGATES_H
