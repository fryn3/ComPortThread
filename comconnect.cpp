#include <QEventLoop>

#include "comconnect.h"

ComPort::Connect::Connect(Settings s, bool useQueue, QObject *parent)
                            : QObject(parent), m_serial(new QSerialPort(this)), m_useQueue(useQueue) {
    m_serial->setPortName(s.name);
    m_serial->setBaudRate(s.baudRate);
    m_serial->setDataBits(s.dataBits);
    m_serial->setParity(s.parity);
    m_serial->setStopBits(s.stopBits);
    m_serial->setFlowControl(s.flowControl);
    connect(this, &Connect::stoped, [this] () { m_loop.exit(); });
}

void ComPort::Connect::start() {
    if (!m_serial->open(QIODevice::ReadWrite)) {
        emit stoped(1);
        return;
    }
    m_queue.clear();
    connect(m_serial, &QSerialPort::readyRead,
            this, &ComPort::Connect::_rxData);
    emit started();
    m_loop.exec();
}

void ComPort::Connect::stop() {
    if (m_serial->isOpen()) {
        m_serial->close();
    }
    emit stoped(0);
}

void ComPort::Connect::txMsg(QByteArray txMsg) {
    m_serial->write(txMsg);
}

void ComPort::Connect::_rxData() {
    auto msg = m_serial->readAll();
    if (m_useQueue) {
        m_queue.enqueue(msg);
    }
    emit rxMsg(msg);
}

void ComPort::Connect::readRxBuffSlot() {
    if (m_queue.isEmpty()) {
        emit readRxBuffSig(QByteArray());
        return;
    }
    emit readRxBuffSig(m_queue.dequeue());
}

