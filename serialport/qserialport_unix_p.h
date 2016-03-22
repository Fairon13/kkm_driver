/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSERIALPORT_UNIX_P_H
#define QSERIALPORT_UNIX_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qserialport_p.h"

#include <QtCore/qlockfile.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qstringlist.h>

#include <limits.h>
#include <termios.h>
#ifndef Q_OS_ANDROID
#ifdef Q_OS_LINUX
#  include <linux/serial.h>
#endif
#else
struct serial_struct {
    int     type;
    int     line;
    unsigned int    port;
    int     irq;
    int     flags;
    int     xmit_fifo_size;
    int     custom_divisor;
    int     baud_base;
    unsigned short  close_delay;
    char    io_type;
    char    reserved_char[1];
    int     hub6;
    unsigned short  closing_wait;
    unsigned short  closing_wait2;
    unsigned char   *iomem_base;
    unsigned short  iomem_reg_shift;
    unsigned int    port_high;
    unsigned long   iomap_base;
};
#define ASYNC_SPD_CUST  0x0030
#define ASYNC_SPD_MASK  0x1030
#define PORT_UNKNOWN    0
#endif

QT_BEGIN_NAMESPACE

QString serialPortLockFilePath(const QString &portName);

class QSocketNotifier;

class QSerialPortPrivate : public QSerialPortPrivateData
{
    Q_DECLARE_PUBLIC(QSerialPort)

public:
    QSerialPortPrivate(QSerialPort *q);

    bool open(QIODevice::OpenMode mode);
    void close();

    QSerialPort::PinoutSignals pinoutSignals();

    bool setDataTerminalReady(bool set);
    bool setRequestToSend(bool set);

    bool flush();
    bool clear(QSerialPort::Directions directions);

    bool sendBreak(int duration);
    bool setBreakEnabled(bool set);

    qint64 readData(char *data, qint64 maxSize);

    bool waitForReadyRead(int msecs);
    bool waitForBytesWritten(int msecs);

    bool setBaudRate();
    bool setBaudRate(qint32 baudRate, QSerialPort::Directions directions);
    bool setDataBits(QSerialPort::DataBits dataBits);
    bool setParity(QSerialPort::Parity parity);
    bool setStopBits(QSerialPort::StopBits stopBits);
    bool setFlowControl(QSerialPort::FlowControl flowControl);

    void setError(const QSerialPortErrorInfo &errorInfo);

    bool readNotification();
    bool startAsyncWrite();
    bool completeAsyncWrite();

    qint64 writeData(const char *data, qint64 maxSize);

    static qint32 settingFromBaudRate(qint32 baudRate);

    static QList<qint32> standardBaudRates();

    struct termios restoredTermios;
    int descriptor;

    QSocketNotifier *readNotifier;
    QSocketNotifier *writeNotifier;

    bool emittedReadyRead;
    bool emittedBytesWritten;

    qint64 pendingBytesWritten;
    bool writeSequenceStarted;

    QScopedPointer<QLockFile> lockFileScopedPointer;

private:
    bool initialize(QIODevice::OpenMode mode);
    bool setTermios(const termios *tio);
    bool getTermios(termios *tio);

    bool setStandardBaudRate(qint32 baudRate, QSerialPort::Directions directions);
    bool setCustomBaudRate(qint32 baudRate, QSerialPort::Directions directions);
    QSerialPortErrorInfo getSystemError(int systemErrorCode = -1) const;

    bool isReadNotificationEnabled() const;
    void setReadNotificationEnabled(bool enable);
    bool isWriteNotificationEnabled() const;
    void setWriteNotificationEnabled(bool enable);

    bool waitForReadOrWrite(bool *selectForRead, bool *selectForWrite,
                            bool checkRead, bool checkWrite,
                            int msecs);

    qint64 readFromPort(char *data, qint64 maxSize);
    qint64 writeToPort(const char *data, qint64 maxSize);

#ifndef CMSPAR
    qint64 writePerChar(const char *data, qint64 maxSize);
#endif

};

QT_END_NAMESPACE

#endif // QSERIALPORT_UNIX_P_H
