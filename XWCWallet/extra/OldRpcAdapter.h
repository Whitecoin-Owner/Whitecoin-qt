#ifndef OLDRPCADAPTER_H
#define OLDRPCADAPTER_H

#include <QTcpServer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

struct OldRpc_InfoResult
{
    double balance = 0;     // 钱包内所有账户的余额
    int blocks = 0;
    int connections = 0;
    QString version;
    QString label;
    double maxPay = 0;
    QString hot;
    QString cold;

    QJsonObject toJson()
    {
        QJsonObject object;
        object.insert("balance", balance);
        object.insert("blocks", blocks);
        object.insert("connections", connections);
        object.insert("version", version);
        object.insert("label", label);
        object.insert("maxPay", maxPay);
        object.insert("hot", hot);
        object.insert("cold", cold);
        return object;
    }
};

struct OldRpc_ValidateAddressResult
{
    bool isvalid = false;
    QString address;
    bool ismine = false;
    bool isscript = false;
    bool iscompressed = false;
    QString account;

    QJsonObject toJson()
    {
        QJsonObject object;
        object.insert("isvalid", isvalid);
        object.insert("address", address);
        object.insert("ismine", ismine);
        object.insert("isscript", isscript);
        object.insert("iscompressed", iscompressed);
        object.insert("account", account);
        return object;
    }
};

struct OldRpc_GetTransactionResult
{
    double amount = 0;
    double fee = 0;
    int confirmations = 0;
    QString blockhash;
    int blockindex = 0;
    int blocktime = 0;
    QString txid;
    int time = 0;
    int timereceived = 0;
    QString comment;
    QString to;
    QString from;
    QString message;

    QJsonObject toJson()
    {
        QJsonObject object;
        object.insert("amount", amount);
        object.insert("fee", fee);
        object.insert("confirmations", confirmations);
        object.insert("blockhash", blockhash);
        object.insert("blockindex", blockindex);
        object.insert("blocktime", blocktime);
        object.insert("txid", txid);
        object.insert("time", time);
        object.insert("timereceived", timereceived);
        object.insert("comment", comment);
        object.insert("to", to);
        object.insert("from", from);
        object.insert("message", message);
        return object;
    }
};


class OldRpcAdapter : public QObject
{
    Q_OBJECT
public:
    OldRpcAdapter(QObject* parent = nullptr);
    ~OldRpcAdapter();

    void startServer();
    void listen();
    void close();

    bool isRunning();

private slots:
    void jsonDataUpdated(QString id);

    void onNewConnection();
    void onDisconnect();
    void readSocket();

private:
    QJsonObject getInfo();
    double getBalance(QString label);
    QJsonObject validateAddress(QString address);
    QJsonObject getTransaction(QString trxId);
    QJsonArray getListTransactions(QString label, int count, int from);
    void newAddress(QString label);

private:
    QTcpServer* server = nullptr;
    QTcpSocket* m_socket = nullptr;
};

#endif // OLDRPCADAPTER_H
