
#include <QKeyEvent>
#include <QDebug>
#include <QFileInfo>
#ifdef WIN32
#include <windows.h>
#endif

#include "lockpage.h"
#include "ui_lockpage.h"
#include "wallet.h"

#include "commondialog.h"

LockPage::LockPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LockPage)
{
    ui->setupUi(this);

    connect( XWCWallet::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    setAutoFillBackground(true);
    QPalette palette;
    palette.setBrush(QPalette::Window,  QBrush(QPixmap(":/ui/wallet_ui/login_bg.png").scaled(this->size())));
    setPalette(palette);

    ui->closeBtn->setStyleSheet("QToolButton{background-image:url(:/ui/wallet_ui/cover_close.png);background-repeat: no-repeat;background-position: center;border: none;}");
    ui->miniBtn->setStyleSheet("QToolButton{background-image:url(:/ui/wallet_ui/cover_min.png);background-repeat: no-repeat;background-position: center;border: none;}");


#ifdef TARGET_OS_MAC
    ui->pwdLineEdit->setAttribute(Qt::WA_MacShowFocusRect,false);
#endif
    ui->pwdLineEdit->setFocus();
    ui->pwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    ui->pwdLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

#ifdef WIN32
    if( GetKeyState(VK_CAPITAL) )
    {
        ui->tipLabel->setText( tr("Caps lock opened!") );
    }
#endif

    unlockTimer = new QTimer(this);
    connect(unlockTimer, &QTimer::timeout, this, &LockPage::onTimer);
    unlockTimer->setSingleShot(true);

    ui->syncLabel->setText("Block: " + QString::number(XWCWallet::getInstance()->walletInfo.blockHeight));
    QTimer* timer = new QTimer(this);
    timer->start(1000);
    connect(timer, &QTimer::timeout, [this](){
        ui->syncLabel->setText("Block: " + QString::number(XWCWallet::getInstance()->walletInfo.blockHeight));
    });
}

LockPage::~LockPage()
{
	
    delete ui;

	
}

void LockPage::on_enterBtn_clicked()
{
	

    if( ui->pwdLineEdit->text().isEmpty() )
    {
        ui->tipLabel->setText( tr("Empty!") );
        return;
    }

    if( ui->pwdLineEdit->text().size() < 8)
    {
        ui->tipLabel->setText(tr("At least 8 letters!"));
        ui->pwdLineEdit->clear();
        return;
    }


    XWCWallet::getInstance()->postRPC( "id-unlock-lockpage", toJsonFormat( "unlock", QJsonArray() << ui->pwdLineEdit->text() ));
 qDebug() << "id_unlock_lockpage" ;
    emit showShadowWidget();
    ui->pwdLineEdit->setEnabled(false);

    unlockTimer->start(20000);
}


void LockPage::on_pwdLineEdit_returnPressed()
{
    on_enterBtn_clicked();	
}


void LockPage::on_closeBtn_clicked()
{
    if( XWCWallet::getInstance()->closeToMinimize)
    {
        emit tray();
    }
    else
    {
        CommonDialog commonDialog(CommonDialog::OkAndCancel);
        commonDialog.setText( tr( "Sure to close the Wallet?"));
        bool choice = commonDialog.pop();

        if( choice)
        {
            emit closeWallet();
        }
        else
        {
            return;
        }

    }
}

void LockPage::on_pwdLineEdit_textChanged(const QString &arg1)
{
    if( !arg1.isEmpty())
    {
        ui->tipLabel->clear();
    }

    if( arg1.indexOf(' ') > -1)
    {
        ui->pwdLineEdit->setText( ui->pwdLineEdit->text().remove(' '));
    }
}

void LockPage::jsonDataUpdated(QString id)
{
    if( id == "id-unlock-lockpage")
    {
        unlockTimer->stop();

        emit hideShadowWidget();
        ui->pwdLineEdit->clear();
        ui->pwdLineEdit->setEnabled(true);
        ui->pwdLineEdit->setFocus();

        QString  result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if( result == "\"result\":null")
        {
            emit unlock();
        }
        else if( result.mid(0,8) == "\"error\":")
        {
            ui->tipLabel->setText(tr("Wrong password!"));
            ui->pwdLineEdit->clear();
        }

        return;
    }

}



void LockPage::keyPressEvent(QKeyEvent *e)
{
#ifdef WIN32
    if( e->key() == Qt::Key_CapsLock)
    {
        if( GetKeyState(VK_CAPITAL) == -127 )  // 不知道为什么跟构造函数中同样的调用返回的short不一样
        {
            ui->tipLabel->setText( tr("Caps lock opened!") );
        }
        else
        {

            ui->tipLabel->setText( tr("Caps lock closed!") );
        }
    }
#endif

    QWidget::keyPressEvent(e);
}

void LockPage::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
}

void LockPage::on_miniBtn_clicked()
{
    emit minimum();
}

void LockPage::onTimer()
{
    emit hideShadowWidget();
    ui->pwdLineEdit->setEnabled(true);
}
