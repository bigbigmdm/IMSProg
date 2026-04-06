/*
 * Copyright (C) 2023 Mikhail Medvedev <e-ink-reader@yandex.ru>
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include "mainwindow.h"
#include <QApplication>
#include <QFileInfo>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("imsprog");
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(12);
    QApplication::setFont(font);
    QStringList allPaths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    QDir binDir(QCoreApplication::applicationDirPath());
    QString binRelPath = QDir::cleanPath(binDir.absoluteFilePath("../share/" + QCoreApplication::applicationName()));
    allPaths.insert(1, QDir::cleanPath(binRelPath));

    QString localeName = QLocale::system().name();
    QString translateName = "chipProgrammer_" + localeName;
    qDebug() << "localeName = " << localeName << ", translateName = " << translateName;

    foreach (const QString &path, allPaths)
    {
        QString fullPath = QDir(path).filePath(translateName + ".qm");
        qDebug() << "Trying " << fullPath;

        if (QFile::exists(fullPath))
        {
            qDebug() << "Found translation file:" << fullPath;
            QTranslator *translator = new QTranslator(&a);

            if (translator->load(translateName, path))
            {
                a.installTranslator(translator);
                qDebug() << "Installed" << translateName << "from" << path;
                break;
            }
            else
            {
                delete translator; 
            }
        }
    }

    QStringList cmdline_args = QCoreApplication::arguments();
    MainWindow w;
    w.show();

    return a.exec();
}
