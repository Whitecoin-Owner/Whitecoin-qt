#include <QInputDialog>
#include <QMessageBox>
#include "NftTokenPage.h"
#include "AddTokenDialog.h"
#include "ToolButtonWidget.h"
#include "TokenTransferWidget.h"
#include "ui_NftTokenPage.h"
#include "wallet.h"
#include "extra/LogToFile.h"

NftTokenPage::NftTokenPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NftTokenPage)
{
    ui->setupUi(this);

    connect( XWCWallet::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    ui->tokenTableWidget->installEventFilter(this);
    ui->tokenTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tokenTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tokenTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->tokenTableWidget->setMouseTracking(true);
    ui->tokenTableWidget->setShowGrid(false);

    ui->tokenTableWidget->horizontalHeader()->setSectionsClickable(true);
    ui->tokenTableWidget->horizontalHeader()->setVisible(true);
    ui->tokenTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    ui->tokenTableWidget->setColumnWidth(0,100);
    ui->tokenTableWidget->setColumnWidth(1,160);
    ui->tokenTableWidget->setColumnWidth(2,160);
    ui->tokenTableWidget->setColumnWidth(3,150);
    ui->tokenTableWidget->setColumnWidth(4,80);
    ui->tokenTableWidget->setStyleSheet(TABLEWIDGET_STYLE_1);

    ui->accountComboBox->clear();
    QStringList accounts = XWCWallet::getInstance()->accountInfoMap.keys();
    ui->accountComboBox->addItems(accounts);

    if(accounts.contains(XWCWallet::getInstance()->currentAccount))
    {
        ui->accountComboBox->setCurrentText(XWCWallet::getInstance()->currentAccount);
    }
    ui->addNftBtn->setStyleSheet(TOOLBUTTON_STYLE_1);
    nftUpdatedNum = 0;
    fetchTokensInfo();
//    refreshTimer = new QTimer(this);
//    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));
//    refreshTimer->start(3);
}

NftTokenPage::~NftTokenPage()
{
    delete ui;
}

void NftTokenPage::refresh()
{
//        this->showAccountTokens();
    QStringList accounts = XWCWallet::getInstance()->accountInfoMap.keys();

    foreach (QString contractId, this->nftContracts.keys())
    {
        QString accountAddress = XWCWallet::getInstance()->accountInfoMap.value(ui->accountComboBox->currentText()).address;
        XWCWallet::getInstance()->postRPC( "NftTokenPage-invoke_contract_offline-balanceOf-" + contractId + "-" + accountAddress,
                                       toJsonFormat( "invoke_contract_offline",
                                       QJsonArray() << ui->accountComboBox->currentText() << contractId
                                          << "balanceOf"  << accountAddress));
    }
}

void NftTokenPage::on_accountComboBox_currentIndexChanged(const QString &arg1)
{
    XWCWallet::getInstance()->currentAccount = ui->accountComboBox->currentText();
    showAccountTokens();
}

void NftTokenPage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(239,242,245),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(239,242,245),Qt::SolidPattern));

    painter.drawRect(rect());
}

void NftTokenPage::fetchTokensInfo()
{
    logToFile(QStringList() << "fetchTokensInfo()");
    XWCWallet::getInstance()->configFile->beginGroup("/nftTokens");
    QStringList contractIds = XWCWallet::getInstance()->configFile->childKeys();
    XWCWallet::getInstance()->configFile->endGroup();
    QString accountAddress = XWCWallet::getInstance()->accountInfoMap.value(ui->accountComboBox->currentText()).address;
    foreach (QString contractId, contractIds)
    {
        QStringList contractInfo = contractId.split(",");
        nftContracts[contractInfo[0]] = NftContractInfo();
        if (contractInfo.length() == 1) {   //newly added nft contract
            XWCWallet::getInstance()->postRPC( "NftTokenPage-invoke_contract_offline-tokenName-" + contractId,
                                               toJsonFormat( "invoke_contract_offline",
                                                    QJsonArray() << ui->accountComboBox->currentText() << contractId
                                                        << "tokenName" << ""));
            XWCWallet::getInstance()->postRPC( "NftTokenPage-invoke_contract_offline-symbol-" + contractId,
                                               toJsonFormat( "invoke_contract_offline",
                                                    QJsonArray() << ui->accountComboBox->currentText() << contractId
                                                        << "symbol" << ""));
        } else if (contractInfo.length() == 3) {    // nft contract basic info
            nftContracts[contractInfo[0]].name = contractInfo[1];
            nftContracts[contractInfo[0]].symbol = contractInfo[2];
            nftContracts[contractInfo[0]].baseUri = contractInfo[3];
        }
        foreach (QString acct, XWCWallet::getInstance()->accountInfoMap.keys()) {
            QString address = XWCWallet::getInstance()->accountInfoMap.value(acct).address;
            XWCWallet::getInstance()->postRPC(
                "NftTokenPage-invoke_contract_offline-balanceOf-" + contractId + "-" + address,
                toJsonFormat( "invoke_contract_offline",
                              QJsonArray() << ui->accountComboBox->currentText() << contractId
                                           << "balanceOf" << address));
        }
    }
    XWCWallet::getInstance()->postRPC(
                "NftTokenPage-invoke_contract_offline-fetchNftTokens-finish",
                toJsonFormat( "fetchNftTokens-finish", QJsonArray() ));
}


void NftTokenPage::jsonDataUpdated(QString id)
{
    QString contractUpdated;
//    logToFile(QStringList() << id);
    if( id.startsWith("NftTokenPage-invoke_contract_offline-balanceOf-"))
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        QStringList params = id.split("-");
        QString contractId = params[3];
        QString address = params[4];

        if(result.startsWith(QString("\"result\":")))
        {
            result.prepend("{");
            result.append("}");

            QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
            QJsonObject object = parse_doucment.object();
            int balance = object.value("result").toString().toInt();
            int currentBalance = 0;
            if (nftContracts[contractId].nftTokens.contains(address)) {
                currentBalance = nftContracts[contractId].nftTokens[address].tokenNumber;
            } else {
                nftContracts[contractId].nftTokens[address] = NftTokenInfo();
            }
            logToFile(QStringList() << "NftTokenPage-balanceOf-" << address << "-" << QString::number(balance) << ":" << QString::number(currentBalance));
            if (currentBalance == balance) {
                return;
            }
            nftUpdatedNum += balance;
            nftContracts[contractId].nftTokens[address].tokenNumber = balance;
            nftContracts[contractId].nftTokens[address].tokenIdUri.clear();
            for (int i=1; i<=balance; i++) {
                XWCWallet::getInstance()->postRPC(
                            "NftTokenPage-invoke_contract_offline-tokenOfOwnerByIndex-"
                                + contractId + "-" + address,
                            toJsonFormat(
                                "invoke_contract_offline",
                                QJsonArray() << ui->accountComboBox->currentText() << contractId
                                    << "tokenOfOwnerByIndex" << address+","+QString::number(i)));
            }
        }
    } else if (id.startsWith("NftTokenPage-invoke_contract_offline-tokenName-")) {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        QString contractId = id.mid( QString("NftTokenPage-invoke_contract_offline-tokenName-").size());
        if(result.startsWith(QString("\"result\":")))
        {
            result.prepend("{");
            result.append("}");

            QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
            QJsonObject object = parse_doucment.object();
            QString name = object.value("result").toString();
            nftContracts[contractId].name = name;
            contractUpdated = contractId;
        }
    } else if (id.startsWith("NftTokenPage-invoke_contract_offline-symbol-")) {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        QString contractId = id.mid( QString("NftTokenPage-invoke_contract_offline-symbol-").size());
        if(result.startsWith(QString("\"result\":")))
        {
            result.prepend("{");
            result.append("}");

            QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
            QJsonObject object = parse_doucment.object();
            QString symbol = object.value("result").toString();
            nftContracts[contractId].symbol = symbol;
            contractUpdated = contractId;
        }
    } else if (id.startsWith("NftTokenPage-invoke_contract_offline-tokenOfOwnerByIndex-")) {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        QStringList params = id.split("-");
        QString contractId = params[3];
        QString address = params[4];

        if(result.startsWith(QString("\"result\":"))) {
            result.prepend("{");
            result.append("}");
            QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
            QJsonObject object = parse_doucment.object();
            NftTokenInfo& token = nftContracts[contractId].nftTokens[address];
            QString tokenId = object.value("result").toString();
//            logToFile(QStringList() << "NftTokenPage-invoke_contract_offline-tokenOfOwnerByIndex1-" << address << "-" << tokenId);
            if (!token.tokenIdUri.contains(tokenId)) {
                token.tokenIdUri.insert(tokenId, "");
                XWCWallet::getInstance()->postRPC(
                            "NftTokenPage-invoke_contract_offline-tokenURI-"
                                + contractId + "-" + address + "-" + tokenId,
                            toJsonFormat(
                                "invoke_contract_offline",
                                QJsonArray() << ui->accountComboBox->currentText() << contractId
                                    << "tokenURI" << tokenId));
            }
        }
    } else if (id.startsWith("NftTokenPage-invoke_contract_offline-tokenURI-")) {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        QStringList params = id.split("-");
        QString contractId = params[3];
        QString address = params[4];
        QString tokenId = params[5];

        if(result.startsWith(QString("\"result\":"))) {
            result.prepend("{");
            result.append("}");
            QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
            QJsonObject object = parse_doucment.object();
            logToFile(QStringList() << "NftTokenPage-invoke_contract_offline-tokenURI-" << object.value("result").toString() << "-" << tokenId);
            NftTokenInfo& token = nftContracts[contractId].nftTokens[address];
            nftUpdatedNum -= 1;
            token.tokenIdUri[tokenId] = object.value("result").toString();
            if (nftUpdatedNum == 0) {
                showAccountTokens();
            }
        }
    } else if (id.startsWith("NftTokenPage-invoke_contract-transfer-")) {
        logToFile(QStringList() << "Post: " << id);
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        QStringList params = id.split("-");
        QString toInfo = params[3];
        if(result.startsWith(QString("\"result\":")))
        {
            result.prepend("{");
            result.append("}");
            QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
            QJsonObject object = parse_doucment.object().value("result").toObject();
            QString txid = object.value("trxid").toString();
            if (txs.find(txid) == txs.end()) {
                QMessageBox::about(
                     nullptr, "Transfer NFT Success", "Tx Id: "+txid);
                fetchTokensInfo();
            } else {
                txs.insert(txid);
            }
        } else if(result.startsWith("\"error\":"))
        {
            QMessageBox::about(
                 nullptr, "Transfer NFT Fail", "Please check your balance and NFT ownership.");
        }
    } else if( id == "NftTokenPage-invoke_contract_offline-fetchTokens-finish") {
        showAccountTokens();
    } else if (id == "NftTokenPage-invoke_contract_offline-fetchNftTokens-finish") {
        NftContractInfo c = nftContracts[contractUpdated];
        XWCWallet::getInstance()->configFile->setValue("/nftTokens/"+contractUpdated, contractUpdated+","+c.name+","+c.symbol);
    }
}

void NftTokenPage::showAccountTokens()
{
    logToFile(QStringList() << "NftTokenPage::showAccountTokens");
    ui->tokenTableWidget->setRowCount(0);

    int i = 0;
    QString accountAddress = XWCWallet::getInstance()->accountInfoMap.value(ui->accountComboBox->currentText()).address;
    foreach (const QString &cid, nftContracts.keys()) {
        i += nftContracts[cid].nftTokens[accountAddress].tokenIdUri.size();
    }
    logToFile(QStringList() << QString::number(i));
    ui->tokenTableWidget->setRowCount(i);

    i = 0;
    foreach (const QString &cid, nftContracts.keys())
    {
        NftTokenInfo &tokens = nftContracts[cid].nftTokens[accountAddress];
        QMap<QString, QString>::const_iterator it = tokens.tokenIdUri.constBegin();
        while (it != tokens.tokenIdUri.constEnd()) {
            logToFile(QStringList() << cid << "," << nftContracts[cid].symbol << "," << it.key() << "," << it.value());
            ui->tokenTableWidget->setItem( i, 0, new QTableWidgetItem(nftContracts[cid].symbol));
            ui->tokenTableWidget->setItem( i, 1, new QTableWidgetItem(cid));
            ui->tokenTableWidget->setItem( i, 2, new QTableWidgetItem(it.key()));
            ui->tokenTableWidget->setItem( i, 3, new QTableWidgetItem(it.value()));
            ui->tokenTableWidget->setItem( i, 4, new QTableWidgetItem(tr("transfer")));

            ToolButtonWidget *toolButton = new ToolButtonWidget();
            toolButton->setText(ui->tokenTableWidget->item(i,4)->text());
            ui->tokenTableWidget->setCellWidget(i,4,toolButton);
            connect(toolButton,&ToolButtonWidget::clicked,std::bind(&NftTokenPage::on_tokenTableWidget_cellClicked,this,i,4));
            ++it;
            ++i;
        }
    }
    tableWidgetSetItemZebraColor(ui->tokenTableWidget);
}

void NftTokenPage::on_tokenTableWidget_cellClicked(int row, int column)
{
    logToFile(QStringList() << QString::number(column));
    if(column == 4)
    {
        QString accountAddress = XWCWallet::getInstance()->accountInfoMap.value(ui->accountComboBox->currentText()).address;
        bool isOK;
        QString to = QInputDialog::getText(
                    nullptr, "Transfer NFT", "Please input address",
                    QLineEdit::Normal, "", &isOK);

        if(isOK) {
            QString params = QString("%1,%2,%3").arg(accountAddress).arg(to).arg(ui->tokenTableWidget->item(row, 2)->text());
            XWCWallet::getInstance()->postRPC(
                        "NftTokenPage-invoke_contract-transfer-" + params, toJsonFormat( "invoke_contract",
                            QJsonArray() << ui->accountComboBox->currentText()
                                << XWCWallet::getInstance()->currentContractFee() << 200000
                                << ui->tokenTableWidget->item(row, 1)->text()
                                << "transferFrom"  << params));
//               QMessageBox::information(
//                    nullptr, "Information", "to:" + to + "<br/>params:" + params + "<br/>contract:" + ui->tokenTableWidget->item(row, 1)->text(),
//                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        }
    }
}

void NftTokenPage::on_addNftBtn_clicked() {
    AddTokenDialog addTokenDialog;
    addTokenDialog.pop();

    if(addTokenDialog.newTokenAdded) {
        QTimer::singleShot(100, this, &NftTokenPage::fetchTokensInfo);
    }
}
