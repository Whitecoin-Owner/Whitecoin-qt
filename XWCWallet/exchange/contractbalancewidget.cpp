#include "contractbalancewidget.h"
#include "ui_contractbalancewidget.h"

#include "wallet.h"
#include "commondialog.h"
#include "depositexchangecontractdialog.h"
#include "withdrawexchangecontractdialog.h"
#include "ToolButtonWidget.h"
#include "WithdrawOrderDialog.h"
#include "dialog/ErrorResultDialog.h"
#include "dialog/TransactionResultDialog.h"
#include "control/BlankDefaultWidget.h"
#include "poundage/PageScrollWidget.h"


static const int ROWNUMBER = 8;
ContractBalanceWidget::ContractBalanceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContractBalanceWidget)
{
    ui->setupUi(this);

    connect( XWCWallet::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    ui->balancesTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->balancesTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->balancesTableWidget->setFocusPolicy(Qt::NoFocus);
//    ui->balancesTableWidget->setFrameShape(QFrame::NoFrame);
    ui->balancesTableWidget->setMouseTracking(true);
    ui->balancesTableWidget->setShowGrid(false);//隐藏表格线
    ui->balancesTableWidget->horizontalHeader()->setSectionsClickable(true);
//    ui->balancesTableWidget->horizontalHeader()->setFixedHeight(35);
    ui->balancesTableWidget->horizontalHeader()->setVisible(true);
    ui->balancesTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->balancesTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->balancesTableWidget->verticalHeader()->setVisible(false);
    ui->balancesTableWidget->setColumnWidth(0,250);
    ui->balancesTableWidget->setColumnWidth(1,250);
    ui->balancesTableWidget->setColumnWidth(2,130);

    ui->balancesTableWidget->setStyleSheet(TABLEWIDGET_STYLE_1);

    ui->depositBtn->setStyleSheet(TOOLBUTTON_STYLE_1);
    ui->openForUsersBtn->setStyleSheet(TOOLBUTTON_STYLE_1);

    ui->openForUsersBtn->setToolTip(tr("Before you open your contract for all users, other users can not buy your sell-orders."));
    ui->openForUsersBtn->hide();

    pageWidget = new PageScrollWidget();
    ui->stackedWidget->addWidget(pageWidget);
    connect(pageWidget,&PageScrollWidget::currentPageChangeSignal,this,&ContractBalanceWidget::pageChangeSlot);

    blankWidget = new BlankDefaultWidget(ui->balancesTableWidget);
    blankWidget->setTextTip(tr("There is no asset in the contract!"));


    XWCWallet::getInstance()->mainFrame->installBlurEffect(ui->balancesTableWidget);
    init();
}

ContractBalanceWidget::~ContractBalanceWidget()
{
    delete ui;
}

void ContractBalanceWidget::init()
{

}

void ContractBalanceWidget::setAccount(QString _accountName)
{
    accountName = _accountName;

    refresh();
}

void ContractBalanceWidget::refresh()
{
    QString contractAddress = XWCWallet::getInstance()->getExchangeContractAddress(accountName);

    ui->contractAddressLabel->setText(contractAddress);

    showContractBalances();

    if(XWCWallet::getInstance()->getExchangeContractState(accountName) == "NOT_INITED")
    {
        ui->openForUsersBtn->show();
    }
    else
    {
        ui->openForUsersBtn->hide();
    }
}

void ContractBalanceWidget::jsonDataUpdated(QString id)
{

    if( id == "id-invoke_contract_testing-openForUsers-" + accountName )
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
            withdrawOrderDialog.setAccountName(accountName);
            withdrawOrderDialog.setText(tr("You need to pay the fee for contract execution."));
            if(withdrawOrderDialog.pop())
            {
                QString contractAddress = XWCWallet::getInstance()->getExchangeContractAddress(accountName);

                XWCWallet::getInstance()->postRPC( "id-invoke_contract-openForUsers", toJsonFormat( "invoke_contract",
                                                                                    QJsonArray() << accountName
                                                                                    << XWCWallet::getInstance()->currentContractFee() << stepCount
                                                                                    << contractAddress
                                                                                    << "openForUsers"  << ""));

            }

        }
        else if(result.contains("no sell order"))
        {
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText( tr("Please put on at least 1 sell-order!") );
            commonDialog.pop();
        }

        return;
    }

    if( id == "id-invoke_contract-openForUsers")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            TransactionResultDialog transactionResultDialog;
            transactionResultDialog.setInfoText(tr("Open the contract for users successfully! Please wait for the confirmation of the block chain. Please do not repeat the operation."));
            transactionResultDialog.setDetailText(result);
            transactionResultDialog.pop();
        }
        else if(result.startsWith("\"error\":"))
        {
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Fail to call the function of the contract!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }

        return;
    }
}

void ContractBalanceWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(239,242,245),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(239,242,245),Qt::SolidPattern));

    painter.drawRect(rect());
}

void ContractBalanceWidget::showContractBalances()
{
    ExchangeContractBalances balances = XWCWallet::getInstance()->accountExchangeContractBalancesMap.value(accountName);

    QStringList keys = balances.keys();
    int size = keys.size();
    ui->balancesTableWidget->setRowCount(0);
    ui->balancesTableWidget->setRowCount(size);

    for(int i = 0; i < size; i++)
    {
        ui->balancesTableWidget->setRowHeight(i,42);

        QString key = keys.at(i);
        AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(key));

        ui->balancesTableWidget->setItem(i,0, new QTableWidgetItem( revertERCSymbol( key)));
        ui->balancesTableWidget->setItem(i,1, new QTableWidgetItem(getBigNumberString(balances.value(key), assetInfo.precision)));
        ui->balancesTableWidget->setItem(i,2, new QTableWidgetItem(tr("withdraw")));

        ToolButtonWidgetItem *toolButtonItem = new ToolButtonWidgetItem(i,2);
        toolButtonItem->setText(ui->balancesTableWidget->item(i,2)->text());
        ui->balancesTableWidget->setCellWidget(i,2,toolButtonItem);
        connect(toolButtonItem,SIGNAL(itemClicked(int,int)),this,SLOT(onItemClicked(int,int)));

    }

    tableWidgetSetItemZebraColor(ui->balancesTableWidget);

    int page = (ui->balancesTableWidget->rowCount()%ROWNUMBER==0 && ui->balancesTableWidget->rowCount() != 0) ?
                ui->balancesTableWidget->rowCount()/ROWNUMBER : ui->balancesTableWidget->rowCount()/ROWNUMBER+1;
    pageWidget->SetTotalPage(page);
    pageWidget->setShowTip(ui->balancesTableWidget->rowCount(),ROWNUMBER);
    pageChangeSlot(pageWidget->GetCurrentPage());
    pageWidget->setVisible(0 != size);

    blankWidget->setVisible(0 == size);
}

void ContractBalanceWidget::on_depositBtn_clicked()
{
    QString contractAddress = XWCWallet::getInstance()->getExchangeContractAddress(accountName);

    if(contractAddress.isEmpty())
    {

    }
    else
    {
        DepositExchangeContractDialog depositExchangeContractDialog;
        depositExchangeContractDialog.pop();
    }

}

void ContractBalanceWidget::onItemClicked(int _row, int _column)
{
    if(_column == 2)
    {

        if(!XWCWallet::getInstance()->ValidateOnChainOperation()) return;
        // 合约提现
        WithdrawExchangeContractDialog withdrawExchangeContractDialog;
        withdrawExchangeContractDialog.setCurrentAsset(ui->balancesTableWidget->item(_row,0)->text());
        withdrawExchangeContractDialog.pop();
        return;
    }
}

void ContractBalanceWidget::on_openForUsersBtn_clicked()
{
    QString contractAddress = XWCWallet::getInstance()->getExchangeContractAddress(accountName);

    XWCWallet::getInstance()->postRPC( "id-invoke_contract_testing-openForUsers-" + accountName, toJsonFormat( "invoke_contract_testing",
                                                                           QJsonArray() << accountName << contractAddress
                                                                                                             << "openForUsers"  << ""));
}

void ContractBalanceWidget::pageChangeSlot(unsigned int page)
{
    for(int i = 0;i < ui->balancesTableWidget->rowCount();++i)
    {
        if(i < page*ROWNUMBER)
        {
            ui->balancesTableWidget->setRowHidden(i,true);
        }
        else if(page * ROWNUMBER <= i && i < page*ROWNUMBER + ROWNUMBER)
        {
            ui->balancesTableWidget->setRowHidden(i,false);
        }
        else
        {
            ui->balancesTableWidget->setRowHidden(i,true);
        }
    }

}
