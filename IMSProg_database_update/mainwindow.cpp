#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QUrl>
#include <QMessageBox>
#include <QErrorMessage>
#include <QStatusBar>
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <QMainWindow>
#include <QNetworkRequest>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentReply(nullptr)
    , outputFile(nullptr)
    , lastBytesReceived(0)
    , totalBytesReceived(0)
{
    ui->setupUi(this);
    
    qDebug() << QSslSocket::sslLibraryBuildVersionString();

    manager = new QNetworkAccessManager(this);
    setupConnections();
    
    fileUrl = qApp->property("app/urlDataFile").toString();
    fileName = qApp->property("app/userChipDatabaseFile").toString();
    fileBackup = qApp->property("app/userChipBackupFile").toString();

    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);
}

MainWindow::~MainWindow()
{
    if (currentReply) {
        currentReply->abort();
        currentReply->deleteLater();
    }
    
    if (outputFile) {
        outputFile->close();
        delete outputFile;
    }
    
    delete ui;
}

void MainWindow::setupConnections()
{
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::startDownload);
}

void MainWindow::startDownload()
{
    if (currentReply) {
        currentReply->abort();
        currentReply->deleteLater();
        currentReply = nullptr;
    }
    
     chipData.clear();

    if (outputFile) {
        outputFile->close();
        delete outputFile;
        outputFile = nullptr;
    }
    


    QFileInfo fileInfo(fileName);
    fileInfo.refresh();
    qint64 oldDatabaseSize;

    if (fileInfo.exists())
    {
        if (QFile::exists(fileBackup))
        {
            QFile::remove(fileBackup);
        }
        //if ->return
        QFile::copy(fileName,fileBackup);
        oldDatabaseSize = fileInfo.size();
        oldRecords = static_cast<int>(oldDatabaseSize / 0x44 -  1);
    }
    else
    {
        qDebug()<< "Not found old file";
        oldRecords = 0;
    }

    if (fileUrl.isEmpty() || fileName.isEmpty()) {
        return;
    }

    ui->statusbar->showMessage(tr("Downloading file IMSProg.Dat"));
    ui->startButton->setEnabled(false);
    ui->progressBar->setValue(0);
    
    QUrl url(fileUrl);
    if (!url.isValid()) {
        showError(tr("Invalid URL"));
        ui->startButton->setEnabled(true);
        return;
    }
    
    QNetworkRequest request(url);
    currentReply = manager->get(request);
    
    outputFile = new QFile(fileName);
    if (!outputFile->open(QIODevice::WriteOnly)) {
        showError("Can't create file: " + fileName);
        currentReply->deleteLater();
        currentReply = nullptr;
        ui->startButton->setEnabled(true);
        return;
    }
    
    connect(currentReply, &QNetworkReply::readyRead, this, &MainWindow::onReadyRead);
    connect(currentReply, &QNetworkReply::downloadProgress, this, &MainWindow::onDownloadProgress);
    connect(currentReply, &QNetworkReply::finished, this, &MainWindow::onFinished);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(currentReply, &QNetworkReply::errorOccurred, this, &MainWindow::onError);
#else
    connect(currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &MainWindow::onError);
#endif
    
    lastBytesReceived = 0;
    totalBytesReceived = 0;
}

void MainWindow::onReadyRead()
{
    if (outputFile && currentReply) {
        QByteArray data = currentReply->readAll();
        chipData.append(data);
        //outputFile->write(data);
        totalBytesReceived += data.size();
    }
}


void MainWindow::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        int percent = static_cast<int>((bytesReceived * 100) / bytesTotal);
        ui->progressBar->setValue(percent);
    }
}


void MainWindow::onFinished()
{
    int countNewChips = 0;
    if (outputFile) {
        countNewChips = chipData.size() / 0x44 -1;
        if (chipData.size() > 0x44)
        {
            outputFile->write(chipData);
            outputFile->flush();
        }

        outputFile->close();
        delete outputFile;
        outputFile = nullptr;
        ui->statusbar->clearMessage();
    }
    
    if (currentReply && currentReply->error() == QNetworkReply::NoError) {
        ui->progressBar->setValue(100);
        QString resultOfOperation = QString(tr("The database has been updated!\n\nThe old database contained %1 chips,\nThe new database contains %2 chips."))
                    .arg(oldRecords)
                    .arg(countNewChips);
        QMessageBox::information(this, "Ok", resultOfOperation);
    }
    
    if (currentReply) {
        currentReply->deleteLater();
        currentReply = nullptr;
    }
    
    ui->startButton->setEnabled(true);
}

void MainWindow::onError(QNetworkReply::NetworkError code)
{
    Q_UNUSED(code);
    if (currentReply) {
        showError(tr("Error loading file: ") + currentReply->errorString());
        currentReply->deleteLater();
        currentReply = nullptr;
    }
    
    if (outputFile) {
        outputFile->close();
        delete outputFile;
        outputFile = nullptr;
    }
    
    ui->startButton->setEnabled(true);
    ui->progressBar->setValue(0);
   // copy .bak -> dat
    QFileInfo fileInfo(fileName);
    if (fileInfo.size() < 0x44)
    {
        if (QFile::exists(fileName) && QFile::exists(fileBackup))
        {
            QFile::remove(fileName);
        }
        //if ->return
        QFile::copy(fileBackup, fileName);
    }
    fileInfo.refresh();

}

void MainWindow::showError(const QString &message)
{
    QMessageBox::critical(this, tr("Error:"), message);
}


void MainWindow::on_exitButton_clicked()
{
   MainWindow::close();
}

