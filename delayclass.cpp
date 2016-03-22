#include "delayclass.h"

DelayClass::DelayClass() : QThread()
{
    moveToThread(this);
}

void DelayClass::delay(int msec)
{
    QMetaObject::invokeMethod(this, "slt_delay", Qt::BlockingQueuedConnection, Q_ARG(int, msec));
}

void DelayClass::slt_delay(int msec)
{
    msleep(msec);
}

