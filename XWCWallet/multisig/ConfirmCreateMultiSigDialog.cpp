#include "ConfirmCreateMultiSigDialog.h"
#include "ui_ConfirmCreateMultiSigDialog.h"

#include "wallet.h"
#include "FeeChooseWidget.h"

ConfirmCreateMultiSigDialog::ConfirmCreateMultiSigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfirmCreateMultiSigDialog)
{
    ui->setupUi(this);

    connect( XWCWallet::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    setParent(XWCWallet::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet(BACKGROUNDWIDGET_STYLE);
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet(CONTAINERWIDGET_STYLE);

    ui->okBtn->setStyleSheet(OKBTN_STYLE);
    ui->closeBtn->setStyleSheet(CANCELBTN_STYLE);
    ui->accountComboBox->setStyleSheet(COMBOBOX_BORDER_STYLE);



    QStringList accounts = XWCWallet::getInstance()->accountInfoMap.keys();
    ui->accountComboBox->addItems(accounts);

    feeWidget = new FeeChooseWidget( 0.002,XWCWallet::getInstance()->feeType,
                                                 ui->accountComboBox->currentText(), ui->containerWidget);
    connect(feeWidget,&FeeChooseWidget::feeSufficient,ui->okBtn,&QToolButton::setEnabled);
    feeWidget->move(30,120);
    feeWidget->resize(260,120);

    connect(ui->accountComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onAccountComboBoxCurrentIndexChanged(QString)));
    onAccountComboBoxCurrentIndexChanged(ui->accountComboBox->currentText());
}

ConfirmCreateMultiSigDialog::~ConfirmCreateMultiSigDialog()
{
    delete ui;
}

bool ConfirmCreateMultiSigDialog::pop()
{
    move(0,0);
    exec();

    return yesOrNo;
}

void ConfirmCreateMultiSigDialog::jsonDataUpdated(QString id)
{
    if( id == "ConfirmCreateMultiSigDialog-unlock")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);

        if(result == "\"result\":null")
        {
            yesOrNo = true;
            account = ui->accountComboBox->currentText();

            feeWidget->updatePoundageID();
            close();
        }
        else if(result.startsWith("\"error\":"))
        {
            ui->okBtn->setEnabled(true);
            ui->tipLabel->setText("<body><font style=\"font-size:12px\" color=#EB005E>" + tr("Wrong password!") + "</font></body>" );
        }
        return;
    }
}

void ConfirmCreateMultiSigDialog::on_okBtn_clicked()
{
    if( ui->pwdLineEdit->text().isEmpty())  return;

    ui->okBtn->setEnabled(false);
    XWCWallet::getInstance()->postRPC( "ConfirmCreateMultiSigDialog-unlock", toJsonFormat( "unlock", QJsonArray() << ui->pwdLineEdit->text()
                                               ));
}

void ConfirmCreateMultiSigDialog::on_closeBtn_clicked()
{
    close();
}

void ConfirmCreateMultiSigDialog::on_pwdLineEdit_textChanged(const QString &arg1)
{
    ui->tipLabel->clear();
}

void ConfirmCreateMultiSigDialog::onAccountComboBoxCurrentIndexChanged(const QString &arg1)
{
    feeWidget->updateAccountNameSlots(ui->accountComboBox->currentText(), true);
}
