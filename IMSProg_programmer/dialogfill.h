#ifndef DIALOGFILL_H
#define DIALOGFILL_H

#include <QDialog>
#include <QString>

namespace Ui {
class DialogFill;
}

class DialogFill : public QDialog
{
    Q_OBJECT

public:
    explicit DialogFill(QWidget *parent = nullptr);
    ~DialogFill();

private slots:
    void on_pushButton_clicked();

signals:
    void sendAddr4(QString);

private:
    Ui::DialogFill *ui;
    QString addrData;
};

#endif // DIALOGFILL_H
