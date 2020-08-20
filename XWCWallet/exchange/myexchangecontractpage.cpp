#include "myexchangecontractpage.h"
#include "ui_myexchangecontractpage.h"

#include <QtMath>

#include "wallet.h"
#include "commondialog.h"
#include "selldialog.h"
#include "contractbalancewidget.h"
#include "ToolButtonWidget.h"
#include "WithdrawOrderDialog.h"
#include "depositpage/FeeChargeWidget.h"
#include "dialog/ErrorResultDialog.h"
#include "dialog/TransactionResultDialog.h"
#include "control/BlankDefaultWidget.h"
#include "poundage/PageScrollWidget.h"

static const int ROWNUMBER = 6;
MyExchangeContractPage::MyExchangeContractPage(QWidget *parent) :
    QWidget(parent),
    currentWidget(NULL),
    ui(new Ui::MyExchangeContractPage)
{
    ui->setupUi(this);

    connect( XWCWallet::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    ui->ordersTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->ordersTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->ordersTableWidget->setFocusPolicy(Qt::NoFocus);
//    ui->ordersTableWidget->setFrameShape(QFrame::NoFrame);
    ui->ordersTableWidget->setMouseTracking(true);
    ui->ordersTableWidget->setShowGrid(false);//隐藏表格线
    ui->ordersTableWidget->horizontalHeader()->setSectionsClickable(true);
//    ui->ordersTableWidget->horizontalHeader()->setFixedHeight(35);
    ui->ordersTableWidget->horizontalHeader()->setVisible(true);
    ui->ordersTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->ordersTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->ordersTableWidget->verticalHeader()->setVisible(false);
    ui->ordersTableWidget->setColumnWidth(0,160);
    ui->ordersTableWidget->setColumnWidth(1,160);
    ui->ordersTableWidget->setColumnWidth(2,150);
    ui->ordersTableWidget->setColumnWidth(3,80);
    ui->ordersTableWidget->setColumnWidth(4,80);

    ui->ordersTableWidget->setStyleSheet(TABLEWIDGET_STYLE_1);

    ui->balanceBtn->setStyleSheet(TOOLBUTTON_STYLE_1);
    ui->sellBtn->setStyleSheet(TOOLBUTTON_STYLE_1);
    ui->withdrawAllBtn->setStyleSheet(TOOLBUTTON_STYLE_1);
    ui->registerBtn->setStyleSheet(TOOLBUTTON_STYLE_1);

    pageWidget = new PageScrollWidget();
    ui->stackedWidget->addWidget(pageWidget);
    connect(pageWidget,&PageScrollWidget::currentPageChangeSignal,this,&MyExchangeContractPage::pageChangeSlot);

    blankWidget = new BlankDefaultWidget(ui->ordersTableWidget);
    blankWidget->setTextTip(tr("There are no orders!"));


    XWCWallet::getInstance()->mainFrame->installBlurEffect(ui->ordersTableWidget);
    init();
}

MyExchangeContractPage::~MyExchangeContractPage()
{
    delete ui;
}

void MyExchangeContractPage::init()
{
    QStringList accounts = XWCWallet::getInstance()->accountInfoMap.keys();
    ui->accountComboBox->addItems(accounts);

    if(accounts.contains(XWCWallet::getInstance()->currentAccount))
    {
        ui->accountComboBox->setCurrentText(XWCWallet::getInstance()->currentAccount);
    }

    QStringList assetIds = XWCWallet::getInstance()->assetInfoMap.keys();
    foreach (QString assetId, assetIds)
    {
        ui->assetComboBox->addItem( revertERCSymbol( XWCWallet::getInstance()->assetInfoMap.value(assetId).symbol), assetId);
        ui->assetComboBox2->addItem( revertERCSymbol( XWCWallet::getInstance()->assetInfoMap.value(assetId).symbol), assetId);
    }

    if(assetIds.contains(XWCWallet::getInstance()->currentSellAssetId))
    {
        ui->assetComboBox->setCurrentText( revertERCSymbol( XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->currentSellAssetId).symbol));
    }

    if(assetIds.contains(XWCWallet::getInstance()->currentBuyAssetId))
    {
        ui->assetComboBox2->setCurrentText( revertERCSymbol( XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->currentBuyAssetId).symbol));
    }

    inited = true;

    on_accountComboBox_currentIndexChanged(ui->accountComboBox->currentText());
}

void MyExchangeContractPage::refresh()
{
    on_accountComboBox_currentIndexChanged(ui->accountComboBox->currentText());

    if( currentWidget != NULL && currentWidget->objectName() == "contractBalanceWidget")
    {
        static_cast<ContractBalanceWidget*>(currentWidget)->refresh();
    }
}

void MyExchangeContractPage::onBack()
{
    if(currentWidget)
    {
        currentWidget->close();
        currentWidget = NULL;
    }
}

void MyExchangeContractPage::showOrders()
{
    SellOrders orders = accountSellOrdersMap.value(ui->accountComboBox->currentText());
    QVector<OrderAmount> vector = orders.value( getRealAssetSymbol( ui->assetComboBox->currentText()) + "," + getRealAssetSymbol( ui->assetComboBox2->currentText()));

    AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(ui->assetComboBox->currentData().toString());
    AssetInfo assetInfo2 = XWCWallet::getInstance()->assetInfoMap.value(ui->assetComboBox2->currentData().toString());

    int size = vector.size();
    ui->ordersTableWidget->setRowCount(0);
    ui->ordersTableWidget->setRowCount(size);

    for(int i = 0; i < size; i++)
    {
        ui->ordersTableWidget->setRowHeight(i,42);

        OrderAmount amount = vector.at(i);

        ui->ordersTableWidget->setItem(i,0, new QTableWidgetItem(getBigNumberString(amount.sellAmount, assetInfo.precision)));
        ui->ordersTableWidget->setItem(i,1, new QTableWidgetItem(getBigNumberString(amount.buyAmount, assetInfo2.precision)));

        double price = ((double)amount.sellAmount / qPow(10,assetInfo.precision)) / (amount.buyAmount / qPow(10,assetInfo2.precision) );
        ui->ordersTableWidget->setItem(i,2, new QTableWidgetItem(QString::number(price,'g',12)));
        ui->ordersTableWidget->setItem(i,3, new QTableWidgetItem(tr("cancel")));
        ui->ordersTableWidget->setItem(i,4, new QTableWidgetItem(tr("add")));

        for(int j = 3;j < 5;++j)
        {
            ToolButtonWidgetItem *toolButtonItem = new ToolButtonWidgetItem(i,j);
            toolButtonItem->setText(ui->ordersTableWidget->item(i,j)->text());
            ui->ordersTableWidget->setCellWidget(i,j,toolButtonItem);
            connect(toolButtonItem,SIGNAL(itemClicked(int,int)),this,SLOT(onItemClicked(int,int)));
        }
    }
    tableWidgetSetItemZebraColor(ui->ordersTableWidget);

    int page = (ui->ordersTableWidget->rowCount()%ROWNUMBER==0 && ui->ordersTableWidget->rowCount() != 0) ?
                ui->ordersTableWidget->rowCount()/ROWNUMBER : ui->ordersTableWidget->rowCount()/ROWNUMBER+1;
    pageWidget->SetTotalPage(page);
    pageWidget->setShowTip(ui->ordersTableWidget->rowCount(),ROWNUMBER);
    pageChangeSlot(pageWidget->GetCurrentPage());
    pageWidget->setVisible(0 != size);
    blankWidget->setVisible(0 == size);

}

void MyExchangeContractPage::updateTableHeaders()
{
    if(ui->assetComboBox->currentText() == ui->assetComboBox2->currentText())
    {
        ui->ordersTableWidget->horizontalHeaderItem(0)->setText(tr("SELL"));
        ui->ordersTableWidget->horizontalHeaderItem(1)->setText(tr("BUY"));
        ui->ordersTableWidget->horizontalHeaderItem(2)->setText(tr("PRICE"));
    }
    else
    {
        ui->ordersTableWidget->horizontalHeaderItem(0)->setText(tr("SELL / %1").arg(ui->assetComboBox->currentText()));
        ui->ordersTableWidget->horizontalHeaderItem(1)->setText(tr("BUY / %1").arg(ui->assetComboBox2->currentText()));
        ui->ordersTableWidget->horizontalHeaderItem(2)->setText(tr("PRICE (%1/%2)").arg(ui->assetComboBox->currentText()).arg(ui->assetComboBox2->currentText()));
    }
}

void MyExchangeContractPage::fetchAccountExchangeContractBalances()
{
    QString contractAddress = XWCWallet::getInstance()->getExchangeContractAddress(ui->accountComboBox->currentText());

    if(!contractAddress.isEmpty())
    {
        XWCWallet::getInstance()->postRPC( "id-invoke_contract_offline-owner_assets-" + ui->accountComboBox->currentText(), toJsonFormat( "invoke_contract_offline",
                                                                               QJsonArray() << ui->accountComboBox->currentText() << contractAddress
                                                                               << "owner_assets"  << ""));
    }
}


void MyExchangeContractPage::jsonDataUpdated(QString id)
{
    if( id.startsWith("id-invoke_contract_offline-owner_assets-") )
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        QString accountName = id.mid(QString("id-invoke_contract_offline-owner_assets-").size());

        if(result.startsWith("\"result\":"))
        {
            result.prepend("{");
            result.append("}");

            QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
            QJsonObject jsonObject = parse_doucment.object();
            QJsonObject object = QJsonDocument::fromJson(jsonObject.take("result").toString().toLatin1()).object();

            ExchangeContractBalances balances;
            foreach (QString key, object.keys())
            {
                QJsonValue amountValue = object.take(key);
                unsigned long long amount = 0;

                amount = jsonValueToULL(amountValue);

                balances.insert(key,amount);
            }

            XWCWallet::getInstance()->accountExchangeContractBalancesMap.insert(accountName,balances);
        }

        return;
    }

    if( id == "id-register_contract")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            ui->registerBtn->setVisible(false);
            TransactionResultDialog transactionResultDialog;
            transactionResultDialog.setInfoText(tr("Create exchange contract successfully! Please wait for the confirmation of the block chain. Please do not repeat the creation of the contract."));
            transactionResultDialog.setDetailText(result);
            transactionResultDialog.pop();
        }
        else if(result.startsWith("\"error\":"))
        {            
            ui->registerBtn->setVisible(true);
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Fail to register exchange contract!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }

        return;
    }


    if( id.startsWith("id-invoke_contract_offline-sell_orders-"))
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        QString accountName = id.mid(QString("id-invoke_contract_offline-sell_orders-").size());

        if(result.startsWith("\"result\":"))
        {
            result.prepend("{");
            result.append("}");

            QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
            QJsonObject jsonObject = parse_doucment.object();
            QJsonObject object = QJsonDocument::fromJson(jsonObject.take("result").toString().toLatin1()).object();

            SellOrders orders;
            foreach (QString key, object.keys())
            {
                QJsonObject object2 = QJsonDocument::fromJson(object.take(key).toString().toLatin1()).object();
                QJsonArray orderArray = object2.take("orderArray").toArray();

                QVector<OrderAmount> vector;
                foreach (QJsonValue v, orderArray)
                {
                    QString amountStr = v.toString();
                    OrderAmount amount;
                    amount.sellAmount = amountStr.split(",").at(0).toULongLong();
                    amount.buyAmount = amountStr.split(",").at(1).toULongLong();
                    vector.append(amount);
                }

                orders.insert(key,vector);
            }

            accountSellOrdersMap.insert(accountName,orders);
        }

        showOrders();

        return;
    }

    if( id.startsWith("id-invoke_contract_testing-cancelSellOrder-") )
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;
        if(result.startsWith("\"result\":"))
        {

            if(!XWCWallet::getInstance()->ValidateOnChainOperation()) return;

            XWCWallet::TotalContractFee totalFee = XWCWallet::getInstance()->parseTotalContractFee(result);
            int stepCount = totalFee.step;
            unsigned long long totalAmount = totalFee.baseAmount + ceil(totalFee.step * XWCWallet::getInstance()->contractFee / 100.0);

            WithdrawOrderDialog withdrawOrderDialog;
            withdrawOrderDialog.setContractFee(getBigNumberString(totalAmount, ASSET_PRECISION).toDouble());
            withdrawOrderDialog.setAccountName(ui->accountComboBox->currentText());
            withdrawOrderDialog.setText(tr("Sure to cancel this order? You need to pay the fee for contract execution."));
            if(withdrawOrderDialog.pop())
            {
                QString contractAddress = XWCWallet::getInstance()->getExchangeContractAddress(ui->accountComboBox->currentText());

                QString params = id.mid(QString("id-invoke_contract_testing-cancelSellOrder-").size());
                withdrawOrderDialog.updatePoundageID();
                XWCWallet::getInstance()->postRPC( "id-invoke_contract-cancelSellOrder", toJsonFormat( "invoke_contract",
                                                                                       QJsonArray() << ui->accountComboBox->currentText()
                                                                                       << XWCWallet::getInstance()->currentContractFee() << stepCount
                                                                                       << contractAddress
                                                                                       << "cancelSellOrder"  << params));
            }



        }

        return;
    }

    if( id == "id-invoke_contract-cancelSellOrder" )
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;
        if(result.startsWith("\"result\":"))
        {
            TransactionResultDialog transactionResultDialog;
            transactionResultDialog.setInfoText(tr("Transaction of cancel-order has been sent out!"));
            transactionResultDialog.setDetailText(result);
            transactionResultDialog.pop();
        }
        else if(result.startsWith("\"error\":"))
        {
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Fail to cancel sell-order!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }


        return;
    }

    if( id.startsWith("id-invoke_contract_testing-cancelSellOrderPair-") )
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;
        if(result.startsWith("\"result\":"))
        {

            if(!XWCWallet::getInstance()->ValidateOnChainOperation()) return;

            XWCWallet::TotalContractFee totalFee = XWCWallet::getInstance()->parseTotalContractFee(result);
            int stepCount = totalFee.step;
            unsigned long long totalAmount = totalFee.baseAmount + ceil(totalFee.step * XWCWallet::getInstance()->contractFee / 100.0);

            WithdrawOrderDialog withdrawOrderDialog;
            withdrawOrderDialog.setContractFee(getBigNumberString(totalAmount, ASSET_PRECISION).toDouble());
            withdrawOrderDialog.setAccountName(ui->accountComboBox->currentText());
            withdrawOrderDialog.setText(tr("You need to pay the fee for contract execution."));
            qDebug()<<totalAmount;
            if(withdrawOrderDialog.pop())
            {
                QString contractAddress = XWCWallet::getInstance()->getExchangeContractAddress(ui->accountComboBox->currentText());
                QString params = id.mid(QString("id-invoke_contract_testing-cancelSellOrderPair-").size());

                withdrawOrderDialog.updatePoundageID();
                XWCWallet::getInstance()->postRPC( "id-invoke_contract-cancelSellOrderPair", toJsonFormat( "invoke_contract",
                                                                                                         QJsonArray() << ui->accountComboBox->currentText()
                                                                                                         << XWCWallet::getInstance()->currentContractFee() << stepCount
                                                                                                         << contractAddress
                                                                                                         << "cancelSellOrderPair"  << params));


            }

        }

        return;
    }

    if( id == "id-invoke_contract-cancelSellOrderPair" )
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;
        if(result.startsWith("\"result\":"))
        {
            TransactionResultDialog transactionResultDialog;
            transactionResultDialog.setInfoText(tr("Transaction of cancel-order-pair has been sent out!"));
            transactionResultDialog.setDetailText(result);
            transactionResultDialog.pop();
        }
        else if(result.startsWith("\"error\":"))
        {
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Fail to cancel sell-order pair!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }

        return;
    }

    if("contract-register_contract_testing" == id)
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            XWCWallet::TotalContractFee totalFee = XWCWallet::getInstance()->parseTotalContractFee(result);
            int stepCount = totalFee.step;

            unsigned long long totalAmount = totalFee.baseAmount + ceil(totalFee.step * XWCWallet::getInstance()->contractFee / 100.0);

            qDebug()<<totalAmount;

            FeeChargeWidget *fee = new FeeChargeWidget( getBigNumberString(totalAmount, ASSET_PRECISION).toDouble(),XWCWallet::getInstance()->feeType,
                                                         ui->accountComboBox->currentText(),XWCWallet::getInstance()->mainFrame);
            //fee->SetTitle(tr("Register Contract"));
            fee->SetInfo(tr("register contract!"));
            connect(fee,&FeeChargeWidget::confirmSignal,[this,stepCount,fee](){
                QString filePath = "contracts/exchange.lua.gpc";
                QFileInfo fileInfo(filePath);
                if(fileInfo.exists())
                {
                    fee->updatePoundageID();
                    qDebug()<<"set poundage";
                    XWCWallet::getInstance()->postRPC( "id-register_contract", toJsonFormat( "register_contract",
                                                                                           QJsonArray() << this->ui->accountComboBox->currentText() << XWCWallet::getInstance()->currentContractFee()
                                                                                           << stepCount  << filePath));
                }

            });

            fee->show();
        }
        else
        {
//            CommonDialog dia(CommonDialog::OkOnly);
//            dia.setText(result);
//            dia.pop();
        }
    }
}


void MyExchangeContractPage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(239,242,245),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(239,242,245),Qt::SolidPattern));

    painter.drawRect(rect());
}

void MyExchangeContractPage::registerContract()
{
    // 如果还没有兑换合约  先注册
    CommonDialog commonDialog(CommonDialog::OkAndCancel);
    commonDialog.setText(tr("You don't have an exchange contract at the moment. Will you create it?"));
    if(commonDialog.pop())
    {
        QString filePath = "contracts/exchange.lua.gpc";
        QFileInfo fileInfo(filePath);
        if(fileInfo.exists())
        {
            //查询注册合约费用
            XWCWallet::getInstance()->postRPC("contract-register_contract_testing",toJsonFormat( "register_contract_testing",
                                                                                               QJsonArray() << ui->accountComboBox->currentText() << filePath));

        }
        else
        {
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText(tr("Can not find file contracts/exchange.lua.gpc!"));
            commonDialog.pop();
        }
    }
}

void MyExchangeContractPage::on_accountComboBox_currentIndexChanged(const QString &arg1)
{
    if(!inited)     return;

    XWCWallet::getInstance()->currentAccount = ui->accountComboBox->currentText();
    QString contractAddress = XWCWallet::getInstance()->getExchangeContractAddress(ui->accountComboBox->currentText());

    ui->registerBtn->setVisible(contractAddress.isEmpty());
    if(contractAddress.isEmpty())
    {
        ui->ordersTableWidget->setRowCount(0);
    }
    else
    {
        XWCWallet::getInstance()->postRPC( "id-invoke_contract_offline-sell_orders-" + ui->accountComboBox->currentText(), toJsonFormat( "invoke_contract_offline",
                                                                               QJsonArray() << ui->accountComboBox->currentText() << contractAddress
                                                                               << "sell_orders"  << ""));

        fetchAccountExchangeContractBalances();
    }

}

void MyExchangeContractPage::on_assetComboBox_currentIndexChanged(const QString &arg1)
{
    if(!inited)     return;

    XWCWallet::getInstance()->currentSellAssetId = ui->assetComboBox->currentData().toString();
    updateTableHeaders();
    if(ui->assetComboBox->currentText() == ui->assetComboBox2->currentText())   return;

    showOrders();
}

void MyExchangeContractPage::on_assetComboBox2_currentIndexChanged(const QString &arg1)
{
    if(!inited)     return;

    XWCWallet::getInstance()->currentBuyAssetId = ui->assetComboBox2->currentData().toString();
    updateTableHeaders();
    if(ui->assetComboBox->currentText() == ui->assetComboBox2->currentText())   return;

    showOrders();
}

void MyExchangeContractPage::on_sellBtn_clicked()
{
    if(!XWCWallet::getInstance()->ValidateOnChainOperation()) return;

    QString contractAddress = XWCWallet::getInstance()->getExchangeContractAddress(ui->accountComboBox->currentText());

    if(contractAddress.isEmpty())
    {
        registerContract();
    }
    else
    {
        SellDialog sellDialog;
        sellDialog.setSellAsset( getRealAssetSymbol( ui->assetComboBox->currentText()));
        sellDialog.setBuyAsset( getRealAssetSymbol( ui->assetComboBox2->currentText()));
        sellDialog.pop();

        if(sellDialog.goToDeposit)
        {
            on_balanceBtn_clicked();
        }
    }


}

void MyExchangeContractPage::on_registerBtn_clicked()
{
    if(!XWCWallet::getInstance()->ValidateOnChainOperation()) return;

    QString contractAddress = XWCWallet::getInstance()->getExchangeContractAddress(ui->accountComboBox->currentText());

    if(contractAddress.isEmpty())
    {
        QString filePath = "contracts/exchange.lua.gpc";
        QFileInfo fileInfo(filePath);
        if(fileInfo.exists())
        {
            //查询注册合约费用
            XWCWallet::getInstance()->postRPC("contract-register_contract_testing",toJsonFormat( "register_contract_testing",
                                                                                               QJsonArray() << ui->accountComboBox->currentText() << filePath));


        }
        else
        {
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText(tr("Can not find file contracts/exchange.lua.gpc!"));
            commonDialog.pop();
        }
    }
}

void MyExchangeContractPage::on_balanceBtn_clicked()
{
    QString contractAddress = XWCWallet::getInstance()->getExchangeContractAddress(ui->accountComboBox->currentText());

    if(contractAddress.isEmpty())
    {
        if(!XWCWallet::getInstance()->ValidateOnChainOperation()) return;
        registerContract();
    }
    else
    {
        ContractBalanceWidget* contractBalanceWidget = new ContractBalanceWidget(this);
        contractBalanceWidget->setAttribute(Qt::WA_DeleteOnClose);
        contractBalanceWidget->setObjectName("contractBalanceWidget");
        contractBalanceWidget->setAccount(ui->accountComboBox->currentText());
        contractBalanceWidget->move(0,0);
        contractBalanceWidget->show();

        currentWidget = contractBalanceWidget;

        emit backBtnVisible(true);
    }
}



void MyExchangeContractPage::on_withdrawAllBtn_clicked()
{
    if(ui->ordersTableWidget->rowCount() < 1)   return;

    if(!XWCWallet::getInstance()->ValidateOnChainOperation()) return;

    CommonDialog commonDialog(CommonDialog::OkAndCancel);
    commonDialog.setText( tr("Sure to cancel all orders of %1-to-%2 ?").arg(ui->assetComboBox->currentText()).arg(ui->assetComboBox2->currentText()) );
    if(commonDialog.pop())
    {
        QString contractAddress = XWCWallet::getInstance()->getExchangeContractAddress(ui->accountComboBox->currentText());
        QString params = QString("%1,%2").arg( getRealAssetSymbol( ui->assetComboBox->currentText())).arg( getRealAssetSymbol( ui->assetComboBox2->currentText()));

        XWCWallet::getInstance()->postRPC( "id-invoke_contract_testing-cancelSellOrderPair-" + params, toJsonFormat( "invoke_contract_testing",
                                                                                                 QJsonArray() << ui->accountComboBox->currentText()
                                                                                                 << contractAddress
                                                                                                 << "cancelSellOrderPair"  << params));
    }
}

void MyExchangeContractPage::onItemClicked(int _row, int _column)
{
    if(_column == 3)
    {
        if(!XWCWallet::getInstance()->ValidateOnChainOperation()) return;
        // 撤销挂单
        QString contractAddress = XWCWallet::getInstance()->getExchangeContractAddress(ui->accountComboBox->currentText());
        AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(ui->assetComboBox->currentData().toString());
        AssetInfo assetInfo2 = XWCWallet::getInstance()->assetInfoMap.value(ui->assetComboBox2->currentData().toString());

        QString params = QString("%1,%2,%3,%4").arg(assetInfo.symbol).arg(decimalToIntegerStr(ui->ordersTableWidget->item(_row,0)->text(), assetInfo.precision))
                .arg(assetInfo2.symbol).arg(decimalToIntegerStr(ui->ordersTableWidget->item(_row,1)->text(), assetInfo2.precision));


        XWCWallet::getInstance()->postRPC( "id-invoke_contract_testing-cancelSellOrder-" + params, toJsonFormat( "invoke_contract_testing",
                                                                                                               QJsonArray() << ui->accountComboBox->currentText()
                                                                                                               << contractAddress
                                                                                                               << "cancelSellOrder"  << params));


        return;
    }

    if(_column == 4)
    {
        if(!XWCWallet::getInstance()->ValidateOnChainOperation()) return;
        SellDialog sellDialog;
        sellDialog.setSellAsset( getRealAssetSymbol( ui->assetComboBox->currentText()));
        sellDialog.setBuyAsset( getRealAssetSymbol( ui->assetComboBox2->currentText()));
        sellDialog.pop();

        if(sellDialog.goToDeposit)
        {
            on_balanceBtn_clicked();
        }

        return;
    }
}

void MyExchangeContractPage::pageChangeSlot(unsigned int page)
{
    for(int i = 0;i < ui->ordersTableWidget->rowCount();++i)
    {
        if(i < page*ROWNUMBER)
        {
            ui->ordersTableWidget->setRowHidden(i,true);
        }
        else if(page * ROWNUMBER <= i && i < page*ROWNUMBER + ROWNUMBER)
        {
            ui->ordersTableWidget->setRowHidden(i,false);
        }
        else
        {
            ui->ordersTableWidget->setRowHidden(i,true);
        }
    }

}

void MyExchangeContractPage::on_swapBtn_clicked()
{
    inited = false;
    QString temp = ui->assetComboBox->currentText();
    ui->assetComboBox->setCurrentText(ui->assetComboBox2->currentText());
    XWCWallet::getInstance()->currentSellAssetId = ui->assetComboBox->currentData().toString();
    inited = true;
    ui->assetComboBox2->setCurrentText(temp);
}

void MyExchangeContractPage::on_onchainOrderBtn_clicked()
{
    Q_EMIT showOnchainOrderPage();
}
