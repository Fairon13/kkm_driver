#ifndef KKM_DEVICE_H
#define KKM_DEVICE_H

#include <QObject>
#include "serialport/qserialport.h"

class DelayClass;
class KKM_Device : public QObject
{
    Q_OBJECT

    QSerialPort*    _port;
    DelayClass*     _delay;

    QByteArray  codeData(const QByteArray& src);
    QByteArray  encodeData(const QByteArray& src);
    bool        checkCRC(const QByteArray& src);

    static QString     symbol(const char ch);

public:
    explicit KKM_Device(QObject *parent = 0);
    bool    init();
    void    setDelayer(DelayClass*  delay) { _delay = delay; }

    bool    sendCommand(const QByteArray& arr, QByteArray *res = 0, const int timeout_T5 = 10000);

signals:

public slots:
};

#endif // KKM_DEVICE_H
