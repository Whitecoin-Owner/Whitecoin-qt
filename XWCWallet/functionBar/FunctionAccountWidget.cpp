#include "FunctionAccountWidget.h"
#include "ui_FunctionAccountWidget.h"

#include "extra/style.h"
#include <QPainter>
#include <QDebug>

FunctionAccountWidget::FunctionAccountWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FunctionAccountWidget)
{
    ui->setupUi(this);
    InitWidget();
}

FunctionAccountWidget::~FunctionAccountWidget()
{
    delete ui;
}
void FunctionAccountWidget::retranslator()
{
    ui->retranslateUi(this);
}
void FunctionAccountWidget::DefaultShow()
{
    AssetShowSlots();
}

void FunctionAccountWidget::AssetShowSlots()
{
    ui->pushButton_miner->setChecked(false);
    ui->pushButton_asset->setChecked(true);
    ui->pushButton_bonus->setChecked(false);
    ui->pushButton_record->setChecked(false);

    emit showAccountSignal();
}

void FunctionAccountWidget::MinerShowSlots()
{
    ui->pushButton_miner->setChecked(true);
    ui->pushButton_asset->setChecked(false);
    ui->pushButton_bonus->setChecked(false);
    ui->pushButton_record->setChecked(false);

    emit showMinerSignal();
}

void FunctionAccountWidget::BonusShowSlots()
{
    ui->pushButton_miner->setChecked(false);
    ui->pushButton_asset->setChecked(false);
    ui->pushButton_bonus->setChecked(true);
    ui->pushButton_record->setChecked(false);

    emit showBonusSignal();
}

void FunctionAccountWidget::RecordShowSlots()
{
    ui->pushButton_miner->setChecked(false);
    ui->pushButton_asset->setChecked(false);
    ui->pushButton_bonus->setChecked(false);
    ui->pushButton_record->setChecked(true);

    emit showRecordSignal();
}

void FunctionAccountWidget::InitWidget()
{
    InitStyle();
    ui->pushButton_asset->setCheckable(true);
    ui->pushButton_miner->setCheckable(true);
    ui->pushButton_bonus->setCheckable(true);
    ui->pushButton_record->setCheckable(true);

    connect(ui->pushButton_asset,&QPushButton::clicked,this,&FunctionAccountWidget::AssetShowSlots);
    connect(ui->pushButton_miner,&QPushButton::clicked,this,&FunctionAccountWidget::MinerShowSlots);
    connect(ui->pushButton_bonus,&QPushButton::clicked,this,&FunctionAccountWidget::BonusShowSlots);
    connect(ui->pushButton_record,&QPushButton::clicked,this,&FunctionAccountWidget::RecordShowSlots);
}

void FunctionAccountWidget::InitStyle()
{
    setStyleSheet(FUNCTIONBAR_PUSHBUTTON_STYLE );
}

void FunctionAccountWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.setBrush(QBrush(QColor(83,107,215)));
    painter.drawRect(QRect(-1,-1,131,481));

    painter.setPen(QColor(70,90,197));
    painter.drawLine(0,59,130,59);
}
