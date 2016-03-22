#ifndef KKM_PROTOCOL_H
#define KKM_PROTOCOL_H

#include <QObject>

struct CheckEntry {
    QString desc;
    float   count; //количество в шт. или кг.
    float   price; //цена в рублях
    char    section; //секция
};

class KKM_Device;
class DelayClass;
class KKM_Protocol : public QObject
{
    Q_OBJECT

    KKM_Device*     _dev;
    DelayClass*   _delayer;

    QList<CheckEntry>   _check;

    static const char *toBCD_5(int val);

public:
    explicit KKM_Protocol(QObject *parent = 0);

    bool    init(KKM_Device* dev, DelayClass* delayer);

    void    deviceType();
    bool    exitMode();
    bool    setMode(const char mode);
    bool    getState(char &mode, char &flags);
    bool    printZ();
    bool    printCliche();
    bool    registration(const char flags, const float price, const float count, const char section);
    bool    openCheck(const char flags);
    bool    closeCheck(const char payType, const float sum);
    bool    discountCheck(const float discount);
    bool    cancelCheck();
    bool    openSession();

//Thread safe api
    bool    thr_beep();
    bool    thr_printX();
    bool    thr_print(const QString &text);

    //API check
    bool    thr_checkOpen();
    bool    thr_checkAddEntry(const QString &desc, const float price, const float count, const char section);
    bool    thr_checkClose(const float sum, const float discount);
    bool    thr_checkCancel();
    bool    thr_checkPrint(QList<CheckEntry> lst, const float sum, const float discount);

    //API session
    bool    thr_sessionOpen();
    bool    thr_sessionClose();

public slots:
    void    beep();
    bool    printX();
    bool    print(const QString &str);

    //API check
    bool    checkOpen();
    bool    checkAddEntry(const QString &desc, const float price, const float count, const char section);
    bool    checkAddEntry(const CheckEntry &ent);
    bool    checkAddEntry(const QList<CheckEntry> &lst);
    bool    checkClose(const float sum, const float discount);
    bool    checkCancel();
    bool    checkPrint(QList<CheckEntry> lst, const float sum, const float discount);

    //API session
    bool    sessionOpen();
    bool    sessionClose();
};

#endif // KKM_PROTOCOL_H
