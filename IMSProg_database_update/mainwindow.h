#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QElapsedTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void startDownload();
    void onReadyRead();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onFinished();
    void onError(QNetworkReply::NetworkError code);
    void on_exitButton_clicked();

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *manager;
    QNetworkReply *currentReply;
    QFile *outputFile;
    QElapsedTimer speedTimer;
    qint64 lastBytesReceived;
    qint64 totalBytesReceived;
    QByteArray chipData;
    int oldRecords;
    QString fileUrl;
    QString fileName;
    QString fileBackup;
    
    void showError(const QString &message);
    void setupConnections();
};

#endif // MAINWINDOW_H
