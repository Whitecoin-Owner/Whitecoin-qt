#ifndef POUNDAGEDATAUTIL_H
#define POUNDAGEDATAUTIL_H

#include <memory>
#include <vector>
#include <QString>
#include <QDateTime>

class PoundageUnit
{
public:
    PoundageUnit()
        :sourceCoinID("1.3.0")
        ,chainType("BTC")
        ,sourceCoinNumber(0)
        ,targetCoinNumber(0)
        ,poundageFinished(false)
    {

    }
public:
    QString poundageID;//手续费ID
    QString ownerAdress;//发行人地址
    QDateTime publishTime;//发行时间
    QString chainType;//发行税单类型
    QString sourceCoinID;//源币类型
    QString targetCoinID;//目标币类型
    double sourceCoinNumber;//源币数量XWC
    double targetCoinNumber;//目标币数量
    double balanceNumber;//账户余额,目标币剩余数量
    bool  poundageFinished;//承兑单是否已经用完余额
public:
    double calSourceLeftNumber()
    {
        if(targetCoinNumber < 1e-20) return 0;
        return balanceNumber/targetCoinNumber*sourceCoinNumber;
    }
};

class PoundageSheet
{
public:
    PoundageSheet()
    {

    }
    void clear()
    {
        poundages.clear();
    }
    void push_back(const std::shared_ptr<PoundageUnit> &data)
    {
        poundages.push_back(data);
    }
    int size()
    {
        return static_cast<int>(poundages.size());
    }
    //true表示，按照时间由早到晚
    void sortByTime(bool greater = true)
    {
        if(greater)
        {
            std::stable_sort(poundages.begin(),poundages.end(),[](const std::shared_ptr<PoundageUnit> &left,const std::shared_ptr<PoundageUnit> &right){
                    return left->publishTime < right->publishTime;
            });
        }
        else
        {
            std::stable_sort(poundages.begin(),poundages.end(),[](const std::shared_ptr<PoundageUnit> &left,const std::shared_ptr<PoundageUnit> &right){
                    return left->publishTime > right->publishTime;
            });
        }
    }
    //按链类型删选
    void filterByChainType(const QString &chainType)
    {
        if("all" == chainType || "All" == chainType || "ALL" == chainType) return;
        poundages.erase(std::remove_if(poundages.begin(),poundages.end(),[chainType](std::shared_ptr<PoundageUnit> info){
                                       return info->chainType != chainType;})
                        ,poundages.end());
    }
    //按汇率排序,true表示按汇率由低到高
    void sortByRate(bool greater = true)
    {
        std::stable_sort(poundages.begin(),poundages.end(),[greater](const std::shared_ptr<PoundageUnit> &left,const std::shared_ptr<PoundageUnit> &right){
            return greater ? left->sourceCoinNumber/left->targetCoinNumber < right->sourceCoinNumber/right->targetCoinNumber
                           : left->sourceCoinNumber/left->targetCoinNumber > right->sourceCoinNumber/right->targetCoinNumber;
        });
    }
    //判断是否已有该id
    bool isIdExisted(const QString &id)
    {
        return poundages.end() != std::find_if(poundages.begin(),poundages.end(),[id](const std::shared_ptr<PoundageUnit> &info){
            return info->poundageID == id;
        });
    }

public:
    std::vector<std::shared_ptr<PoundageUnit>> poundages;
};

class QJsonObject;
class PoundageDataUtil
{
public:
    PoundageDataUtil();
public:
    static bool convertJsonToPoundage(const QString &jsonString,std::shared_ptr<PoundageSheet> &sheet);

    static bool ParseJsonObjToUnit(QJsonObject jsonObj, std::shared_ptr<PoundageUnit> &poundageUnit);
};

#endif // POUNDAGEDATAUTIL_H
