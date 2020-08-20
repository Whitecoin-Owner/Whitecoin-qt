#include "NameTransferDialog.h"
#include "ui_NameTransferDialog.h"

#include "wallet.h"
#include "dialog/TransactionResultDialog.h"
#include "dialog/ErrorResultDialog.h"

NameTransferDialog::NameTransferDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NameTransferDialog)
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
    ui->closeBtn->setStyleSheet(CANCELBTN_STYLE);

    QRegExp regx("[a-z][a-z0-9\-]+$");
    QValidator *validator = new QRegExpValidator(regx, this);
    ui->newNameLineEdit->setValidator( validator );

    init();
}

NameTransferDialog::~NameTransferDialog()
{
    delete ui;
}

void NameTransferDialog::pop()
{
    move(0,0);
    exec();
}

void NameTransferDialog::init()
{
    ui->accountComboBox->clear();
    QStringList accounts = XWCWallet::getInstance()->getRegisteredAccounts();
    ui->accountComboBox->addItems(accounts);

    if(accounts.contains(XWCWallet::getInstance()->currentAccount))
    {
        ui->accountComboBox->setCurrentText(XWCWallet::getInstance()->currentAccount);
    }

    QStringList assetIds = XWCWallet::getInstance()->assetInfoMap.keys();
    foreach (QString assetId, assetIds)
    {
        ui->assetComboBox->addItem( revertERCSymbol( XWCWallet::getInstance()->assetInfoMap.value(assetId).symbol), assetId);
    }
}

void NameTransferDialog::jsonDataUpdated(QString id)
{
    if( id == "NameTransferDialog-get_account-" + ui->newNameLineEdit->text() )
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);

        if( result == "\"result\":{}")
        {
            newNameOk = true;
            checkOkBtnEnabled();
            ui->nameTipLabel->setText("<body><font style=\"font-size:12px\" color=#543D89>" + tr( "The name is available") + "</font></body>" );
        }
        else
        {
            newNameOk = false;
            checkOkBtnEnabled();
            ui->nameTipLabel->setText("<body><font style=\"font-size:12px\" color=#EB005E>" + tr( "This name has been used") + "</font></body>" );
        }

        return;
    }

    if( id == "NameTransferDialog-name_transfer_to_address")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);

        if(result.startsWith("\"result\":"))
        {
            result.prepend("{");
            result.append("}");
            QJsonObject object = QJsonDocument::fromJson(result.toUtf8()).object();
            QString decodedStr = object.value("result").toString();

            XWCWallet::getInstance()->postRPC( "NameTransferDialog-decode_multisig_transaction-" + decodedStr, toJsonFormat( "decode_multisig_transaction", QJsonArray() << decodedStr));
        }
        else
        {
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Fail to create name-transfer trx!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }
        return;
    }

    if( id.startsWith("NameTransferDialog-decode_multisig_transaction-"))
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        _decodedStr = id.mid(QString("NameTransferDialog-decode_multisig_transaction-").size());

        result.prepend("{");
        result.append("}");
        QJsonObject object = QJsonDocument::fromJson(result.toUtf8()).object();
        QJsonObject trxObject = object.value("result").toObject();
        _trxCode = QJsonDocument(trxObject).toJson();

        XWCWallet::getInstance()->postRPC( "NameTransferDialog-get_transaction_id", toJsonFormat( "get_transaction_id", QJsonArray() << trxObject));

        return;
    }

    if( id == "NameTransferDialog-get_transaction_id")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
qDebug() << id << result;

        result.prepend("{");
        result.append("}");
        QJsonObject object = QJsonDocument::fromJson(result.toUtf8()).object();
        QString trxId = object.value("result").toString();

        XWCWallet::getInstance()->transactionDB.insertNameTransferTrx(_decodedStr, QStringList() << _trxCode << trxId);

        close();

        return;
    }
}

void NameTransferDialog::on_okBtn_clicked()
{
    QString assetId = ui->assetComboBox->currentData().toString();
    AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(assetId);

    QJsonObject assetObject;
    assetObject.insert("asset_id", assetId);
    assetObject.insert("amount", decimalToIntegerStr(ui->amountLineEdit->text(),assetInfo.precision) );

    XWCWallet::getInstance()->postRPC( "NameTransferDialog-name_transfer_to_address", toJsonFormat( "name_transfer_to_address",
                                                                                    QJsonArray() << ui->accountComboBox->currentText()
                                                                                    << ui->toAddressLineEdit->text() << assetObject
                                                                                    << ui->newNameLineEdit->text()));
}

void NameTransferDialog::on_closeBtn_clicked()
{
    close();
}

void NameTransferDialog::on_newNameLineEdit_textChanged(const QString &arg1)
{
    if( arg1.isEmpty())
    {
        newNameOk = false;
        checkOkBtnEnabled();
        ui->nameTipLabel->setText("" );
        return;
    }

    if( 63 < arg1.size() )
    {
        newNameOk = false;
        checkOkBtnEnabled();
        ui->nameTipLabel->setText("<body><font style=\"font-size:12px\" color=#EB005E>" + tr("More than 63 characters!") + "</font></body>" );
        return;
    }

    if( XWCWallet::getInstance()->accountInfoMap.keys().contains(arg1))
    {
        newNameOk = false;
        checkOkBtnEnabled();
        ui->nameTipLabel->setText("<body><font style=\"font-size:12px\" color=#EB005E>" + tr( "This name has been used") + "</font></body>" );
        return;
    }

    XWCWallet::getInstance()->postRPC( "NameTransferDialog-get_account-" + arg1, toJsonFormat( "get_account", QJsonArray() << arg1 ));

}

void NameTransferDialog::on_assetComboBox_currentIndexChanged(const QString &arg1)
{
    AssetInfo info = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId( getRealAssetSymbol( ui->assetComboBox->currentText()) ));
    QRegExp rx1(QString("^([0]|[1-9][0-9]{0,10})(?:\\.\\d{0,%1})?$|(^\\t?$)").arg(info.precision));
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->amountLineEdit->setValidator(pReg1);

    ui->amountLineEdit->clear();
}

void NameTransferDialog::checkOkBtnEnabled()
{
    if(newNameOk && ui->amountLineEdit->text().toDouble() > 0 && checkAddress(ui->toAddressLineEdit->text()))
    {
        ui->okBtn->setEnabled(true);
    }
    else
    {
        ui->okBtn->setEnabled(false);
    }
}

void NameTransferDialog::on_amountLineEdit_textChanged(const QString &arg1)
{
    checkOkBtnEnabled();
}

void NameTransferDialog::on_toAddressLineEdit_textChanged(const QString &arg1)
{
    checkOkBtnEnabled();
}
