#include "AssetIconItem.h"
#include "ui_AssetIconItem.h"

#include "wallet.h"


AssetIconItem::AssetIconItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AssetIconItem)
{
    ui->setupUi(this);

    setBackgroundColor("white");
}

AssetIconItem::~AssetIconItem()
{
    delete ui;
}

void AssetIconItem::setAsset(QString assetSymbol)
{
    ui->iconLabel->setStyleSheet(QString("border-image: url(:/ui/wallet_ui/coin_%1.png);").arg(assetSymbol));
    ui->assetLabel->setStyleSheet("QLabel{font: 11px \"Microsoft YaHei UI Light\";color:rgb(52,37,90);}");
    ui->assetLabel->setText(revertERCSymbol(assetSymbol));
}

void AssetIconItem::setBackgroundColor(QString color)
{
    setStyleSheet(QString("#widget{background-color:%1;}").arg(color));
}
