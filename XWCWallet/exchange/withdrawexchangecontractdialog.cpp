#include "withdrawexchangecontractdialog.h"
#include "ui_withdrawexchangecontractdialog.h"

#include "wallet.h"
#include "commondialog.h"
#include "FeeChooseWidget.h"
#include "dialog/ErrorResultDialog.h"
#include "dialog/TransactionResultDialog.h"

WithdrawExchangeContractDialog::WithdrawExchangeContractDialog(bool _isExchangeMode, QWidget *parent) :
    QDialog(parent),
    isExchangeMode(_isExchangeMode),
    ui(new Ui::WithdrawExchangeContractDialog)
{
    ui->setupUi(this);

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
    ui->assetComboBox->setStyleSheet(COMBOBOX_BORDER_STYLE);

    feeChoose = new FeeChooseWidget(0,XWCWallet::getInstance()->feeType);
    ui->stackedWidget->addWidget(feeChoose);
    feeChoose->resize(ui->stackedWidget->size());
    ui->stackedWidget->setCurrentIndex(0);

    connect( XWCWallet::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    init();
}

WithdrawExchangeContractDialog::~WithdrawExchangeContractDialog()
{
    delete ui;
}

void WithdrawExchangeContractDialog::pop()
{
    move(0,0);
    exec();
}

void WithdrawExchangeContractDialog::init()
{
    ui->accountNameLabel->setText(XWCWallet::getInstance()->currentAccount);

    QStringList assetIds = XWCWallet::getInstance()->assetInfoMap.keys();
    foreach (QString assetId, assetIds)
    {
        ui->assetComboBox->addItem( revertERCSymbol( XWCWallet::getInstance()->assetInfoMap.value(assetId).symbol), assetId);
    }

    ui->exchangeTipLabel->setVisible(isExchangeMode);
}

void WithdrawExchangeContractDialog::setCurrentAsset(QString _assetSymbol)
{
    ui->assetComboBox->setCurrentText( revertERCSymbol( _assetSymbol));
}

void WithdrawExchangeContractDialog::jsonDataUpdated(QString id)
{

    if( id.startsWith( "id-unlock-WithdrawExchangeContractDialog") )
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if( result == "\"result\":null")
        {
            AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId( getRealAssetSymbol( ui->assetComboBox->currentText())));
            QString contractAddress = isExchangeMode ? EXCHANGE_MODE_CONTRACT_ADDRESS
                                                     : XWCWallet::getInstance()->getExchangeContractAddress(ui->accountNameLabel->text());
            QString func = isExchangeMode ? "withdraw" : "withdrawAsset";

            QString params = isExchangeMode ? QString("%1,%2").arg(decimalToIntegerStr(ui->amountLineEdit->text(), assetInfo.precision)).arg(getRealAssetSymbol( ui->assetComboBox->currentText()))
                      : QString("%1,%2").arg( getRealAssetSymbol( ui->assetComboBox->currentText())).arg(decimalToIntegerStr(ui->amountLineEdit->text(), assetInfo.precision));

            feeChoose->updatePoundageID();
            XWCWallet::getInstance()->postRPC( "id-invoke_contract-withdrawAsset", toJsonFormat( "invoke_contract",
                                                                                   QJsonArray() << ui->accountNameLabel->text()
                                                                                   << XWCWallet::getInstance()->currentContractFee() << stepCount
                                                                                   << contractAddress
                                                                                   << func  << params));
        }
        else if(result.startsWith("\"error\":"))
        {
            ui->okBtn->setEnabled(true);
            ui->tipLabel->setText("<body><font style=\"font-size:12px\" color=#ff224c>" + tr("Wrong password!") + "</font></body>" );
        }

        return;
    }

    if( id == "id-invoke_contract-withdrawAsset")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            close();

            TransactionResultDialog transactionResultDialog;
            transactionResultDialog.setInfoText(tr("Transaction of withdraw-balance from the exchange contract has been sent out!"));
            transactionResultDialog.setDetailText(result);
            transactionResultDialog.pop();
        }
        else if(result.startsWith("\"error\":"))
        {   
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Fail to withdraw balance from the exchange contract!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }

        return;
    }

    if( id == "id-invoke_contract_testing-withdrawAsset")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            XWCWallet::TotalContractFee totalFee = XWCWallet::getInstance()->parseTotalContractFee(result);
            stepCount = totalFee.step;
            unsigned long long totalAmount = totalFee.baseAmount + ceil(totalFee.step * XWCWallet::getInstance()->contractFee / 100.0);

            feeChoose->updateFeeNumberSlots(getBigNumberString(totalAmount, ASSET_PRECISION).toDouble());
            feeChoose->updateAccountNameSlots(ui->accountNameLabel->text(),true);
        }

        return;
    }
}

void WithdrawExchangeContractDialog::on_okBtn_clicked()
{
    if(ui->amountLineEdit->text().toDouble() <= 0)  return;
    if(ui->pwdLineEdit->text().isEmpty())          return;

    ui->okBtn->setEnabled(false);
    XWCWallet::getInstance()->postRPC( "id-unlock-WithdrawExchangeContractDialog", toJsonFormat( "unlock", QJsonArray() << ui->pwdLineEdit->text()
                                               ));
}

void WithdrawExchangeContractDialog::on_cancelBtn_clicked()
{
    close();
}

void WithdrawExchangeContractDialog::on_assetComboBox_currentIndexChanged(const QString &arg1)
{
    AssetAmountMap map = XWCWallet::getInstance()->accountInfoMap.value(ui->accountNameLabel->text()).assetAmountMap;
    AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId( getRealAssetSymbol( ui->assetComboBox->currentText())));


    QRegExp rx1(QString("^([0]|[1-9][0-9]{0,10})(?:\\.\\d{0,%1})?$|(^\\t?$)").arg(assetInfo.precision));
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->amountLineEdit->setValidator(pReg1);
    ui->amountLineEdit->clear();

    unsigned long long maxAmount = isExchangeMode ? XWCWallet::getInstance()->assetExchangeBalanceMap.value( getRealAssetSymbol( ui->assetComboBox->currentText())).available
              : XWCWallet::getInstance()->accountExchangeContractBalancesMap.value(ui->accountNameLabel->text()).value( getRealAssetSymbol( ui->assetComboBox->currentText()));
    ui->amountLineEdit->setPlaceholderText(tr("Max: %1 %2").arg(getBigNumberString(maxAmount, assetInfo.precision)).arg( revertERCSymbol( assetInfo.symbol)));
}

void WithdrawExchangeContractDialog::on_withdrawAllBtn_clicked()
{
    unsigned long long maxAmount = isExchangeMode ? XWCWallet::getInstance()->assetExchangeBalanceMap.value( getRealAssetSymbol( ui->assetComboBox->currentText())).available
              : XWCWallet::getInstance()->accountExchangeContractBalancesMap.value(ui->accountNameLabel->text()).value( getRealAssetSymbol( ui->assetComboBox->currentText()));
    AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId( getRealAssetSymbol( ui->assetComboBox->currentText())));

    ui->amountLineEdit->setText(getBigNumberString(maxAmount, assetInfo.precision));
}

void WithdrawExchangeContractDialog::on_closeBtn_clicked()
{
    close();
}

void WithdrawExchangeContractDialog::estimateContractFee()
{
    AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId( getRealAssetSymbol( ui->assetComboBox->currentText())));
    QString contractAddress = isExchangeMode ? EXCHANGE_MODE_CONTRACT_ADDRESS
                                             : XWCWallet::getInstance()->getExchangeContractAddress(ui->accountNameLabel->text());
    QString func = isExchangeMode ? "withdraw" : "withdrawAsset";

    QString params = isExchangeMode ? QString("%1,%2").arg(decimalToIntegerStr(ui->amountLineEdit->text(), assetInfo.precision)).arg(getRealAssetSymbol( ui->assetComboBox->currentText()))
              : QString("%1,%2").arg( getRealAssetSymbol( ui->assetComboBox->currentText())).arg(decimalToIntegerStr(ui->amountLineEdit->text(), assetInfo.precision));

    XWCWallet::getInstance()->postRPC( "id-invoke_contract_testing-withdrawAsset", toJsonFormat( "invoke_contract_testing",
                                                                           QJsonArray() << ui->accountNameLabel->text()
                                                                           << contractAddress
                                                                           << func  << params));


}

void WithdrawExchangeContractDialog::on_amountLineEdit_textChanged(const QString &arg1)
{
    estimateContractFee();
}
