#include "kkm_device.h"
#include <QDebug>

#include "serialport/qserialportinfo.h"
#include "delayclass.h"

//для FPrint-11ПТК
#define    MaxDataSize  46

#define    CNTRL_ENQ    0x5
#define    CNTRL_ACK    0x6
#define    CNTRL_STX    0x2
#define    CNTRL_ETX    0x3
#define    CNTRL_EOT    0x4
#define    CNTRL_NAK    0x15
#define    CNTRL_DLE    0x10

QByteArray KKM_Device::codeData(const QByteArray &src)
{
    QByteArray dst;

    if(src.size() > MaxDataSize)
        return dst;

    const char* src_data = src.constData();

    //добалвяем STX
    dst.append(CNTRL_STX);

    //маскируем данные
    for(int n = 0; n < src.size(); ++n){
        if(src_data[n] == CNTRL_DLE || src_data[n] == CNTRL_ETX)
            dst.append(CNTRL_DLE);

        dst.append(src_data[n]);
    }

    //добавляем ETX
    dst.append(CNTRL_ETX);

    //считаем CRC для данных + ETX
    char* dst_data = dst.data();
    char crc = dst_data[1];
    for(int n = 2; n < dst.size(); ++n){
        crc = crc^dst_data[n];
    }

    //записываем CRC
    dst.append(crc);
    return dst;
}

QByteArray KKM_Device::encodeData(const QByteArray &src)
{
    QByteArray arr;

    //размаскирование данных
    const char *src_data = src.constData();
    for(int n = 1; n < src.size() - 2; ++n){
        if(src_data[n] == CNTRL_DLE)
            ++n;

        arr.append(src_data[n]);
    }

    return arr;
}

bool KKM_Device::checkCRC(const QByteArray &src)
{
    //проверка crc
    const char* src_data = src.constData() + 1;
    char  crc = *src_data;
    for(int n = 1; n < src.size() - 2; ++n)
        crc = crc^src_data[n];

    return crc == src.at(src.size() - 1);
}

QString KKM_Device::symbol(const char ch)
{
    QString str;
    switch(ch){
        case CNTRL_ACK: str = "ACK"; break;
        case CNTRL_DLE: str = "DLE"; break;
        case CNTRL_ENQ: str = "ENQ"; break;
        case CNTRL_EOT: str = "EOT"; break;
        case CNTRL_ETX: str = "ETX"; break;
        case CNTRL_NAK: str = "NAK"; break;
        case CNTRL_STX: str = "STX"; break;
    default:
        str.append(ch);
    }
    return str;
}

KKM_Device::KKM_Device(QObject *parent) : QObject(parent)
{
    _port = 0;
}

bool KKM_Device::init()
{
    QList<QSerialPortInfo>  lst = QSerialPortInfo::availablePorts();
    foreach(const QSerialPortInfo  &info, lst){
        qDebug() << "Find serial port: " << info.description();
        if(info.description().contains("FPrint")){
            _port = new QSerialPort(info);
            if(_port->open(QIODevice::ReadWrite)){
                _port->setBaudRate(QSerialPort::Baud115200);
                _port->setDataBits(QSerialPort::Data8);
                _port->setFlowControl(QSerialPort::NoFlowControl);
                _port->setParity(QSerialPort::NoParity);
                _port->setStopBits(QSerialPort::OneStop);

                qDebug() << "Succesfull open port name: " << info.portName();
                return true;
            }
            qDebug() << "Can't open port name: " << info.portName();
            delete _port;
            _port = 0;
        }
    }

    qDebug() << "Can't find any port";
    return false;
}

bool KKM_Device::sendCommand(const QByteArray &arr, QByteArray* res, const int timeout_T5)
{
    static const int timeout_T1 = 500;
    static const int timeout_T2 = 2000;
    static const int timeout_T3 = 500;
    static const int timeout_T4 = 500;
    static const int timeout_T6 = 500;
    static const int timeout_T7 = 500;

    QByteArray   cod = codeData(arr);

    char    ch;
    int     frc, rc;
    for(frc = 0; frc < 100; ++frc){

        bool needLoop;
        for(rc = 0; rc < 5; ++rc){
            needLoop = false;

            ch = CNTRL_ENQ;
            _port->write(&ch, 1);
            _port->waitForBytesWritten(-1);
            qDebug() << "send: ENQ";

            if(_port->waitForReadyRead(timeout_T1) == false){
                qDebug() << "T1 is out";
                continue;
            }

            _port->read(&ch, 1);
            qDebug() << "read: " << symbol(ch);

            if(ch == CNTRL_ACK)
                break;

            if(ch == CNTRL_ENQ){
                //T7 delay
                _delay->delay(timeout_T7);
                needLoop = true;
                break;
            }

            if(ch != CNTRL_NAK){
                needLoop = true;
                break;
            }

            //T1 delay
            _delay->delay(timeout_T1);
        }

        if(needLoop == false)
            break;
    }

    if( rc == 5 || frc == 100 ){ //No connection

        ch = CNTRL_EOT;
        _port->write(&ch, 1);
        _port->waitForBytesWritten(-1);
        qDebug() << "send: EOT";

        return false;
    }

    //Посылка данных
    for(frc = 0; frc < 10; ++frc){

        _port->write(cod);
        _port->waitForBytesWritten(-1);
        qDebug() << "send data";

        if(_port->waitForReadyRead(timeout_T3) == false){
            qDebug() << "T3 is out";
            continue;
        }

        _port->read(&ch, 1);
        qDebug() << "read: " << symbol(ch);

        if(ch == CNTRL_ENQ && frc > 0)
            break;

        if(ch == CNTRL_ACK)
            break;
    }

    if(ch != CNTRL_ENQ || frc == 10){
        char ch2 = CNTRL_EOT;
        _port->write(&ch2, 1);
        _port->waitForBytesWritten(-1);
        qDebug() << "send: EOT";

        if( frc == 10 ) //No connection
            return false;
    }

    if(res == 0)   //Command sended and no answer
        return true;

    //Ожидание ответа
    if(ch == CNTRL_ACK){

        for(rc = 0; rc < 100; ++rc){
            if(_port->waitForReadyRead(timeout_T5) == false){
                qDebug() << "T5 is out";
                return false;  //No answer
            }

            _port->read(&ch, 1);
            qDebug() << "read: " << symbol(ch);

            if(ch == CNTRL_ENQ)
                break;
        }

        if(rc == 100) //No answer
            return false;
    }

    //Приём данных
    QByteArray  arx;
    bool    sendAck = true;
    bool    needWait = true;
    for(frc = 0; frc < 10; ++frc){

        if(sendAck){
            ch = CNTRL_ACK;
            _port->write(&ch, 1);
            _port->waitForBytesWritten(-1);
            qDebug() << "send: ACK";
            sendAck = false;
        }

        if(needWait){

            for(rc = 0; rc < 100; ++rc){

                if(_port->waitForReadyRead(timeout_T2) == false){
                    qDebug() << "T2 is out";
                    return false; //No answer
                }

                _port->read(&ch, 1);
                qDebug() << "read: " << symbol(ch);

                if(ch == CNTRL_ENQ || ch == CNTRL_STX)
                    break;
            }

            if(rc == 100)
                return false; //No answer

            if(ch == CNTRL_ENQ){
                sendAck = true;
                continue;
            }
            needWait = false;
        }


        //read answer
        arx.clear(); arx.append(ch); //ch == STX
        bool    DLE_flag = false;

        for(rc = 0; rc < 64; ++rc) {
            if(_port->atEnd())
                if(_port->waitForReadyRead(timeout_T6) == false){
                    qDebug() << "T6 is out";
                    continue;
                }

            qDebug() << "read data";
            while(_port->atEnd() == false){
                _port->read(&ch, 1);
                arx.append(ch);

                if(DLE_flag)
                    DLE_flag = false;
                else {
                    if(ch == CNTRL_DLE)
                        DLE_flag = true;
                    else if(ch == CNTRL_ETX)
                        break;
                }
            }

            if(DLE_flag == false && ch == CNTRL_ETX){
                qDebug() << "read: ETX";
                break;
            }
        }

        if(rc == 64)
            continue; //buffer overflow

        if(_port->atEnd())
            if(_port->waitForReadyRead(timeout_T6) == false){
                qDebug() << "T6 is out";
                continue;
            }

        _port->read(&ch, 1);
        qDebug() << "read: " << symbol(ch);

        arx.append(ch);

        if(checkCRC(arx) == false){
            ch = CNTRL_NAK;
            _port->write(&ch, 1);
            _port->waitForBytesWritten(-1);
            qDebug() << "send: NAK";
            continue;
        }

        ch = CNTRL_ACK;
        _port->write(&ch, 1);
        _port->waitForBytesWritten(-1);
        qDebug() << "send: ACK";

        if(_port->waitForReadyRead(timeout_T4) == false){
            qDebug() << "T4 is out";
            break;
        }

        _port->read(&ch, 1);
        qDebug() << "read: " << symbol(ch);

        if(ch == CNTRL_EOT)
            break;

        if(ch == CNTRL_STX)
            continue;

        if(_port->waitForReadyRead(timeout_T6) == false){
            qDebug() << "T6 is out";
            break;
        }

        _port->read(&ch, 1);
        qDebug() << "read: " << symbol(ch);

        needWait = true;
    }

    if(frc == 10)
        return false; //No answer

    *res = encodeData(arx);
    return true;
}
