#include "MultiSigTransferWidget.h"
#include "ui_MultiSigTransferWidget.h"

#include "wallet.h"
#include "MultiSigPage.h"
#include "transfer/ContactChooseWidget.h"
#include "transfer/BlurWidget.h"
#include "commondialog.h"
#include "dialog/ErrorResultDialog.h"
#include "dialog/TransactionResultDialog.h"
#include "MultiSigTrxResultDialog.h"
#include "FeeChooseWidget.h"

MultiSigTransferWidget::MultiSigTransferWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MultiSigTransferWidget)
{
    ui->setupUi(this);

    connect( XWCWallet::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    InitStyle();

    ui->amountLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    QRegExp regx("[a-zA-Z0-9\ \n]+$");
    QValidator *validator = new QRegExpValidator(regx, this);
    ui->sendtoLineEdit->setValidator( validator );
    ui->sendtoLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    ui->tipLabel3->hide();
    ui->tipLabel4->hide();
    ui->tipLabel6->hide();

    QStringList keys = XWCWallet::getInstance()->assetInfoMap.keys();
    foreach (QString key, keys)
    {
        ui->assetComboBox->addItem( revertERCSymbol( XWCWallet::getInstance()->assetInfoMap.value(key).symbol));
    }
    connect(ui->assetComboBox,SIGNAL(currentIndexChanged(QString)),this,SLOT(onAssetComboBoxCurrentIndexChanged(QString)));

    feeWidget = new FeeChooseWidget(XWCWallet::getInstance()->feeChargeInfo.transferFee.toDouble(),XWCWallet::getInstance()->feeType);
    ui->stackedWidget->addWidget(feeWidget);
    ui->stackedWidget->setCurrentWidget(feeWidget);
    ui->stackedWidget->currentWidget()->resize(ui->stackedWidget->size());
    connect(this,&MultiSigTransferWidget::usePoundage,feeWidget,&FeeChooseWidget::updatePoundageID);

}

MultiSigTransferWidget::~MultiSigTransferWidget()
{
    delete ui;
}

void MultiSigTransferWidget::setFromAddress(QString address)
{
    ui->fromAddressLabel->setText(address);

    MultiSigPage* page = static_cast<MultiSigPage*>(this->parent());
    feeWidget->setBalance(page->multiSigBalancesMap.value(address));
}

void MultiSigTransferWidget::setAsset(QString asset)
{
    qDebug() << "aaaaaaaaaaaaaaaaa " << asset;
    ui->assetComboBox->setCurrentText( revertERCSymbol( asset));
    onAssetComboBoxCurrentIndexChanged(ui->assetComboBox->currentText());
}

void MultiSigTransferWidget::jsonDataUpdated(QString id)
{
    if( id == "MultiSigTransferWidget-transfer_from_to_address")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;
        if( result.startsWith("\"result\":"))             // 成功
        {
            result.prepend("{");
            result.append("}");
            QJsonObject object = QJsonDocument::fromJson(result.toUtf8()).object();
            QString trxCode = object.value("result").toString();

            MultiSigTrxResultDialog multiSigTrxResultDialog;
            multiSigTrxResultDialog.setTrxCode(trxCode);
            multiSigTrxResultDialog.pop();
        }
        else
        {
            ErrorResultDialog errorResultDialog;
            if(result.contains("Insufficient Balance"))
            {
                errorResultDialog.setInfoText(tr("Balance not enough!"));
            }
            else
            {
                errorResultDialog.setInfoText(tr("Fail to transfer!"));
            }
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }
        return;
    }
}

void MultiSigTransferWidget::on_sendBtn_clicked()
{
    if(!XWCWallet::getInstance()->ValidateOnChainOperation()) return;

    if(ui->amountLineEdit->text().size() == 0 || ui->sendtoLineEdit->text().size() == 0)
    {
        CommonDialog tipDialog(CommonDialog::OkOnly);
        tipDialog.setText( tr("Please enter the amount and address."));
        tipDialog.pop();
        return;
    }

    if( ui->amountLineEdit->text().toDouble()  <= 0)
    {
        CommonDialog tipDialog(CommonDialog::OkOnly);
        tipDialog.setText( tr("The amount can not be 0"));
        tipDialog.pop();
        return;
    }

    QString memo = ui->memoTextEdit->toPlainText();

    AddressType type = checkAddress(ui->sendtoLineEdit->text(),AccountAddress | MultiSigAddress);
    if( type == AccountAddress || type == MultiSigAddress)
    {
        emit usePoundage();
        XWCWallet::getInstance()->postRPC( "MultiSigTransferWidget-transfer_from_to_address",
                                         toJsonFormat( "transfer_from_to_address",
                                                       QJsonArray() << ui->fromAddressLabel->text() << ui->sendtoLineEdit->text()
                                                       << ui->amountLineEdit->text() << getRealAssetSymbol( ui->assetComboBox->currentText())
                                                       << memo ));
    }
}

void MultiSigTransferWidget::InitStyle()
{
    setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(239,242,245));
    setPalette(palette);

    ui->sendBtn->setStyleSheet(OKBTN_STYLE);

    ui->memoTextEdit->setStyleSheet("QTextEdit{background: transparent;color: rgb(90,115,227);font: 11px \"Microsoft YaHei UI Light\";border:none;border-bottom:1px solid rgb(90,115,227);}\
                                    QTextEdit:focus{border-bottom-color:rgb(90,115,227);}\
                                    QTextEdit:disabled{color: rgb(151,151,151);}");

    setStyleSheet("QToolButton#toolButton_chooseContact{background-image:url(:/ui/wallet_ui/trans.png);border:none;\
                   background-color:transparent;background-repeat: no-repeat;background-position: center;}\
                   QToolButton#toolButton_chooseContact:hover{background-color:black;");

}

void MultiSigTransferWidget::onAssetComboBoxCurrentIndexChanged(const QString &arg1)
{
    MultiSigPage* page = static_cast<MultiSigPage*>(this->parent());
    QString assetId = XWCWallet::getInstance()->getAssetId( getRealAssetSymbol( ui->assetComboBox->currentText()));
    unsigned long long amount = page->multiSigBalancesMap.value(ui->fromAddressLabel->text()).value(assetId).amount;
    AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(assetId);
    ui->amountLineEdit->setPlaceholderText(tr("Max: %1").arg(getBigNumberString(amount, assetInfo.precision)));

    QRegExp rx1(QString("^([0]|[1-9][0-9]{0,10})(?:\\.\\d{0,%1})?$|(^\\t?$)").arg(assetInfo.precision));
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->amountLineEdit->setValidator(pReg1);
}

void MultiSigTransferWidget::on_toolButton_chooseContact_clicked()
{
    ContactChooseWidget *wi = new ContactChooseWidget(this);
    if(wi->isInitSuccess())
    {
        BlurWidget *blur = new BlurWidget(this);
        connect(wi,&ContactChooseWidget::closeSignal,blur,&BlurWidget::close);
        connect(wi,&ContactChooseWidget::selectContactSignal,this,&MultiSigTransferWidget::selectContactSlots);

        blur->show();
        wi->move(QPoint(160,140));
        wi->show();
        wi->raise();
    }
}

void MultiSigTransferWidget::selectContactSlots(const QString &name, const QString &address)
{
    ui->sendtoLineEdit->setText(address);
}

void MultiSigTransferWidget::on_memoTextEdit_textChanged()
{
    QTextCodec* utfCodec = QTextCodec::codecForName("UTF-8");
    QByteArray ba = utfCodec->fromUnicode(ui->memoTextEdit->toPlainText());
    double fee = static_cast<double>(ba.size())*0.01/1024 + XWCWallet::getInstance()->feeChargeInfo.transferFee.toDouble();

    feeWidget->updateFeeNumberSlots(fee);
}
