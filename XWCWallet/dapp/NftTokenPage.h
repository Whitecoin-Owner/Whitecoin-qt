#ifndef NFTTOKENPAGE_H
#define NFTTOKENPAGE_H

#include <QWidget>
#include <QMap>
#include <QSet>

namespace Ui {
class NftTokenPage;
}

struct NftTokenInfo {
    QString contractId;
    int tokenNumber;
    QMap<QString, QString> tokenIdUri;

    NftTokenInfo() {
        tokenNumber = 0;
    }
};

struct NftContractInfo {
    QString name;
    QString symbol;
    QString baseUri;
    QMap<QString, NftTokenInfo> nftTokens;
};

class NftTokenPage : public QWidget
{
    Q_OBJECT

public:
    explicit NftTokenPage(QWidget *parent = nullptr);
    ~NftTokenPage();

    QMap<QString, NftContractInfo> nftContracts;

private slots:
    void on_accountComboBox_currentIndexChanged(const QString &arg1);
    void jsonDataUpdated(QString id);
    void fetchTokensInfo();
    void refresh();
    void on_addNftBtn_clicked();
    void on_tokenTableWidget_cellClicked(int row, int column);

private:
    Ui::NftTokenPage *ui;
    QAtomicInt nftUpdatedNum;
    QSet<QString> txs;
    QTimer *refreshTimer;
    void paintEvent(QPaintEvent*);
    void showAccountTokens();
};

#endif // NFTTOKENPAGE_H
