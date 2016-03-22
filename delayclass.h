#ifndef DELAYCLASS_H
#define DELAYCLASS_H

#include <QThread>

class DelayClass : public QThread
{
    Q_OBJECT

public:
    DelayClass();
    void    delay(int msec);

public slots:
    void    slt_delay(int msec);

};

#endif // DELAYCLASS_H
