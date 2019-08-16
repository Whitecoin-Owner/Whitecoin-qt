#include "AutoUpdateNotify.h"

#include <QTimer>
#include "update/UpdateProcess.h"
#include "wallet.h"
#include "autoUpdate/AutoUpdateDialog.h"

class AutoUpdateNotify::DataPrivate
{
public:
    DataPrivate()
        :isAutoDetectOn(false)
        ,isInDetect(false)
        ,updateProcess(new UpdateProcess())
        ,isForceUpdate(false)
        ,version("")
    {

    }
    ~DataPrivate()
    {
        if(updateProcess)
        {
            delete updateProcess;
            updateProcess = nullptr;
        }
    }
public:
    bool isAutoDetectOn;//是否开启了自动检查更新
    bool isInDetect;//当前是否正在检查
    UpdateProcess *updateProcess;
    QTimer *updateTimer;

    QString version;
    bool isForceUpdate;
};

AutoUpdateNotify::AutoUpdateNotify(QObject *parent)
    : QObject(parent)
    ,_p(new DataPrivate())
{
    InitData();
}

AutoUpdateNotify::~AutoUpdateNotify()
{
    delete _p;
    _p = nullptr;
}

void AutoUpdateNotify::startAutoDetect()
{
    _p->isAutoDetectOn = true;
    _p->isInDetect = false;
    _p->updateTimer->start(1000);
}

void AutoUpdateNotify::stopAutoDetect()
{
    _p->isAutoDetectOn = false;
}

void AutoUpdateNotify::checkUpdate()
{
    if(!_p->isAutoDetectOn || _p->isInDetect) return;
    _p->isInDetect = true;
    _p->version = "";
    _p->isForceUpdate = false;
    _p->updateTimer->setInterval(3600000);
    _p->updateProcess->InitServerURL(XWCWallet::getInstance()->middlewarePath);
    _p->updateProcess->checkUpdate();
}

void AutoUpdateNotify::InitData()
{
    _p->updateTimer = new QTimer(this);
    connect(_p->updateTimer,&QTimer::timeout,this,&AutoUpdateNotify::checkUpdate);

    connect(_p->updateProcess,&UpdateProcess::updateWrong,this,&AutoUpdateNotify::UpdateWrongSlot);
    connect(_p->updateProcess,&UpdateProcess::NewstVersionSignal,this,&AutoUpdateNotify::CheckResultSlot);
    connect(_p->updateProcess,&UpdateProcess::updateFinish,this,&AutoUpdateNotify::UpdateFinishSlot);
}

void AutoUpdateNotify::CheckResultSlot(const QString &version,bool isupdateForce)
{
    _p->isInDetect = false;
    if(version.isEmpty()) return;
    if(_p->updateProcess->getUpdateLogInfo().contains("not found"))
    {
        UpdateWrongSlot();
        return;
    }

    _p->isInDetect = true;
    _p->version = version;
    _p->isForceUpdate =isupdateForce;
    _p->updateProcess->startUpdate();
}

void AutoUpdateNotify::UpdateWrongSlot()
{
    _p->isInDetect = true;
    QTimer::singleShot(10000,this,&AutoUpdateNotify::recheckNow);
}

void AutoUpdateNotify::UpdateFinishSlot()
{
    static AutoUpdateDialog *dia = new AutoUpdateDialog();
    if(dia->isVisible())
    {
        return;
    }
    dia->setVersion(_p->version,_p->isForceUpdate);
    dia->setUpdateLog(this->_p->updateProcess->getUpdateLogInfo());
    if(dia->pop())
    {
        _p->isAutoDetectOn=false;
        XWCWallet::getInstance()->SetUpdateNeeded(true);
        XWCWallet::getInstance()->mainFrame->onCloseWallet();
    }
    else
    {
        _p->isInDetect = false;
        _p->isAutoDetectOn=true;
        XWCWallet::getInstance()->SetUpdateNeeded(false);
    }
}

void AutoUpdateNotify::recheckNow()
{
    _p->isInDetect = false;
    _p->isForceUpdate = false;
    _p->version = "";
    XWCWallet::getInstance()->SetUpdateNeeded(false);
    this->checkUpdate();
}
