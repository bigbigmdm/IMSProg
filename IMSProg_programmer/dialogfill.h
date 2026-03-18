#ifndef DIALOGFILL_H
#define DIALOGFILL_H

#include <QDialog>

namespace Ui {
class DialogFill;
}

class DialogFill : public QDialog
{
    Q_OBJECT

public:
    explicit DialogFill(QWidget *parent = nullptr);
    ~DialogFill();

private:
    Ui::DialogFill *ui;
};

#endif // DIALOGFILL_H
