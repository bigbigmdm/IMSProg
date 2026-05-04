/*
 * Copyright (C) 2026 Mikhail Medvedev <e-ink-reader@yandex.ru>
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
#include <QApplication>
#include <QSslSocket>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTranslator>
#include "mainwindow.h"

static QString setUpTranslation(const QStringList &searchPaths)
{
    QString localeName = QLocale::system().name();
    //QString translateName = "chipProgrammer_" + localeName;
    QString translateName = "chipUpdater_" + localeName;

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
    qApp->setProperty("app/userChipDatabaseFile", userAppDataLocation.filePath("IMSProg.Dat"));
    qApp->setProperty("app/userConfigFile", userAppDataLocation.filePath("config.ini"));
    qApp->setProperty("app/urlDataFile", "https://antenna-dvb-t2.ru/dl_all/IMSProg.Dat");

    qDebug() << "Ini path " << userAppDataLocation.filePath("config.ini");
    qDebug() << "User database path " << userAppDataLocation.filePath("IMSProg.Dat");
}

int main(int argc, char *argv[])
{
    qDebug() << "SSL support:" << QSslSocket::supportsSsl();
    qDebug() << "Build version:" << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << "Runtime version:" << QSslSocket::sslLibraryVersionString();

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("imsprog");
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(12);
    QApplication::setFont(font);
    initPaths();

    MainWindow w;
    w.show();

    return a.exec();
}
