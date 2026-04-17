#include "ezp_chip_editor.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QFileInfo>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("imsprog");
        QStringList allPaths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
        QStringList foundPaths;

        foreach (const QString &path, allPaths)
        {
            QString fullPath = path + "/chipProgrammer_de_DE.qm";
            QFile datfile(fullPath);
            if (QFileInfo(datfile).exists()) foundPaths << path;
        }

         // translation path is foundPaths.first();

    QTranslator translator;
    QString translateName = "chipEditor_" + QLocale::system().name();
    if(translator.load(translateName, foundPaths.first())) a.installTranslator(&translator);
    a.installTranslator(&translator);
    MainWindow w;
    w.show();

    return a.exec();
}
