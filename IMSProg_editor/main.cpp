#include "ezp_chip_editor.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;
    QString translateName = "chipEditor_" + QLocale::system().name();
    if(translator.load(translateName, "languages/")) a.installTranslator(&translator);
    a.installTranslator(&translator);
    MainWindow w;
    w.show();

    return a.exec();
}
