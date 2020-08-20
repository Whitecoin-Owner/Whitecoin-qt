#include "FunctionCitizenWidget.h"
#include "ui_FunctionCitizenWidget.h"

#include "wallet.h"

FunctionCitizenWidget::FunctionCitizenWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FunctionCitizenWidget)
{
    ui->setupUi(this);
    InitWidget();
}

FunctionCitizenWidget::~FunctionCitizenWidget()
{
    delete ui;
}

void FunctionCitizenWidget::retranslator()
{
    ui->retranslateUi(this);
}

void FunctionCitizenWidget::DefaultShow()
{
    on_accountInfoBtn_clicked();
}

void FunctionCitizenWidget::InitWidget()
{
    InitStyle();
    ui->accountInfoBtn->setCheckable(true);
    ui->proposalBtn->setCheckable(true);
    ui->policyBtn->setCheckable(true);
}

void FunctionCitizenWidget::InitStyle()
{
    setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(84,116,235));
    setPalette(palette);

    setStyleSheet(FUNCTIONBAR_PUSHBUTTON_STYLE);
}

void FunctionCitizenWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.setBrush(QBrush(QColor(83,107,215)));
    painter.drawRect(QRect(-1,-1,131,481));

    painter.setPen(QColor(70,90,197));
    painter.drawLine(0,59,130,59);
}


void FunctionCitizenWidget::on_accountInfoBtn_clicked()
{
    ui->accountInfoBtn->setChecked(true);
    ui->proposalBtn->setChecked(false);
    ui->policyBtn->setChecked(false);
    showCitizenAccountSignal();
}

void FunctionCitizenWidget::on_proposalBtn_clicked()
{
    ui->accountInfoBtn->setChecked(false);
    ui->proposalBtn->setChecked(true);
    ui->policyBtn->setChecked(false);
    showCitizenProposalSignal();
}

void FunctionCitizenWidget::on_policyBtn_clicked()
{
    ui->accountInfoBtn->setChecked(false);
    ui->proposalBtn->setChecked(false);
    ui->policyBtn->setChecked(true);
    showCitizenPolicySignal();
}

