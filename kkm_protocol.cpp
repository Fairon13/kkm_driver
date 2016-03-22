#include "kkm_protocol.h"
#include "delayclass.h"
#include "kkm_device.h"
#include "QTextCodec"

const char* KKM_Protocol::toBCD_5(int val)
{
    static char dst[5] = {0};

    for(int n = 4; n >= 0; --n){
        char tmp_1 = val % 10; val /= 10;
        char tmp_2 = val % 10; val /= 10;
        tmp_1 |= (tmp_2 << 4);
        dst[n] = tmp_1;
    }

    return dst;
}

KKM_Protocol::KKM_Protocol(QObject *parent) : QObject(parent)
{
    _dev = 0;
}

bool KKM_Protocol::init(KKM_Device *dev, DelayClass *delayer)
{
    _dev = dev;
    _delayer = delayer;

    return true;
}

void KKM_Protocol::beep()
{
    QByteArray    cmd;

    cmd.append(char(0));
    cmd.append(char(0));
    cmd.append(char(0x47));

    _dev->sendCommand(cmd);
}

void KKM_Protocol::deviceType()
{
    QByteArray    cmd;

    cmd.append(char(0));
    cmd.append(char(0));
    cmd.append(char(0xa5));

    QByteArray  res;
    _dev->sendCommand(cmd, &res);
}

bool KKM_Protocol::exitMode()
{
    QByteArray    cmd;

    cmd.append(char(0));
    cmd.append(char(0));
    cmd.append(char(0x48));

    QByteArray  res;
    if(_dev->sendCommand(cmd, &res) == false)
        return false;

    if(res.at(0) != 0x55)
        return false;

    if(res.at(1) != 0)
        return false;

    return true;
}

bool KKM_Protocol::setMode(const char mode)
{
    char state, flags;
    if(getState(state, flags) == false)
        return false;

    if(state == mode)
        return true;

    if(exitMode() == false)
        return false;

    if(getState(state, flags) == false)
        return false;

    if(state != 0)
        return false;

    if(mode == 0)
        return true;

    QByteArray    cmd;

    cmd.append(char(0));
    cmd.append(char(0));
    cmd.append(char(0x56));
    cmd.append(mode);
    cmd.append(char(0));
    cmd.append(char(0));
    cmd.append(char(0));
    cmd.append(char(0x30));

    QByteArray  res;
    if(_dev->sendCommand(cmd, &res) == false)
        return false;

    if(res.at(0) != 0x55)
        return false;

    if(res.at(1) != 0)
        return false;

    return true;
}

bool KKM_Protocol::getState(char &state, char &flags)
{
    QByteArray    cmd;

    cmd.append(char(0));
    cmd.append(char(0));
    cmd.append(char(0x45));

    QByteArray  res;
    if(_dev->sendCommand(cmd, &res) == false)
        return false;

    if(res.at(0) != 0x55)
        return false;

    state = res.at(1);
    flags = res.at(2);
    return true;
}

bool KKM_Protocol::printX()
{
    if(setMode(2) == false)
        return false;

    QByteArray    cmd;

    cmd.append(char(0));
    cmd.append(char(0));
    cmd.append(char(0x67));
    cmd.append(char(1));

    QByteArray  res;
    if(_dev->sendCommand(cmd, &res) == false)
        return false;

    if(res.at(0) != 0x55)
        return false;

    if(res.at(1) != 0)
        return false;

    char    state, flags;
    do{
        _delayer->delay(500);

        if(getState(state, flags) == false)
            return false;
    }while(state != 0x02);

    if(flags & 3)
        return false;

    return true;
}

bool KKM_Protocol::printZ()
{
    if(setMode(3) == false)
        return false;

    QByteArray    cmd;

    cmd.append(char(0));
    cmd.append(char(0));
    cmd.append(char(0x5a));

    QByteArray  res;
    if(_dev->sendCommand(cmd, &res) == false)
        return false;

    if(res.at(0) != 0x55)
        return false;

    if(res.at(1) != 0)
        return false;

    char    state, flags;
    do{
        _delayer->delay(500);

        if(getState(state, flags) == false)
            return false;
    }while(state == 0x23);

    if(state == 0x03)
        return true;

    if(state != 0x17)
        return false;

    do{
        _delayer->delay(500);

        if(getState(state, flags) == false)
            return false;
    }while(state == 0x17);

    return true;
}

bool KKM_Protocol::print(const QString &str)
{
    QByteArray    cmd;
    cmd.append(char(0));
    cmd.append(char(0));
    cmd.append(char(0x4c));

    QTextCodec* cdc = QTextCodec::codecForName("IBM-866");
    QByteArray txt = cdc->fromUnicode(str);
    const char*   data = txt.constData();
    int txtSize = qMin(txt.size(), 32);
    for(int n = 0; n < txtSize; ++n){
        const unsigned char ch = data[n];
        if(ch == 0xfc)
            cmd.append(char(0x24));
        else if(ch == 0x24)
            cmd.append(char(0xfc));
        else
            cmd.append(ch);
    }

    QByteArray  res;
    if(_dev->sendCommand(cmd, &res) == false)
        return false;

    if(res.at(0) != 0x55)
        return false;

    if(res.at(1) != 0)
        return false;

    return true;
}

bool KKM_Protocol::printCliche()
{
    QByteArray    cmd;
    cmd.append(char(0));
    cmd.append(char(0));
    cmd.append(char(0x6c));

    QByteArray  res;
    if(_dev->sendCommand(cmd, &res) == false)
        return false;

    if(res.at(0) != 0x55)
        return false;

    if(res.at(1) != 0)
        return false;

    return true;
}

bool KKM_Protocol::registration(const char flags, const float price, const float count, const char section)
{
    const char*   bcd;
    QByteArray    cmd;
    cmd.append(char(0));
    cmd.append(char(0));
    cmd.append(char(0x52));
    cmd.append(flags);

    bcd = toBCD_5(price*100.f);
    cmd.append(bcd, 5);

    bcd = toBCD_5(count*1000.f);
    cmd.append(bcd, 5);

    cmd.append(section);

    QByteArray  res;
    if(_dev->sendCommand(cmd, &res) == false)
        return false;

    if(res.at(0) != 0x55)
        return false;

    if(res.at(1) != 0)
        return false;

    return true;
}

bool KKM_Protocol::openCheck(const char flags)
{
    QByteArray    cmd;
    cmd.append(char(0));
    cmd.append(char(0));
    cmd.append(char(0x92));
    cmd.append(flags);
    cmd.append(1);

    QByteArray  res;
    if(_dev->sendCommand(cmd, &res) == false)
        return false;

    if(res.at(0) != 0x55)
        return false;

    if(res.at(1) != 0)
        return false;

    return true;
}

bool KKM_Protocol::cancelCheck()
{
    QByteArray    cmd;
    cmd.append(char(0));
    cmd.append(char(0));
    cmd.append(char(0x59));

    QByteArray  res;
    if(_dev->sendCommand(cmd, &res) == false)
        return false;

    if(res.at(0) != 0x55)
        return false;

    if(res.at(1) != 0)
        return false;

    return true;
}

bool KKM_Protocol::openSession()
{
    QByteArray    cmd;
    cmd.append(char(0));
    cmd.append(char(0));
    cmd.append(char(0x9a));

    QByteArray  res;
    if(_dev->sendCommand(cmd, &res) == false)
        return false;

    if(res.at(0) != 0x55)
        return false;

    if(res.at(1) != 0)
        return false;

    return true;
}

bool KKM_Protocol::thr_beep()
{
    QMetaObject::invokeMethod(this, "beep", Qt::BlockingQueuedConnection);
    return true;
}

bool KKM_Protocol::thr_printX()
{
    bool    res = false;
    QMetaObject::invokeMethod(this, "printX", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, res));
    return res;
}

bool KKM_Protocol::thr_print(const QString &text)
{
    bool    res = false;
    QMetaObject::invokeMethod(this, "print", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, res), Q_ARG(QString, text));
    return res;
}

bool KKM_Protocol::thr_checkOpen()
{
    bool    res = false;
    QMetaObject::invokeMethod(this, "checkOpen", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, res));
    return res;
}

bool KKM_Protocol::thr_checkAddEntry(const QString &desc, const float price, const float count, const char section)
{
    bool    res = false;
    QMetaObject::invokeMethod(this, "checkAddEntry", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, res), Q_ARG(QString, desc), Q_ARG(float, price), Q_ARG(float, count), Q_ARG(char, section));
    return res;
}

bool KKM_Protocol::thr_checkClose(const float sum, const float discount)
{
    if(sum == 0.f)
        return false;

    bool    res = false;
    QMetaObject::invokeMethod(this, "checkClose", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, res), Q_ARG(float, sum), Q_ARG(float, discount));
    return res;
}

bool KKM_Protocol::thr_checkCancel()
{
    bool    res = false;
    QMetaObject::invokeMethod(this, "checkCancel", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, res));
    return res;
}

bool KKM_Protocol::thr_checkPrint(QList<CheckEntry> lst, const float sum, const float discount)
{
    bool    res = false;
    QMetaObject::invokeMethod(this, "checkPrint", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, res), Q_ARG(QList<CheckEntry>, lst), Q_ARG(float, sum), Q_ARG(float, discount));
    return res;
}

bool KKM_Protocol::thr_sessionOpen()
{
    bool    res = false;
    QMetaObject::invokeMethod(this, "sessionOpen", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, res));
    return res;
}

bool KKM_Protocol::thr_sessionClose()
{
    bool    res = false;
    QMetaObject::invokeMethod(this, "sessionClose", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, res));
    return res;
}

bool KKM_Protocol::closeCheck(const char payType, const float sum)
{
    QByteArray    cmd;
    cmd.append(char(0));
    cmd.append(char(0));
    cmd.append(char(0x4a));
    cmd.append(char(0));
    cmd.append(payType);
    const char* bcd = toBCD_5(sum*100.f);
    cmd.append(bcd, 5);

    QByteArray  res;
    if(_dev->sendCommand(cmd, &res) == false)
        return false;

    if(res.at(0) != 0x55)
        return false;

    if(res.at(1) != 0)
        return false;

    return true;
}

bool KKM_Protocol::discountCheck(const float discount)
{
    QByteArray    cmd;
    cmd.append(char(0));
    cmd.append(char(0));
    cmd.append(char(0x43));
    cmd.append(char(0)); //flags
    cmd.append(char(0)); //area
    cmd.append(char(1)); //type
    cmd.append(char(0)); //sign
    const char* bcd = toBCD_5(discount*100.f);
    cmd.append(bcd, 5);

    QByteArray  res;
    if(_dev->sendCommand(cmd, &res) == false)
        return false;

    if(res.at(0) != 0x55)
        return false;

    if(res.at(1) != 0)
        return false;

    return true;
}

bool KKM_Protocol::checkOpen()
{
    return checkCancel();
}

bool KKM_Protocol::checkAddEntry(const QString& desc, const float price, const float count, const char section)
{
    CheckEntry  ent;

    ent.desc = desc;
    ent.price = price;
    ent.count = count;
    ent.section = section;

    _check.append(ent);

    return true;
}

bool KKM_Protocol::checkAddEntry(const CheckEntry &ent)
{
    _check.append(ent);
    return true;
}

bool KKM_Protocol::checkAddEntry(const QList<CheckEntry> &lst)
{
    _check.append(lst);
    return true;
}

bool KKM_Protocol::checkClose(const float sum, const float discount)
{
    if(_check.count() == 0)
        return true;

    if(setMode(1) == false)
        return false;

    if(openCheck(0) == false)
        return false;

    foreach(const CheckEntry &ent, _check){

        if(registration(1, ent.price, ent.count, ent.section) == false){
            cancelCheck();
            return false;
        }

        if(print(ent.desc) == false){
            cancelCheck();
            return false;
        }

        if(registration(0, ent.price, ent.count, ent.section) == false){
            cancelCheck();
            return false;
        }

    }

    if(discount > 0.f){
        if(discountCheck(discount) == false)
            return false;
    }

    if(closeCheck(1, sum) == false){
        cancelCheck();
        return false;
    }

    return true;
}

bool KKM_Protocol::checkCancel()
{
    _check.clear();
    return true;
}

bool KKM_Protocol::checkPrint(QList<CheckEntry> lst, const float sum, const float discount)
{
    if(checkOpen() == false)
        return false;

    if(checkAddEntry(lst) == false)
        return false;

    if(checkClose(sum, discount) == false)
        return false;

    return true;
}

bool KKM_Protocol::sessionOpen()
{
    if(setMode(1) == false)
        return false;

    if(openSession() == false)
        return false;

    return true;
}

bool KKM_Protocol::sessionClose()
{
    return printZ();
}
