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
    QTranslator translator;
    QString translateName = "chipEditor_" + QLocale::system().name();
    QStringList allPaths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);

    foreach (const QString &path, allPaths) {
        //Instead of de_DE can be used any other. Idea is to check that the file is exists somewhere
        //in QStandardPaths::standardLocations. All others can be find in the same folder
        if (QFile::exists(path + "/chipProgrammer_de_DE.qm")) {
            if(translator.load(translateName, path)) a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();

    return a.exec();
}
