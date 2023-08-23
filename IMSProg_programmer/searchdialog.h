#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QtCore>
#include "qhexedit.h"

namespace Ui {
    class SearchDialog;
}

class SearchDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SearchDialog(QHexEdit *hexEdit, QWidget *parent = 0);
    ~SearchDialog();
    qint64 findNext();
    Ui::SearchDialog *ui;

private slots:
    void on_pbFind_clicked();
    void on_pbReplace_clicked();
    void on_pbReplaceAll_clicked();
    void on_pb_png_clicked();
    void on_pb_jpg_clicked();
    void on_pb_gif_clicked();
    void on_pb_zip_clicked();
    void on_pb_tar_clicked();
    void on_pb_bios_clicked();
    void on_pb_uefi_clicked();
    void on_pb_gpt_clicked();

private:
    QByteArray getContent(int comboIndex, const QString &input);
    qint64 replaceOccurrence(qint64 idx, const QByteArray &replaceBa);

    QHexEdit *_hexEdit;
    QByteArray _findBa;
};

#endif // SEARCHDIALOG_H
