#include "OldRpcAdapter.h"
#include "wallet.h"

#define OLD_RPC_ADAPTER_SERVER_PORT (NODE_RPC_PORT + 2)

OldRpcAdapter::OldRpcAdapter(QObject* parent) : QObject (parent)
{
    connect( XWCWallet::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

}

OldRpcAdapter::~OldRpcAdapter()
{
    if(server)
    {
        delete server;
    }
}

void OldRpcAdapter::startServer()
{
    server = new QTcpServer(qApp);
    connect(server, &QTcpServer::newConnection, this, &OldRpcAdapter::onNewConnection);
    listen();
}

void OldRpcAdapter::listen()
{
    if(server)
    {
        server->listen(QHostAddress::LocalHost, OLD_RPC_ADAPTER_SERVER_PORT);
        if(server->isListening())
        {
            qDebug() << "OldRpcAdapter server listening on port " << OLD_RPC_ADAPTER_SERVER_PORT;
        }
        else
        {
            qDebug() << "OldRpcAdapter server cannot bind on port ";
        }
    }
}

void OldRpcAdapter::close()
{
    if(server)
    {
        server->close();
        delete server;
        server = nullptr;
    }
}

bool OldRpcAdapter::isRunning()
{
    if(server)
    {
        return server->isListening();
    }

    return false;
}

void OldRpcAdapter::jsonDataUpdated(QString id)
{
    if( id.startsWith("OldRpcAdapter-wallet_create_account-") )
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        QJsonObject resultObject;

        if(result.startsWith(QString("\"result\":\"%1").arg(ACCOUNT_ADDRESS_PREFIX)))
        {
            result.prepend("{");
            result.append("}");
            QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
            QJsonObject jsonObject = parse_doucment.object();
            QString address = jsonObject.value("result").toString();
            resultObject.insert("result",address);

            XWCWallet::getInstance()->autoSaveWalletFile();
            XWCWallet::getInstance()->configFile->setValue("/settings/backupNeeded",true);
            XWCWallet::getInstance()->IsBackupNeeded = true;
            XWCWallet::getInstance()->witnessConfig->addTrackAddress(address);
            XWCWallet::getInstance()->witnessConfig->save();
        }
        else
        {
            resultObject.insert("error","invalid account name");
        }

        if(m_socket)
        {
            QByteArray writeData = QJsonDocument(resultObject).toJson();
            m_socket->write(writeData, writeData.size());
            m_socket->flush();

            if (!m_socket->waitForBytesWritten())
            {
                m_socket->disconnectFromHost();
                m_socket->deleteLater();
            }
        }

        return;
    }
}

void OldRpcAdapter::onNewConnection()
{
    qDebug() << "onNewConnection";
    QTcpSocket* socketTemp = server->nextPendingConnection();
    m_socket = socketTemp;
    connect(socketTemp, &QTcpSocket::readyRead, this, &OldRpcAdapter::readSocket);
    connect(socketTemp, &QTcpSocket::disconnected, this, &OldRpcAdapter::onDisconnect);
}

void OldRpcAdapter::onDisconnect()
{

}

void OldRpcAdapter::readSocket()
{
    QString requestStr = m_socket->readAll();
    qDebug() << "bbbbbbbbb " << requestStr;
    QByteArray writeData;
    QJsonObject resultObject;
    if(requestStr.startsWith("GET /api/rpc/"))
    {
        QStringList strList = requestStr.split(" ");
        if(strList.size() < 2)
        {
            resultObject.insert("error","cannot parse url");
        }
        else
        {
            QString path = strList.at(1);
            QString apiName = path.mid(QString("/api/rpc/").size());
            qDebug() << "apiName" << apiName;
            if(apiName.contains("?"))
            {
                QStringList strList2 = apiName.split("?");
                apiName = strList2.at(0);
                QString paramStr = strList2.at(1);
                if(apiName == "getAsset")
                {

                }
                else if(apiName == "getBalance")
                {
                    if(paramStr.startsWith("label={") && paramStr.endsWith("}"))
                    {
                        QString label = paramStr.mid(QString("label={").size());
                        label.chop(1);
                        double balance = getBalance(label);
                        resultObject.insert("result",balance);
                    }
                    else
                    {
                        resultObject.insert("error","invalid param(label required)");
                    }
                }
                else if(apiName == "getListTransactions")
                {
                    QStringList paramList = paramStr.split("&");
                    if(paramList.size() == 3 && paramList.at(0).startsWith("label={") && paramList.at(0).endsWith("}")
                            && paramList.at(1).startsWith("count={") && paramList.at(1).endsWith("}")
                            && paramList.at(2).startsWith("from={") && paramList.at(2).endsWith("}"))
                    {
                        QString label = paramList.at(0).mid(QString("label={").size());
                        label.chop(1);
                        QString countStr = paramList.at(1).mid(QString("count={").size());
                        countStr.chop(1);
                        QString fromStr = paramList.at(2).mid(QString("from={").size());
                        fromStr.chop(1);
                        QJsonArray array = getListTransactions(label,countStr.toInt(),fromStr.toInt());
                        resultObject.insert("result",array);
                    }
                    else
                    {
                        resultObject.insert("error","invalid param(label,count,from required)");
                    }
                }
                else if(apiName == "getTransaction")
                {
                    if(paramStr.startsWith("txid={") && paramStr.endsWith("}"))
                    {
                        QString trxId = paramStr.mid(QString("txid={").size());
                        trxId.chop(1);
                        QJsonObject object = getTransaction(trxId);
                        resultObject.insert("result",object);
                    }
                    else
                    {
                        resultObject.insert("error","invalid param(txid required)");
                    }
                }
                else if(apiName == "newAddress")
                {
                    if(paramStr.startsWith("label={") && paramStr.endsWith("}"))
                    {
                        QString label = paramStr.mid(QString("label={").size());
                        label.chop(1);
                        newAddress(label);
                        return;
                    }
                    else
                    {
                        resultObject.insert("error","invalid param(label required)");
                    }
                }
                else if(apiName == "validAddress")
                {
                    if(paramStr.startsWith("address={") && paramStr.endsWith("}"))
                    {
                        QString address = paramStr.mid(QString("address={").size());
                        address.chop(1);
                        QJsonObject object = validateAddress(address);
                        resultObject.insert("result",object);
                    }
                    else
                    {
                        resultObject.insert("error","invalid param(address required)");
                    }
                }
                else if(apiName == "send")
                {

                }
            }
            else
            {
                if(apiName == "getInfo")
                {
                    resultObject.insert("result", getInfo());
                }
            }

        }
    }
    else
    {
        resultObject.insert("error","The only allowed format is \"GET /api/rpc/...\"");
    }

    writeData = QJsonDocument(resultObject).toJson();
    m_socket->write(writeData, writeData.size());
    m_socket->flush();

    if (!m_socket->waitForBytesWritten())
    {
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
    }
}

QJsonObject OldRpcAdapter::getInfo()
{
    OldRpc_InfoResult result;
    result.blocks = XWCWallet::getInstance()->walletInfo.blockHeight;
    result.connections = XWCWallet::getInstance()->walletInfo.connections;
    return result.toJson();
}

double OldRpcAdapter::getBalance(QString label)
{
    AssetAmount aa = XWCWallet::getInstance()->accountInfoMap.value(label).assetAmountMap.value(XWCWallet::getInstance()->getAssetId(ASSET_NAME));
    return getBigNumberString(aa.amount, ASSET_PRECISION).toDouble();
}

QJsonObject OldRpcAdapter::validateAddress(QString address)
{
    OldRpc_ValidateAddressResult result;
    result.address = address;
    if(checkAddress(address) == AccountAddress)
    {
        result.isvalid = true;
        QString accountName = XWCWallet::getInstance()->addressToName(address);
        if(accountName.isEmpty())
        {
            result.ismine = false;
        }
        else
        {
            result.ismine = true;
            result.account = accountName;
        }
    }
    return result.toJson();
}

QJsonObject OldRpcAdapter::getTransaction(QString trxId)
{
    OldRpc_GetTransactionResult result;
    TransactionStruct ts = XWCWallet::getInstance()->transactionDB.getTransactionStruct(trxId);
    if(ts.type == TRANSACTION_TYPE_NORMAL)
    {
        QJsonObject operationObject = QJsonDocument::fromJson(ts.operationStr.toLatin1()).object();
        QJsonObject amountObject = operationObject.value("amount").toObject();
        unsigned long long amount = jsonValueToULL(amountObject.take("amount"));
        QString amountAssetId = amountObject.value("asset_id").toString();
        if(amountAssetId == "1.3.0")    // 只考虑转账XWC的普通交易
        {
            AssetInfo amountAssetInfo = XWCWallet::getInstance()->assetInfoMap.value(amountAssetId);
            QString fromAddress = operationObject.value("from_addr").toString();
            QString toAddress   = operationObject.value("to_addr").toString();

            result.amount = getBigNumberString(amount, ASSET_PRECISION).toDouble();
            result.fee = getBigNumberString(ts.feeAmount, ASSET_PRECISION).toDouble();
            result.confirmations = XWCWallet::getInstance()->walletInfo.blockHeight - ts.blockNum;
            result.blockindex = ts.blockNum;
            result.txid = trxId;

            QDateTime trxTime = QDateTime::fromString(ts.expirationTime, "yyyy-MM-ddThh:mm:ss");
            result.time = trxTime.toTime_t();

            result.to = toAddress;
            result.from = fromAddress;

            QString message = operationObject.value("memo").toObject().value("message").toString();
            result.message = message;
        }
    }

    return result.toJson();
}

QJsonArray OldRpcAdapter::getListTransactions(QString label, int count, int from)
{
    QJsonArray array;
    if(XWCWallet::getInstance()->accountInfoMap.contains(label))
    {
        QString address = XWCWallet::getInstance()->accountInfoMap.value(label).address;
        TransactionTypeIds typeIds = XWCWallet::getInstance()->transactionDB.getAccountTransactionTypeIdsByType(address, TRANSACTION_TYPE_NORMAL);

        TransactionTypeIds filteredTypeIds;  // 排除非XWC的交易 排除未确认交易
        foreach (TransactionTypeId id, typeIds)
        {
            TransactionStruct ts = XWCWallet::getInstance()->transactionDB.getTransactionStruct(id.transactionId);
            if(ts.blockNum > 0)
            {
                QJsonObject operationObject = QJsonDocument::fromJson(ts.operationStr.toLatin1()).object();
                QJsonObject amountObject = operationObject.value("amount").toObject();
                QString amountAssetId = amountObject.value("asset_id").toString();

                if(amountAssetId == "1.3.0")  filteredTypeIds.append(id);
            }
        }

        // 根据区块高度排序 从低到高
        TransactionTypeIds sortedTypeIds;
        for(int i = 0; i < filteredTypeIds.size(); i++)
        {
            if(sortedTypeIds.size() == 0)
            {
                sortedTypeIds.append(filteredTypeIds.at(i));
                continue;
            }

            TransactionStruct ts = XWCWallet::getInstance()->transactionDB.getTransactionStruct(filteredTypeIds.at(i).transactionId);
            for(int j = 0; j < sortedTypeIds.size(); j++)
            {
                TransactionStruct ts2 = XWCWallet::getInstance()->transactionDB.getTransactionStruct(sortedTypeIds.at(j).transactionId);
                if(ts.blockNum < ts2.blockNum)
                {
                    sortedTypeIds.insert(j,filteredTypeIds.at(i));
                    break;
                }

                if(j == sortedTypeIds.size() - 1)
                {
                    sortedTypeIds.append(filteredTypeIds.at(i));
                    break;
                }
            }
        }

        if(sortedTypeIds.size() > from)
        {
            for(int i = from; (i < sortedTypeIds.size()) && (i < from + count); i++)
            {
                TransactionStruct ts = XWCWallet::getInstance()->transactionDB.getTransactionStruct(sortedTypeIds.at(i).transactionId);
                OldRpc_GetTransactionResult oldTrx;
                QJsonObject operationObject = QJsonDocument::fromJson(ts.operationStr.toLatin1()).object();
                QJsonObject amountObject = operationObject.value("amount").toObject();
                unsigned long long amount = jsonValueToULL(amountObject.take("amount"));
                QString amountAssetId = amountObject.value("asset_id").toString();

                AssetInfo amountAssetInfo = XWCWallet::getInstance()->assetInfoMap.value(amountAssetId);
                QString fromAddress = operationObject.value("from_addr").toString();
                QString toAddress   = operationObject.value("to_addr").toString();

                oldTrx.amount = getBigNumberString(amount, ASSET_PRECISION).toDouble();
                oldTrx.fee = getBigNumberString(ts.feeAmount, ASSET_PRECISION).toDouble();
                oldTrx.confirmations = XWCWallet::getInstance()->walletInfo.blockHeight - ts.blockNum;
                oldTrx.blockindex = ts.blockNum;
                oldTrx.txid = sortedTypeIds.at(i).transactionId;

                QDateTime trxTime = QDateTime::fromString(ts.expirationTime, "yyyy-MM-ddThh:mm:ss");
                oldTrx.time = trxTime.toTime_t();

                oldTrx.to = toAddress;
                oldTrx.from = fromAddress;

                QString message = operationObject.value("memo").toObject().value("message").toString();
                oldTrx.message = message;

                array << oldTrx.toJson();
            }
        }
    }

    return array;
}

void OldRpcAdapter::newAddress(QString label)
{
    XWCWallet::getInstance()->postRPC( "OldRpcAdapter-wallet_create_account-" + label, toJsonFormat( "wallet_create_account", QJsonArray() << label ));
}
