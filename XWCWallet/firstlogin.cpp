#include <QDebug>

#include <QKeyEvent>
#ifdef WIN32
#include <Windows.h>
#endif

#include "firstlogin.h"
#include "ui_firstlogin.h"
#include "wallet.h"

#include "commondialog.h"

FirstLogin::FirstLogin(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FirstLogin)
{
    ui->setupUi(this);

    connect(XWCWallet::getInstance(),SIGNAL(jsonDataUpdated(QString)),this,SLOT(jsonDataUpdated(QString)));

    setAutoFillBackground(true);
    QPalette palette;
    palette.setBrush(QPalette::Window,  QBrush(QPixmap(":/ui/wallet_ui/login_bg.png").scaled(this->size())));
    setPalette(palette);

    ui->closeBtn->setStyleSheet("QToolButton{background-image:url(:/ui/wallet_ui/cover_close.png);background-repeat: no-repeat;background-position: center;border: none;}");
    ui->miniBtn->setStyleSheet("QToolButton{background-image:url(:/ui/wallet_ui/cover_min.png);background-repeat: no-repeat;background-position: center;border: none;}");


#ifdef TARGET_OS_MAC
    ui->pwdLineEdit->setAttribute(Qt::WA_MacShowFocusRect,false);
    ui->pwdLineEdit2->setAttribute(Qt::WA_MacShowFocusRect,false);
#endif
    ui->pwdLineEdit->setFocus();
    ui->pwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    ui->pwdLineEdit2->setContextMenuPolicy(Qt::NoContextMenu);
    ui->pwdLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->pwdLineEdit2->setAttribute(Qt::WA_InputMethodEnabled, false);


#ifdef WIN32
    if( GetKeyState(VK_CAPITAL) )
    {
        ui->tipLabel->setText( tr("Caps lock opened!") );
    }
#endif

}

FirstLogin::~FirstLogin()
{
    delete ui;

}

void FirstLogin::on_createBtn_clicked()
{

    if( ui->pwdLineEdit->text().isEmpty() || ui->pwdLineEdit2->text().isEmpty() )
    {
        ui->tipLabel->setText(tr("Empty!"));
        return;
    }

    if( ui->pwdLineEdit->text() == ui->pwdLineEdit2->text())
    {
        if( ui->pwdLineEdit->text().size() < 8)
        {
            ui->tipLabel->setText(tr("Too short! At least 8 bits."));
            return;
        }

        emit showShadowWidget();
        ui->pwdLineEdit->setEnabled(false);
        ui->pwdLineEdit2->setEnabled(false);

        XWCWallet::getInstance()->postRPC( "id-set_password", toJsonFormat( "set_password", QJsonArray() <<  ui->pwdLineEdit->text() ));
    }
    else
    {
        ui->tipLabel->setText(tr("Not consistent!"));
        return;
    }

}


void FirstLogin::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
}



void FirstLogin::on_pwdLineEdit2_returnPressed()
{
    on_createBtn_clicked();
}


void FirstLogin::on_closeBtn_clicked()
{
    emit closeWallet();
}



void FirstLogin::on_pwdLineEdit_textChanged(const QString &arg1)
{
    if( arg1.indexOf(' ') > -1)
    {
        ui->pwdLineEdit->setText( ui->pwdLineEdit->text().remove(' '));
        return;
    }

    ui->tipLabel->clear();
}

void FirstLogin::on_pwdLineEdit2_textChanged(const QString &arg1)
{
    if( arg1.indexOf(' ') > -1)
    {
        ui->pwdLineEdit2->setText( ui->pwdLineEdit2->text().remove(' '));
        return;
    }

    ui->tipLabel->clear();
}


void FirstLogin::jsonDataUpdated(QString id)
{
    if( id == "id-set_password")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << "id-set_password" << result;
        if( result == "\"result\":null" )
        {
            XWCWallet::getInstance()->postRPC( "id-unlock-firstlogin", toJsonFormat( "unlock", QJsonArray() << ui->pwdLineEdit->text() ));
            return;
        }
        else
        {
            emit hideShadowWidget();
            return;
        }

    }

    if( id == "id-unlock-firstlogin")
    {
        emit hideShadowWidget();
        ui->pwdLineEdit->setEnabled(true);
        ui->pwdLineEdit2->setEnabled(true);

        QString result = XWCWallet::getInstance()->jsonDataValue(id);

        if( result == "\"result\":null")
        {
            emit login();
            this->close();
        }
        return;
    }

}


void FirstLogin::keyPressEvent(QKeyEvent *e)
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

void FirstLogin::on_miniBtn_clicked()
{
    emit minimum();
}
