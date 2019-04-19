#ifndef COMCONNECT_H
#define COMCONNECT_H

#include <QEventLoop>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QSerialPort>

namespace ComPort {

struct Settings {
    QString name;
    qint32 baudRate;
    QString stringBaudRate;
    QSerialPort::DataBits dataBits;
    QString stringDataBits;
    QSerialPort::Parity parity;
    QString stringParity;
    QSerialPort::StopBits stopBits;
    QString stringStopBits;
    QSerialPort::FlowControl flowControl;
    QString stringFlowControl;
};

class Connect : public QObject
{
    Q_OBJECT
public:
    Connect (Settings s, bool useQueue = true, QObject *parent = nullptr);
signals:
    void stoped(int status);
    void started(); // возможно не нужно
    void rxMsg(QByteArray rxMsg);
    void readRxBuffSig (QByteArray rxBuff);
public slots:
    void start();
    void stop();
    void txMsg(QByteArray txMsg);
    void readRxBuffSlot();
private slots:
    void _rxData();
private:
    QSerialPort *m_serial;
    QQueue<QByteArray> m_queue;
    QEventLoop m_loop;
    bool m_useQueue;
};

}   // ComPort
#endif // COMCONNECT_H
