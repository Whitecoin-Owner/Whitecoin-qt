#include "FunctionExchangeWidget.h"
#include "ui_FunctionExchangeWidget.h"

#include "extra/style.h"
#include <QPainter>

FunctionExchangeWidget::FunctionExchangeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FunctionExchangeWidget)
{
    ui->setupUi(this);
    InitWidget();
}

FunctionExchangeWidget::~FunctionExchangeWidget()
{
    delete ui;
}

void FunctionExchangeWidget::retranslator()
{
    ui->retranslateUi(this);
}

void FunctionExchangeWidget::DefaultShow()
{
//    on_exchangeBtn_clicked();
    on_onchainOrderBtn_clicked();
}

void FunctionExchangeWidget::InitWidget()
{
    InitStyle();
    ui->exchangeBtn->setCheckable(true);
    ui->onchainOrderBtn->setCheckable(true);
    ui->contractTokenBtn->setCheckable(true);

    ui->exchangeBtn->hide();
}

void FunctionExchangeWidget::InitStyle()
{
    setStyleSheet(FUNCTIONBAR_PUSHBUTTON_STYLE);
}


void FunctionExchangeWidget::on_exchangeBtn_clicked()
{
    ui->exchangeBtn->setChecked(true);
    ui->onchainOrderBtn->setChecked(false);
    ui->contractTokenBtn->setChecked(false);
    Q_EMIT showExchangeModeSignal();
}


void FunctionExchangeWidget::on_onchainOrderBtn_clicked()
{
    ui->exchangeBtn->setChecked(false);
    ui->onchainOrderBtn->setChecked(true);
    ui->contractTokenBtn->setChecked(false);
    Q_EMIT showOnchainOrderSignal();
}

void FunctionExchangeWidget::on_contractTokenBtn_clicked()
{
    ui->exchangeBtn->setChecked(false);
    ui->onchainOrderBtn->setChecked(false);
    ui->contractTokenBtn->setChecked(true);
    Q_EMIT showContractTokenSignal();
}


void FunctionExchangeWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.setBrush(QBrush(QColor(83,107,215)));
    painter.drawRect(QRect(-1,-1,131,481));

    painter.setPen(QColor(70,90,197));
    painter.drawLine(0,59,130,59);
    painter.drawLine(0,199,130,199);
}

