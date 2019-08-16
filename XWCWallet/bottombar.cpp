#include "bottombar.h"
#include "ui_bottombar.h"


#include "consoledialog.h"
#include "wallet.h"
#include "commondialog.h"


#include <QTimer>
#include <QMovie>
#include <QMouseEvent>

BottomBar::BottomBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BottomBar)
{
    
    ui->setupUi(this);

    ui->nodeNumLabel->setToolTip(tr("Number of connected nodes"));
    ui->syncLabel->setToolTip(tr("Local block height / Network block height"));

    connect(XWCWallet::getInstance(), SIGNAL(jsonDataUpdated(QString)),this, SLOT(jsonDataUpdated(QString)));

    this->setAttribute(Qt::WA_TranslucentBackground);

    ui->syncLabel->installEventFilter(this);
//    ui->networkLabel->setPixmap(QPixmap(":/ui/wallet_ui/net.png").scaled(20,20));
}

BottomBar::~BottomBar()
{
    delete ui;
}


void BottomBar::retranslator()
{
    ui->retranslateUi(this);

    ui->nodeNumLabel->setToolTip(tr("Number of connected nodes"));
    ui->syncLabel->setToolTip(tr("Local block height / Network block height"));
}

void BottomBar::jsonDataUpdated(QString id)
{
    if( id == "id-info")
    {
//        XWCWallet::getInstance()->parseBalance();

        QString result = XWCWallet::getInstance()->jsonDataValue( id);

        if( result.isEmpty() )  return;


        result.prepend("{");
        result.append("}");

        QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
        QJsonObject jsonObject = parse_doucment.object();
        QJsonObject object = jsonObject.take("result").toObject();
//        XWCWallet::getInstance()->walletInfo.blockHeight = object.take("head_block_num").toInt();
        XWCWallet::getInstance()->walletInfo.blockId = object.take("head_block_id").toString();
        XWCWallet::getInstance()->walletInfo.blockAge = object.take("head_block_age").toString();
        XWCWallet::getInstance()->walletInfo.chainId = object.take("chain_id").toString();
        XWCWallet::getInstance()->walletInfo.participation = object.take("participation").toString();

        XWCWallet::getInstance()->postRPC("get_current_block_time",toJsonFormat("get_block",QJsonArray()<<XWCWallet::getInstance()->walletInfo.blockHeight));
//        XWCWallet::getInstance()->walletInfo.activeMiners.clear();
//        QJsonArray array = object.take("active_miners").toArray();
//        foreach (QJsonValue v, array)
//        {
//            XWCWallet::getInstance()->walletInfo.activeMiners += v.toString();
//        }


        return;
    }
    else if("get_current_block_time" == id)
    {
        QString result = XWCWallet::getInstance()->jsonDataValue( id);
        if( result.isEmpty() )  return;


        result.prepend("{");
        result.append("}");

        QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
        QJsonObject jsonObject = parse_doucment.object();
        QJsonObject object = jsonObject.take("result").toObject();

        XWCWallet::getInstance()->currentBlockTime = object.value("timestamp").toString();
    }
    else if("id-network_get_info" == id)
    {
        QString result = XWCWallet::getInstance()->jsonDataValue( id);
        if( result.isEmpty() )
        {
            ui->nodeNumLabel->setText("0");
            return;
        }

        result.prepend("{");
        result.append("}");
        QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
        QJsonObject jsonObject = parse_doucment.object();
        QJsonObject resultObject = jsonObject.value("result").toObject();
        ui->nodeNumLabel->setText(QString::number(resultObject.value("connections").toInt()));
        XWCWallet::getInstance()->walletInfo.blockHeight = resultObject.value("current_block_height").toInt();
        XWCWallet::getInstance()->walletInfo.targetBlockHeight = resultObject.value("target_block_height").toInt();

        ui->syncLabel->setText(QString::number(XWCWallet::getInstance()->walletInfo.blockHeight) + " / "
                               + QString::number(XWCWallet::getInstance()->walletInfo.targetBlockHeight));

        CheckBlockSync();

        Q_EMIT walletInfoUpdated();
    }


}

void BottomBar::CheckBlockSync()
{
    if(XWCWallet::getInstance()->walletInfo.blockHeight == 0 || !XWCWallet::getInstance()->walletInfo.blockAge.contains("second"))
    {
        XWCWallet::getInstance()->SetBlockSyncFinish(false);
        return;
    }

    if(XWCWallet::getInstance()->walletInfo.blockHeight + 50 > XWCWallet::getInstance()->walletInfo.targetBlockHeight)
    {
        XWCWallet::getInstance()->SetBlockSyncFinish(true);
    }
    else
    {
        XWCWallet::getInstance()->SetBlockSyncFinish(false);
    }

}

void BottomBar::paintEvent(QPaintEvent *)
{
//    QPainter painter(this);
//    painter.setBrush(QColor(248,249,253));
//    painter.setPen(QColor(248,249,253));
//    painter.drawRect(rect());
//    //painter.drawRect(QRect(0,0,900,40));

//    if(rect().width() > 680)
//    {
//        painter.setBrush(QColor(24,28,45));
//        painter.setPen(QColor(24,28,45));
//        painter.drawRect(0,0,124,40);
    //    }
}

bool BottomBar::eventFilter(QObject *watched, QEvent *e)
{
    if( watched == ui->syncLabel)
    {
        if( e->type() == QEvent::MouseButtonPress)
        {
//            if(!XWCWallet::getInstance()->GetBlockSyncFinish())
//            {
                Q_EMIT showCoverWidget();
//            }
            return true;
        }
        return false;
    }
}

void BottomBar::refresh()
{
    XWCWallet::getInstance()->postRPC( "id-info", toJsonFormat( "info", QJsonArray()));

    XWCWallet::getInstance()->postRPC( "id-network_get_info", toJsonFormat( "network_get_info", QJsonArray()));


}
