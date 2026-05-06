#include "ezp_chip_editor.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDebug>
#include <QFileInfo>
#include <QStandardPaths>

static QString setUpTranslation(const QStringList &searchPaths)
{
    QString localeName = QLocale::system().name();
    QString translateName = "chipEditor_" + localeName;

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

    QDir userAppDataLocation(allPaths.value(0));
    if (!userAppDataLocation.exists()) {
        userAppDataLocation.mkpath(".");
        // XXX some sort of error handling that befits the application
    }

    qApp->setProperty("app/translationDirectory", setUpTranslation(allPaths));
    qApp->setProperty("app/systemChipDatabaseFile", findSystemChipDBFile(allPaths));
    qApp->setProperty("app/userChipDatabaseFile", userAppDataLocation.filePath("IMSProg.Dat"));
}


int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("imsprog");
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(12);
    QApplication::setFont(font);
    QApplication a(argc, argv);
    qDebug() << "Used Qt version:" << QT_VERSION_STR;
    initPaths();

    MainWindow w;
    w.show();

    return a.exec();
}
