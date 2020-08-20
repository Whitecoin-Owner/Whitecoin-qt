#include "PasswordConfirmWidget.h"
#include "ui_PasswordConfirmWidget.h"

#include <QPainter>
#include "wallet.h"
PasswordConfirmWidget::PasswordConfirmWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PasswordConfirmWidget)
{
    ui->setupUi(this);
    InitWidget();
}

PasswordConfirmWidget::~PasswordConfirmWidget()
{
    delete ui;
}

void PasswordConfirmWidget::ConfirmSlots()
{
    emit confirmSignal();
    close();
}

void PasswordConfirmWidget::CancelSlots()
{
    emit cancelSignal();
    close();
}

void PasswordConfirmWidget::passwordChangeSlots(const QString &address)
{
    ui->toolButton_confirm->setEnabled(false);
    if( ui->lineEdit->text().size() < 8)
    {
        ui->lineEdit->setStyleSheet("color:red");
        return;
    }

    XWCWallet::getInstance()->postRPC( "captial_unlock-lockpage", toJsonFormat( "unlock", QJsonArray() << ui->lineEdit->text() ));

}

void PasswordConfirmWidget::jsonDataUpdated(QString id)
{
    if( id == "captial_unlock-lockpage")
    {
        QString  result = XWCWallet::getInstance()->jsonDataValue(id);
        if( result == "\"result\":null")
        {
            ui->lineEdit->setStyleSheet("color:#5474EB");
            ui->toolButton_confirm->setEnabled(true);
        }
        else
        {
            ui->lineEdit->setStyleSheet("color:red");
            ui->toolButton_confirm->setEnabled(false);
        }
    }
}

void PasswordConfirmWidget::passwordReturnPressed()
{
    if(ui->toolButton_confirm->isEnabled())
    {
        ConfirmSlots();
    }
}

void PasswordConfirmWidget::InitWidget()
{
    InitStyle();
    ui->lineEdit->setFocus();
    ui->toolButton_confirm->setEnabled(false);
    ui->lineEdit->setPlaceholderText(tr("please input password..."));
    connect( XWCWallet::getInstance(), &XWCWallet::jsonDataUpdated, this, &PasswordConfirmWidget::jsonDataUpdated);
    connect(ui->toolButton_confirm,&QToolButton::clicked,this,&PasswordConfirmWidget::ConfirmSlots);
    connect(ui->toolButton_cancel,&QToolButton::clicked,this,&PasswordConfirmWidget::CancelSlots);
    connect(ui->lineEdit,&QLineEdit::textEdited,this,&PasswordConfirmWidget::passwordChangeSlots);
    connect(ui->lineEdit,&QLineEdit::returnPressed,this,&PasswordConfirmWidget::passwordReturnPressed);

}

void PasswordConfirmWidget::InitStyle()
{
    setAttribute(Qt::WA_TranslucentBackground, true);

//    setAutoFillBackground(true);
//    QPalette palette;
//    palette.setColor(QPalette::Window, QColor(10,10,10,100));
//    setPalette(palette);

    ui->toolButton_confirm->setStyleSheet(OKBTN_STYLE);
    ui->toolButton_cancel->setStyleSheet(CLOSEBTN_STYLE);

}

void PasswordConfirmWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.drawPixmap(rect(),QPixmap(":/ui/wallet_ui/back_dialog.png").scaled(rect().size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation));

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(239,242,245,255));
    painter.drawRoundedRect(QRect(300,170,360,185),5,5);


    QWidget::paintEvent(event);
}
