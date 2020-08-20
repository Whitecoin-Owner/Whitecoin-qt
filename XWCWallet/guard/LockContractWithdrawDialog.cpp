#include "LockContractWithdrawDialog.h"
#include "ui_LockContractWithdrawDialog.h"

#include "wallet.h"
#include "commondialog.h"
#include "FeeChooseWidget.h"
#include "dialog/ErrorResultDialog.h"
#include "dialog/TransactionResultDialog.h"

LockContractWithdrawDialog::LockContractWithdrawDialog(QString _accountName, QString _asset, QWidget *parent) :
    QDialog(parent),
    m_accountName(_accountName),
    m_asset(_asset),
    ui(new Ui::LockContractWithdrawDialog)
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


    feeChoose = new FeeChooseWidget(0,XWCWallet::getInstance()->feeType);
    ui->stackedWidget->addWidget(feeChoose);
    feeChoose->resize(ui->stackedWidget->size());
    ui->stackedWidget->setCurrentIndex(0);

    init();

}

LockContractWithdrawDialog::~LockContractWithdrawDialog()
{
    delete ui;
}

void LockContractWithdrawDialog::pop()
{
    move(0,0);
    exec();
}

void LockContractWithdrawDialog::init()
{
    ui->accountNameLabel->setText(m_accountName);
    ui->assetLabel->setText( revertERCSymbol( m_asset));

    ui->okBtn->setEnabled(false);
}

void LockContractWithdrawDialog::setMaxAmount(unsigned long long _amount)
{
    m_amount = _amount;
    AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(m_asset));
    QString amountStr = getBigNumberString(m_amount, assetInfo.precision);
    ui->amountLineEdit->setPlaceholderText(tr("Max: %1 %2").arg(amountStr).arg( revertERCSymbol( m_asset)) );

    QRegExp rx1(QString("^([0]|[1-9][0-9]{0,10})(?:\\.\\d{0,%1})?$|(^\\t?$)").arg(assetInfo.precision));
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->amountLineEdit->setValidator(pReg1);
    ui->amountLineEdit->clear();
}

void LockContractWithdrawDialog::jsonDataUpdated(QString id)
{
    if( id == "LockContractWithdrawDialog+invoke_contract_testing")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);

        if(result.startsWith("\"result\":"))
        {
            XWCWallet::TotalContractFee totalFee = XWCWallet::getInstance()->parseTotalContractFee(result);
            stepCount = totalFee.step;
            unsigned long long totalAmount = totalFee.baseAmount + ceil(totalFee.step * XWCWallet::getInstance()->contractFee / 100.0);

            feeChoose->updateFeeNumberSlots(getBigNumberString(totalAmount, ASSET_PRECISION).toDouble());
            feeChoose->updateAccountNameSlots(m_accountName,true);

            checkOkBtnEnabled();
        }

        return;
    }

    if( id == "LockContractWithdrawDialog+unlock" )
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if( result == "\"result\":null")
        {
            AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value( XWCWallet::getInstance()->getAssetId(m_asset));
            QString param = QString("%1,%2").arg(decimalToIntegerStr(ui->amountLineEdit->text(), assetInfo.precision)).arg(m_asset);
            XWCWallet::getInstance()->postRPC( "LockContractWithdrawDialog+invoke_contract",
                                             toJsonFormat( "invoke_contract",
                                                           QJsonArray() << m_accountName
                                                           << XWCWallet::getInstance()->currentContractFee() << stepCount
                                                           << LOCKFUND_CONTRACT_ADDRESS
                                                           << "withdraw"
                                                           << param));

        }
        else if(result.startsWith("\"error\":"))
        {
            ui->okBtn->setEnabled(true);
            ui->tipLabel->setText("<body><font style=\"font-size:12px\" color=#ff224c>" + tr("Wrong password!") + "</font></body>" );
        }

        return;
    }


    if( id == "LockContractWithdrawDialog+invoke_contract")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            close();

            TransactionResultDialog transactionResultDialog;
            transactionResultDialog.setInfoText(tr("Transaction of withdrawing lock position has been sent out!"));
            transactionResultDialog.setDetailText(result);
            transactionResultDialog.pop();
        }
        else if(result.startsWith("\"error\":"))
        {
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Fail to withdrawing lock position!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }

        return;
    }
}

void LockContractWithdrawDialog::on_okBtn_clicked()
{
    if(!XWCWallet::getInstance()->ValidateOnChainOperation())     return;

    if(ui->amountLineEdit->text().toDouble() <= 0)  return;
    if(ui->pwdLineEdit->text().isEmpty())           return;

    ui->okBtn->setEnabled(false);
    XWCWallet::getInstance()->postRPC( "LockContractWithdrawDialog+unlock", toJsonFormat( "unlock", QJsonArray() << ui->pwdLineEdit->text()
                                               ));

}

void LockContractWithdrawDialog::on_cancelBtn_clicked()
{
    close();
}

void LockContractWithdrawDialog::on_closeBtn_clicked()
{
    close();
}

void LockContractWithdrawDialog::on_withdrawAllBtn_clicked()
{
    AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId( m_asset));
    ui->amountLineEdit->setText(getBigNumberString(m_amount, assetInfo.precision));
}

void LockContractWithdrawDialog::on_amountLineEdit_textChanged(const QString &arg1)
{
    stepCount = 0;
    checkOkBtnEnabled();

    if(ui->amountLineEdit->text().toDouble() <= 0)  return;

    AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value( XWCWallet::getInstance()->getAssetId(m_asset));
    QString param = QString("%1,%2").arg(decimalToIntegerStr(ui->amountLineEdit->text(), assetInfo.precision)).arg(m_asset);
    XWCWallet::getInstance()->postRPC( "LockContractWithdrawDialog+invoke_contract_testing",
                                     toJsonFormat( "invoke_contract_testing",
                                                   QJsonArray() << m_accountName
                                                   << LOCKFUND_CONTRACT_ADDRESS
                                                   << "withdraw"
                                                   << param));

    qDebug() << toJsonFormat( "invoke_contract_testing",
                              QJsonArray() << m_accountName
                              << LOCKFUND_CONTRACT_ADDRESS
                              << "withdraw"
                              << param);
}

void LockContractWithdrawDialog::checkOkBtnEnabled()
{
    if(stepCount > 0 && !ui->pwdLineEdit->text().isEmpty())
    {
        ui->okBtn->setEnabled(true);
    }
    else
    {
        ui->okBtn->setEnabled(false);
    }
}

void LockContractWithdrawDialog::on_pwdLineEdit_textChanged(const QString &arg1)
{
    ui->tipLabel->clear();
    checkOkBtnEnabled();
}
