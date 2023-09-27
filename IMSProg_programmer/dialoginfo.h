#ifndef DIALOGINFO_H
#define DIALOGINFO_H

#include <QDialog>

namespace Ui {
class DialogInfo;
}

class DialogInfo : public QDialog
{
    Q_OBJECT

public:
    explicit DialogInfo(QWidget *parent = nullptr);
    ~DialogInfo();
    void setChip(const uint chipType);

private slots:
    void on_pushButton_clicked();

private:
    Ui::DialogInfo *ui;
};

#endif // DIALOGINFO_H
