#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QUrl>
#include <QStatusBar>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QSaveFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentReply(nullptr)
    , tempFile(nullptr)
    , lastBytesReceived(0)
    , totalBytesReceived(0)
    , oldRecords(0)
{
    ui->setupUi(this);

    qDebug() << QSslSocket::sslLibraryBuildVersionString();

    manager = new QNetworkAccessManager(this);
    setupConnections();

    fileUrl = qApp->property("app/urlDataFile").toString();
    fileName = qApp->property("app/userChipDatabaseFile").toString();

    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);
}

MainWindow::~MainWindow()
{
    if (currentReply) {
        currentReply->abort();
        currentReply->deleteLater();
    }

    if (tempFile) {
        tempFile->close();
        delete tempFile;
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

    if (tempFile) {
        tempFile->close();
        delete tempFile;
        tempFile = nullptr;
    }

    totalBytesReceived = 0;
    lastBytesReceived = 0;

    // count of chips
    QFileInfo fileInfo(fileName);
    if (fileInfo.exists() && fileInfo.size() > 0x44) {
        oldRecords = static_cast<int>(fileInfo.size() / 0x44 - 1);
    } else {
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

    tempFile = new QTemporaryFile(this);
    if (!tempFile->open()) {
        showError(tr("Cannot create temporary file for download"));
        ui->startButton->setEnabled(true);
        return;
    }

    QNetworkRequest request(url);
    currentReply = manager->get(request);

    connect(currentReply, &QNetworkReply::readyRead, this, &MainWindow::onReadyRead);
    connect(currentReply, &QNetworkReply::downloadProgress, this, &MainWindow::onDownloadProgress);
    connect(currentReply, &QNetworkReply::finished, this, &MainWindow::onFinished);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(currentReply, &QNetworkReply::errorOccurred, this, &MainWindow::onError);
#else
    connect(currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &MainWindow::onError);
#endif
}

void MainWindow::onReadyRead()
{
    if (currentReply && tempFile) {
        QByteArray data = currentReply->readAll();
        if (!data.isEmpty()) {
            tempFile->write(data);
            totalBytesReceived += data.size();
        }
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
    bool success = false;
    int newRecords = 0;

    if (currentReply && currentReply->error() == QNetworkReply::NoError) {
        if (totalBytesReceived > 0x88) {
            tempFile->close();

            QFile::remove(fileName);
            if (tempFile->copy(fileName)) {
                success = true;
                newRecords = static_cast<int>(totalBytesReceived / 0x44 - 1);
                ui->progressBar->setValue(100);
                QString msg = (tr("The database has been updated!\n\nThe old database contained %1 chips,\nThe new database contains %2 chips."))
                              .arg(oldRecords).arg(newRecords);
                QMessageBox::information(this, tr("Ok"), msg);
            } else {
                showError(tr("Failed to replace the database file"));
            }
        } else {
            showError(tr("Downloaded file is too small (corrupted?)"));
        }
    }

    if (tempFile) {
        tempFile->close();
        delete tempFile;
        tempFile = nullptr;
    }

    if (currentReply) {
        currentReply->deleteLater();
        currentReply = nullptr;
    }

    ui->startButton->setEnabled(true);
    if (!success) {
        ui->progressBar->setValue(0);
    }
    ui->statusbar->clearMessage();
}

void MainWindow::onError(QNetworkReply::NetworkError code)
{
    Q_UNUSED(code);
    if (currentReply) {
        showError(tr("Error loading file: ") + currentReply->errorString());
        currentReply->deleteLater();
        currentReply = nullptr;
    }

    if (tempFile) {
        tempFile->close();
        delete tempFile;
        tempFile = nullptr;
    }

    ui->startButton->setEnabled(true);
    ui->progressBar->setValue(0);
    ui->statusbar->clearMessage();
}

void MainWindow::showError(const QString &message)
{
    QMessageBox::critical(this, tr("Error:"), message);
}

void MainWindow::on_exitButton_clicked()
{
    MainWindow::close();
}

