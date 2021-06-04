#include "AddNftTokenDialog.h"
#include "ui_AddNftTokenDialog.h"

AddNftTokenDialog::AddNftTokenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddNftTokenDialog)
{
    ui->setupUi(this);
}

AddNftTokenDialog::~AddNftTokenDialog()
{
    delete ui;
}
