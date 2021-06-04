#ifndef ADDNFTTOKENDIALOG_H
#define ADDNFTTOKENDIALOG_H

#include <QDialog>

namespace Ui {
class AddNftTokenDialog;
}

class AddNftTokenDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddNftTokenDialog(QWidget *parent = nullptr);
    ~AddNftTokenDialog();

private:
    Ui::AddNftTokenDialog *ui;
};

#endif // ADDNFTTOKENDIALOG_H
