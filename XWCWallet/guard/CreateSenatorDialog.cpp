#include "CreateSenatorDialog.h"
#include "ui_CreateSenatorDialog.h"

#include "wallet.h"
#include "FeeChooseWidget.h"
#include "miner/registerdialog.h"
#include "capitalTransferPage/PasswordConfirmWidget.h"
#include "dialog/TransactionResultDialog.h"
#include "dialog/ErrorResultDialog.h"

CreateSenatorDialog::CreateSenatorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateSenatorDialog)
{
    ui->setupUi(this);
    InitWidget();
}

CreateSenatorDialog::~CreateSenatorDialog()
{
    delete ui;
}

void CreateSenatorDialog::jsonDataUpdated(QString id)
{
    if("id_create_wallfacer" == id)
    {
        close();
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        if( result.startsWith("\"result\":{"))             // 成功
        {
            TransactionResultDialog transactionResultDialog;
            transactionResultDialog.setInfoText(tr("Transaction of create wallfacer has been sent,please wait for confirmation"));
            transactionResultDialog.setDetailText(result);
            transactionResultDialog.pop();
            emit CreateSenatorSuccess();
        }
        else
        {
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Failed!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }
    }
}

void CreateSenatorDialog::on_registerLabel_linkActivated(const QString &link)
{
    close();

    RegisterDialog registerDialog;
    registerDialog.pop();

}

void CreateSenatorDialog::CreateSenatorSlots()
{
    //输入密码框
    PasswordConfirmWidget *wi = new PasswordConfirmWidget(XWCWallet::getInstance()->mainFrame);
    connect(wi,&PasswordConfirmWidget::confirmSignal,[this](){
        if(FeeChooseWidget* feeWidget = dynamic_cast<FeeChooseWidget*>(this->ui->stackedWidget_fee->currentWidget()))
        {
            feeWidget->updatePoundageID();
        }
        XWCWallet::getInstance()->postRPC("id_create_wallfacer",toJsonFormat("create_wallfacer_member",
                                        QJsonArray()<<this->ui->accountComboBox->currentText()<<true));

    });
    wi->show();
}

void CreateSenatorDialog::InitWidget()
{
    connect( XWCWallet::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    setParent(XWCWallet::getInstance()->mainFrame);
    move(0,0);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet(BACKGROUNDWIDGET_STYLE);
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet(CONTAINERWIDGET_STYLE);

    ui->okBtn->setStyleSheet(OKBTN_STYLE);
    ui->cancelBtn->setStyleSheet(CANCELBTN_STYLE);
    ui->closeBtn->setStyleSheet(CLOSEBTN_STYLE);
    ui->accountComboBox->setStyleSheet(COMBOBOX_BORDER_STYLE);

    ui->stackedWidget->setCurrentIndex(0);

    InitData();

    connect(ui->closeBtn,&QToolButton::clicked,this,&CreateSenatorDialog::close);
    connect(ui->cancelBtn,&QToolButton::clicked,this,&CreateSenatorDialog::close);
    connect(ui->okBtn,&QToolButton::clicked,this,&CreateSenatorDialog::CreateSenatorSlots);

    FeeChooseWidget *feeWidget = new FeeChooseWidget(XWCWallet::getInstance()->feeChargeInfo.createSenatorFee.toDouble(),
                                                     XWCWallet::getInstance()->feeType,ui->accountComboBox->currentText());
    QTimer::singleShot(100,[this,feeWidget](){this->ui->okBtn->setEnabled(feeWidget->isSufficient() && !ui->accountComboBox->currentText().isEmpty());});
    connect(feeWidget,&FeeChooseWidget::feeSufficient,ui->okBtn,&QToolButton::setEnabled);
    connect(ui->accountComboBox,static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),
            std::bind(&FeeChooseWidget::updateAccountNameSlots,feeWidget,std::placeholders::_1,true));
    ui->stackedWidget_fee->addWidget(feeWidget);
    ui->stackedWidget_fee->setCurrentWidget(feeWidget);
    ui->stackedWidget_fee->currentWidget()->resize(ui->stackedWidget_fee->size());
}

void CreateSenatorDialog::InitData()
{
    QStringList keys = XWCWallet::getInstance()->getRegisteredAccounts();
    QStringList miners = XWCWallet::getInstance()->minerMap.keys();
    QStringList citizens = XWCWallet::getInstance()->getMyCitizens();
    QStringList senators = XWCWallet::getInstance()->getMyGuards();
    foreach (QString key, keys)
    {
        if(miners.contains(key) || citizens.contains(key) || senators.contains(key))
        {
            keys.removeAll(key);
        }
    }
    ui->accountComboBox->addItems(keys);
    ui->addressLabel->setText(XWCWallet::getInstance()->accountInfoMap.value(ui->accountComboBox->currentText()).address);
    connect(ui->accountComboBox,static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentTextChanged),[this](const QString &name){
        this->ui->addressLabel->setText(XWCWallet::getInstance()->accountInfoMap.value(name).address);
    });

}
