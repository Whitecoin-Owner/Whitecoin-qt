#include "GuardKeyUpdatingInfoDialog.h"
#include "ui_GuardKeyUpdatingInfoDialog.h"

#include "wallet.h"

GuardKeyUpdatingInfoDialog::GuardKeyUpdatingInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GuardKeyUpdatingInfoDialog)
{
    ui->setupUi(this);

    setParent(XWCWallet::getInstance()->mainFrame->containerWidget);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet(BACKGROUNDWIDGET_STYLE);
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet(CONTAINERWIDGET_STYLE);

    ui->closeBtn->setStyleSheet(CANCELBTN_STYLE);

}

GuardKeyUpdatingInfoDialog::~GuardKeyUpdatingInfoDialog()
{
    delete ui;
}

void GuardKeyUpdatingInfoDialog::pop()
{
    move(0,0);
    exec();
}

void GuardKeyUpdatingInfoDialog::setAsset(QString _assetSymbol)
{
    ui->assetLabel->setText(_assetSymbol);
    QStringList guardAccountIds = XWCWallet::getInstance()->getInstance()->getAssetMultisigUpdatedGuards(_assetSymbol);
    int totalNum = XWCWallet::getInstance()->getFormalGuards().size();
    ui->stateLabel->setText(tr("%1 judges have updated. %2 judges have not yet.").arg(guardAccountIds.size()).arg(totalNum - guardAccountIds.size()));

    QString str;
    foreach (QString accountId, guardAccountIds)
    {
        str += XWCWallet::getInstance()->guardAccountIdToName(accountId) + "  ";
    }
    ui->updatedGuardsLabel->setText(str);
}

void GuardKeyUpdatingInfoDialog::on_closeBtn_clicked()
{
    close();
}
