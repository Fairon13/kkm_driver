#include <QCoreApplication>
#include <QSettings>

#include "kkm_device.h"
#include "kkm_protocol.h"
#include "delayclass.h"

#include "httplistener.h"
#include "httphandler.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    KKM_Device  dev;
    if(dev.init() == false)
        return 0;

    DelayClass  delayer;
    delayer.start();

    dev.setDelayer(&delayer);

    KKM_Protocol    prot;
    prot.init(&dev, &delayer);

    QSettings       conf("./settings.ini", QSettings::IniFormat);
    conf.setValue("port", 12000);

    HttpListener    http(&conf, new HttpHandler(&prot), &a);

    return a.exec();
}
