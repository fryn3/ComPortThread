#ifndef COMWIDGET_H
#define COMWIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QThread>
#include "comconnect.h"

QT_BEGIN_NAMESPACE  // may be delete

namespace Ui {
class ComWidget;
}

class QIntValidator;    // may be delete

QT_END_NAMESPACE    //may be delete

class SetFocusFilter: public QObject {
protected:
    virtual bool eventFilter(QObject*, QEvent*) override;
public:
    SetFocusFilter(QObject* o = nullptr) : QObject (o) {}
};

class ComWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ComWidget(QWidget *parent = nullptr);
    ~ComWidget();

public slots:
    void showPortInfo(int idx);
    void fillPortsInfo();
    void slCloseSerialPort();
    void slOpenSerialPort();
private slots:
    void checkCustomBaudRatePolicy(int idx);
    void checkCustomDevicePathPolicy(int idx);
    void rxMsg(QByteArray msg);
private:
    void fillPortsParameters();
    ComPort::Settings getSettings();

private:
    Ui::ComWidget *m_ui;
    ComPort::Connect *m_comPort = nullptr;
    QThread *m_thread = nullptr;
};

#endif // COMWIDGET_H
