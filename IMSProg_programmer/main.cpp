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
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTranslator>

static QString setUpTranslation(const QStringList &searchPaths)
{
    QString localeName = QLocale::system().name();
    QString translateName = "chipProgrammer_" + localeName;

    // skip user-specific dir for translations (first one); try the rest
    foreach (const QString &path, searchPaths.mid(1))
    {
        QTranslator *translator = new QTranslator(qApp);
        if (translator->load(translateName, path))
        {
            qApp->installTranslator(translator);
            qDebug() << "Installed" << translateName << "from" << path;
            return path;
        } else {
            delete translator;
        }
    }

    return QString();
}

static QString findSystemChipDBFile(const QStringList &searchPaths)
{
    foreach (const QString &path, searchPaths.mid(1))
    {
        QString chipdbfile = QDir(path).filePath("IMSProg.Dat");
        if (QFile::exists(chipdbfile)) {
            return chipdbfile;
        }
    }

    return QString();
}

static void initPaths()
{
    QStringList allPaths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    if (allPaths.isEmpty()) {
        // do not translate
        qFatal("Critical error: QStandardPaths::standardLocations(QStandardPaths::AppDataLocation): empty list");
    }

    QDir binDir(QCoreApplication::applicationDirPath());
    QString binRelPath = QDir::cleanPath(binDir.absoluteFilePath("../share/" + QCoreApplication::applicationName()));
    allPaths.insert(1, binRelPath);

    QDir userAppDataLocation(allPaths.at(0));
    if (!userAppDataLocation.exists()) {
        userAppDataLocation.mkpath(".");
        // XXX some sort of error handling that befits the application
    }

    qApp->setProperty("app/translationDirectory", setUpTranslation(allPaths));
    qApp->setProperty("app/systemChipDatabaseFile", findSystemChipDBFile(allPaths));
    qApp->setProperty("app/userChipDatabaseFile", userAppDataLocation.filePath("IMSProg.Dat"));
    qApp->setProperty("app/userConfigFile", userAppDataLocation.filePath("config.ini"));
}


int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setApplicationName("imsprog");
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(12);
    QApplication::setFont(font);
    QApplication a(argc, argv);
    initPaths();

    MainWindow w;
    w.show();

    return a.exec();
}
