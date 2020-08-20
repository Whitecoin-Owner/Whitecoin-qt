#include "CreateCitizenDialog.h"
#include "ui_CreateCitizenDialog.h"

#include "wallet.h"
#include "commondialog.h"
#include "FeeChooseWidget.h"
#include "dialog/ErrorResultDialog.h"
#include "dialog/TransactionResultDialog.h"
#include "miner/registerdialog.h"

CreateCitizenDialog::CreateCitizenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateCitizenDialog)
{
    ui->setupUi(this);

    connect( XWCWallet::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    setParent(XWCWallet::getInstance()->mainFrame->containerWidget);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet(BACKGROUNDWIDGET_STYLE);
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet(CONTAINERWIDGET_STYLE);

    ui->okBtn->setStyleSheet(OKBTN_STYLE);
    ui->cancelBtn->setStyleSheet(CANCELBTN_STYLE);
    ui->closeBtn->setStyleSheet(CLOSEBTN_STYLE);
    ui->okBtn2->setStyleSheet(OKBTN_STYLE);
    ui->cancelBtn2->setStyleSheet(CANCELBTN_STYLE);
    ui->accountComboBox->setStyleSheet(COMBOBOX_BORDER_STYLE);

    FeeChooseWidget *feeWidget = new FeeChooseWidget(XWCWallet::getInstance()->feeChargeInfo.createCitizenFee.toDouble(),
                                                     XWCWallet::getInstance()->feeType);
    connect(feeWidget,&FeeChooseWidget::feeSufficient,ui->okBtn,&QToolButton::setEnabled);
    ui->stackedWidget_fee->addWidget(feeWidget);
    ui->stackedWidget_fee->setCurrentIndex(0);
    ui->stackedWidget_fee->currentWidget()->resize(ui->stackedWidget_fee->size());
    init();
}

CreateCitizenDialog::~CreateCitizenDialog()
{
    delete ui;
}

void CreateCitizenDialog::pop()
{
    move(0,0);
    exec();
}

void CreateCitizenDialog::init()
{
    ui->okBtn->setEnabled(false);

    QStringList keys = XWCWallet::getInstance()->getRegisteredAccounts();
    QStringList miners = XWCWallet::getInstance()->minerMap.keys();
    QStringList guards = XWCWallet::getInstance()->getMyGuards();
    foreach (QString key, keys)
    {
        if(miners.contains(key) || guards.contains(key))
        {
            keys.removeAll(key);
        }
    }
    ui->accountComboBox->addItems(keys);

    ui->stackedWidget->setCurrentIndex(0);
}

void CreateCitizenDialog::jsonDataUpdated(QString id)
{
    if( id == "id-unlock-CreateCitizenDialog")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);

        if(result == "\"result\":null")
        {
            XWCWallet::getInstance()->postRPC(  "id-create_miner", toJsonFormat("create_miner",
                                                                              QJsonArray() << ui->accountComboBox->currentText()
                                                                              << "" << true ));

        }
        else if(result.startsWith("\"error\":"))
        {
            ui->okBtn2->setEnabled(true);
            ui->tipLabel2->setText("<body><font style=\"font-size:12px\" color=#EB005E>" + tr("Wrong password!") + "</font></body>" );
        }

        return;
    }

    if( id == "id-create_miner")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;
        if(result.startsWith("\"result\":{"))
        {
            close();

            TransactionResultDialog transactionResultDialog;
            transactionResultDialog.setInfoText(tr("Transaction of create-miner has been sent out!"));
            transactionResultDialog.setDetailText(result);
            transactionResultDialog.pop();
        }
        else if(result.startsWith("\"error\":"))
        {
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setDetailText(result);

            if(result.contains("Insufficient Balance:"))
            {
                errorResultDialog.setInfoText(tr("Balance of this account is not enough!"));
            }
            else
            {
                errorResultDialog.setInfoText(tr("Fail to register account!"));
            }

            errorResultDialog.pop();
        }

        return;
    }
}

void CreateCitizenDialog::on_closeBtn_clicked()
{
    close();
}

void CreateCitizenDialog::on_okBtn_clicked()
{
    if(ui->accountComboBox->currentText().isEmpty())    return;

    ui->stackedWidget->setCurrentIndex(1);
}

void CreateCitizenDialog::on_cancelBtn_clicked()
{
    close();
}

void CreateCitizenDialog::on_registerLabel_linkActivated(const QString &link)
{
    close();

    RegisterDialog registerDialog;
    registerDialog.pop();
}

void CreateCitizenDialog::on_accountComboBox_currentIndexChanged(const QString &arg1)
{
    ui->addressLabel->setText(XWCWallet::getInstance()->accountInfoMap.value(ui->accountComboBox->currentText()).address);
    if(FeeChooseWidget *feeWidget = qobject_cast<FeeChooseWidget*>(ui->stackedWidget_fee->currentWidget()))
    {
        feeWidget->updateAccountNameSlots(ui->accountComboBox->currentText(),true);
    }
}

void CreateCitizenDialog::on_okBtn2_clicked()
{
    if( ui->pwdLineEdit->text().isEmpty())  return;

    ui->okBtn2->setEnabled(false);
    XWCWallet::getInstance()->postRPC( "id-unlock-CreateCitizenDialog", toJsonFormat( "unlock", QJsonArray() << ui->pwdLineEdit->text()
                                               ));
}

void CreateCitizenDialog::on_pwdLineEdit_textEdited(const QString &arg1)
{
    ui->tipLabel2->clear();
}

void CreateCitizenDialog::on_cancelBtn2_clicked()
{
    close();
}
