#ifndef DIALOGSFDP_H
#define DIALOGSFDP_H

#include <QDialog>
#include <QMessageBox>
extern "C" {
#include "ch341a_spi.h"
#include "spi_controller.h"
}
namespace Ui {
class DialogSFDP;
}

class DialogSFDP : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSFDP(QWidget *parent = nullptr);
    ~DialogSFDP();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::DialogSFDP *ui;
    QString bP(unsigned char z);
};

#endif // DIALOGSFDP_H
