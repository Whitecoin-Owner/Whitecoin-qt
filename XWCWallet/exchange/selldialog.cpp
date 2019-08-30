#include "selldialog.h"
#include "ui_selldialog.h"

#include "wallet.h"
#include "commondialog.h"
#include "FeeChooseWidget.h"
#include "dialog/ErrorResultDialog.h"
#include "dialog/TransactionResultDialog.h"

SellDialog::SellDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SellDialog)
{
    ui->setupUi(this);

    setParent(XWCWallet::getInstance()->mainFrame);

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
    ui->assetComboBox2->setStyleSheet(COMBOBOX_BORDER_STYLE);

    feeChoose = new FeeChooseWidget(0,XWCWallet::getInstance()->feeType);
    ui->stackedWidget->addWidget(feeChoose);
    feeChoose->resize(ui->stackedWidget->size());
    ui->stackedWidget->setCurrentIndex(0);

    connect( XWCWallet::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    ui->sellAmountLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->buyAmountLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    init();
}

SellDialog::~SellDialog()
{
    delete ui;
}

void SellDialog::pop()
{
    move(0,0);
    exec();
}

void SellDialog::init()
{
    ui->accountNameLabel->setText(XWCWallet::getInstance()->currentAccount);

    QStringList assetIds = XWCWallet::getInstance()->assetInfoMap.keys();
    foreach (QString assetId, assetIds)
    {
        ui->assetComboBox->addItem( revertERCSymbol( XWCWallet::getInstance()->assetInfoMap.value(assetId).symbol), assetId);
        ui->assetComboBox2->addItem( revertERCSymbol( XWCWallet::getInstance()->assetInfoMap.value(assetId).symbol), assetId);
    }
}

void SellDialog::setSellAsset(QString _assetSymbol)
{
    ui->assetComboBox->setCurrentText( revertERCSymbol( _assetSymbol));
}

void SellDialog::setBuyAsset(QString _assetSymbol)
{
    ui->assetComboBox2->setCurrentText( revertERCSymbol( _assetSymbol));
}

void SellDialog::jsonDataUpdated(QString id)
{
    if( id == "id-invoke_contract-putOnSellOrder")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            close();

            TransactionResultDialog transactionResultDialog;
            transactionResultDialog.setInfoText(tr("Transaction of sell-order has been sent out!"));
            transactionResultDialog.setDetailText(result);
            transactionResultDialog.pop();
        }
        else if(result.startsWith("\"error\":"))
        {            
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Fail to create sell-order!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }

        return;
    }

    if( id == "id-invoke_contract_testing-putOnSellOrder")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            XWCWallet::TotalContractFee totalFee = XWCWallet::getInstance()->parseTotalContractFee(result);
            stepCount = totalFee.step;
            unsigned long long totalAmount = totalFee.baseAmount + ceil(totalFee.step * XWCWallet::getInstance()->contractFee / 100.0);

            feeChoose->updateFeeNumberSlots(getBigNumberString(totalAmount, ASSET_PRECISION).toDouble());
            feeChoose->updateAccountNameSlots(ui->accountNameLabel->text());
        }

        return;
    }

    if( id.startsWith( "id-unlock-SellDialog") )
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if( result == "\"result\":null")
        {
            QString contractAddress = XWCWallet::getInstance()->getExchangeContractAddress(ui->accountNameLabel->text());

            AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(ui->assetComboBox->currentData().toString());
            AssetInfo assetInfo2 = XWCWallet::getInstance()->assetInfoMap.value(ui->assetComboBox2->currentData().toString());

            QString params = QString("%1,%2,%3,%4").arg( getRealAssetSymbol( ui->assetComboBox->currentText())).arg(decimalToIntegerStr(ui->sellAmountLineEdit->text(), assetInfo.precision))
                   .arg( getRealAssetSymbol( ui->assetComboBox2->currentText())).arg(decimalToIntegerStr(ui->buyAmountLineEdit->text(), assetInfo2.precision));
            feeChoose->updatePoundageID();
            XWCWallet::getInstance()->postRPC( "id-invoke_contract-putOnSellOrder", toJsonFormat( "invoke_contract",
                                                                                   QJsonArray() << ui->accountNameLabel->text()
                                                                                   << XWCWallet::getInstance()->currentContractFee() << stepCount
                                                                                   << contractAddress
                                                                                   << "putOnSellOrder"  << params));
        }
        else if(result.startsWith("\"error\":"))
        {
            ui->okBtn->setEnabled(true);
            ui->tipLabel->setText("<body><font style=\"font-size:12px\" color=#ff224c>" + tr("Wrong password!") + "</font></body>" );
        }

        return;
    }
}

void SellDialog::on_okBtn_clicked()
{
    if(ui->assetComboBox->currentText() == ui->assetComboBox2->currentText())
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
        commonDialog.setText( tr("Assets can not be the same!") );
        commonDialog.pop();

        return;
    }

    if(ui->sellAmountLineEdit->text().toDouble() <= 0 || ui->buyAmountLineEdit->text().toDouble() <= 0)
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
        commonDialog.setText(tr("The amount can not be 0!"));
        commonDialog.pop();
        return;
    }

    ui->okBtn->setEnabled(false);
    XWCWallet::getInstance()->postRPC( "id-unlock-SellDialog", toJsonFormat( "unlock", QJsonArray() << ui->pwdLineEdit->text()
                                               ));
}

void SellDialog::on_cancelBtn_clicked()
{
    close();
}


void SellDialog::on_assetComboBox_currentIndexChanged(const QString &arg1)
{
    ExchangeContractBalances balances = XWCWallet::getInstance()->accountExchangeContractBalancesMap.value(ui->accountNameLabel->text());

    unsigned long long balanceAmount = 0;
    if(balances.contains( getRealAssetSymbol( ui->assetComboBox->currentText())))
    {
        balanceAmount = balances.value( getRealAssetSymbol( ui->assetComboBox->currentText()));
    }

    AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId( getRealAssetSymbol( ui->assetComboBox->currentText())));

    QString balanceStr = getBigNumberString(balanceAmount,assetInfo.precision);

    ui->sellAmountLineEdit->setPlaceholderText(tr("Max: %1 %2").arg(balanceStr).arg( revertERCSymbol( ui->assetComboBox->currentText())));


    QRegExp rx1(QString("^([0]|[1-9][0-9]{0,10})(?:\\.\\d{0,%1})?$|(^\\t?$)").arg(assetInfo.precision));
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->sellAmountLineEdit->setValidator(pReg1);
    ui->sellAmountLineEdit->clear();
}

void SellDialog::on_assetComboBox2_currentIndexChanged(const QString &arg1)
{
    AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId( getRealAssetSymbol( ui->assetComboBox2->currentText())));

    QRegExp rx1(QString("^([0]|[1-9][0-9]{0,10})(?:\\.\\d{0,%1})?$|(^\\t?$)").arg(assetInfo.precision));
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->buyAmountLineEdit->setValidator(pReg1);
    ui->buyAmountLineEdit->clear();
}

void SellDialog::on_closeBtn_clicked()
{
    close();
}

void SellDialog::estimateContractFee()
{
    feeChoose->updateFeeNumberSlots(0);
    if(ui->sellAmountLineEdit->text().toDouble() <= 0)  return;
    if(ui->buyAmountLineEdit->text().toDouble() <= 0)  return;

    QString contractAddress = XWCWallet::getInstance()->getExchangeContractAddress(ui->accountNameLabel->text());

    AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(ui->assetComboBox->currentData().toString());
    AssetInfo assetInfo2 = XWCWallet::getInstance()->assetInfoMap.value(ui->assetComboBox2->currentData().toString());

    QString params = QString("%1,%2,%3,%4").arg( getRealAssetSymbol( ui->assetComboBox->currentText())).arg(decimalToIntegerStr(ui->sellAmountLineEdit->text(), assetInfo.precision))
            .arg( getRealAssetSymbol( ui->assetComboBox2->currentText())).arg(decimalToIntegerStr(ui->buyAmountLineEdit->text(), assetInfo2.precision));
    XWCWallet::getInstance()->postRPC( "id-invoke_contract_testing-putOnSellOrder", toJsonFormat( "invoke_contract_testing",
                                                                           QJsonArray() << ui->accountNameLabel->text()
                                                                           << contractAddress
                                                                           << "putOnSellOrder"  << params));
}

void SellDialog::on_sellAmountLineEdit_textChanged(const QString &arg1)
{
    ExchangeContractBalances balances = XWCWallet::getInstance()->accountExchangeContractBalancesMap.value(ui->accountNameLabel->text());
    unsigned long long balanceAmount = 0;
    if(balances.contains( getRealAssetSymbol( ui->assetComboBox->currentText())))
    {
        balanceAmount = balances.value( getRealAssetSymbol( ui->assetComboBox->currentText()));
    }

    AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId( getRealAssetSymbol( ui->assetComboBox->currentText())));
    QString balanceStr = getBigNumberString(balanceAmount,assetInfo.precision);
    if(ui->sellAmountLineEdit->text().toDouble() > balanceStr.toDouble())
    {
        ui->sellAmountLineEdit->setText(balanceStr);
        return;
    }

    estimateContractFee();
}

void SellDialog::on_buyAmountLineEdit_textChanged(const QString &arg1)
{
    estimateContractFee();
}

void SellDialog::on_depositBtn_clicked()
{
    goToDeposit = true;
    close();
}
