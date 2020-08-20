#include "MultiSigTrxResultDialog.h"
#include "ui_MultiSigTrxResultDialog.h"

#include <QClipboard>

#include "wallet.h"

MultiSigTrxResultDialog::MultiSigTrxResultDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MultiSigTrxResultDialog)
{
    ui->setupUi(this);

    connect( XWCWallet::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    setParent(XWCWallet::getInstance()->mainFrame->containerWidget);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);


}

MultiSigTrxResultDialog::~MultiSigTrxResultDialog()
{
    delete ui;
}

void MultiSigTrxResultDialog::setTrxCode(QString code)
{
    ui->trxCodeTextBrowser->setText(code);

    XWCWallet::getInstance()->postRPC( "MultiSigTrxResultDialog-decode_multisig_transaction",
                                     toJsonFormat( "decode_multisig_transaction",
                                                   QJsonArray() << code ));
}

void MultiSigTrxResultDialog::pop()
{
    move(0,0);
    exec();
}

void MultiSigTrxResultDialog::jsonDataUpdated(QString id)
{
    if( id == "MultiSigTrxResultDialog-decode_multisig_transaction")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if( result.startsWith("\"result\":"))
        {
            result.prepend("{");
            result.append("}");
            QJsonObject object = QJsonDocument::fromJson(result.toUtf8()).object();
            QJsonObject resultObject = object.value("result").toObject();
            ui->trxStructTextBrowser->setText( QJsonDocument(resultObject).toJson());
        }
        else
        {
            ui->trxStructTextBrowser->setText("error: \n" + result);
        }
        return;
    }
}

void MultiSigTrxResultDialog::on_closeBtn_clicked()
{
    close();
}

void MultiSigTrxResultDialog::on_okBtn_clicked()
{
    close();
}

void MultiSigTrxResultDialog::on_copyBtn_clicked()
{
    QClipboard* clipBoard = QApplication::clipboard();
    clipBoard->setText(ui->trxCodeTextBrowser->toPlainText());
}

void MultiSigTrxResultDialog::on_copyBtn2_clicked()
{
    QClipboard* clipBoard = QApplication::clipboard();
    clipBoard->setText(ui->trxStructTextBrowser->toPlainText());
}


