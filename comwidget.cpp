#include "comwidget.h"
#include "ui_comwidget.h"

#include <QMessageBox>
#include <QSerialPortInfo>
#include <QDebug>
#include <QLineEdit>

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

bool SetFocusFilter::eventFilter(QObject *o, QEvent *e) {
    if (e->type() != QEvent::FocusIn
            && e->type() != QEvent::FocusOut
            && e->type() != QEvent::MouseButtonPress) {
        return QObject::eventFilter(o, e);
    }
    static QEvent::Type evLast[2] {};
    bool r = false;
    if (evLast[1] != QEvent::MouseButtonPress && evLast[0] == QEvent::FocusOut) {
        r = true;
    }
    evLast[1] = evLast[0];
    evLast[0] = e->type();
    if (r) {
        qDebug() << true;
        auto cw = dynamic_cast<ComWidget *>(o->parent());
        if (!cw) {
            qDebug() << "!cw";
            return QObject::eventFilter(o, e);
        }
        cw->fillPortsInfo();
        return true;
    }
    return QObject::eventFilter(o, e);
}

ComWidget::ComWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::ComWidget)
{
    m_ui->setupUi(this);
    m_ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);
    m_ui->serialPortInfoListBox->installEventFilter(new SetFocusFilter(m_ui->serialPortInfoListBox));

    connect(m_ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ComWidget::showPortInfo);
    connect(m_ui->baudRateBox,  QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ComWidget::checkCustomBaudRatePolicy);
    connect(m_ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ComWidget::checkCustomDevicePathPolicy);
    connect(m_ui->actConnect, &QAction::toggled, this, &ComWidget::slOpenSerialPort);
    connect(m_ui->btnConnect, &QPushButton::clicked, m_ui->actConnect, &QAction::toggle);
    connect(m_ui->btnDisconnect, &QPushButton::clicked,
            this, &ComWidget::slCloseSerialPort);

    fillPortsParameters();
    fillPortsInfo();
}

ComWidget::~ComWidget() { delete m_ui; }

void ComWidget::showPortInfo(int idx)
{
    if (idx == -1)
        return;

    const QStringList list = m_ui->serialPortInfoListBox->itemData(idx)
                                                            .toStringList();
    m_ui->descriptionLabel->setText(tr("Description: %1")
                                    .arg(list.count() > 1 ? list.at(1)
                                                          : tr(blankString)));
    m_ui->manufacturerLabel->setText(tr("Manufacturer: %1")
                                     .arg(list.count() > 2 ? list.at(2)
                                                           : tr(blankString)));
    m_ui->serialNumberLabel->setText(tr("Serial number: %1")
                                     .arg(list.count() > 3 ? list.at(3)
                                                           : tr(blankString)));
    m_ui->locationLabel->setText(tr("Location: %1")
                                 .arg(list.count() > 4 ? list.at(4)
                                                       : tr(blankString)));
    m_ui->vidLabel->setText(tr("Vendor Identifier: %1")
                            .arg(list.count() > 5 ? list.at(5)
                                                  : tr(blankString)));
    m_ui->pidLabel->setText(tr("Product Identifier: %1")
                            .arg(list.count() > 6 ? list.at(6)
                                                  : tr(blankString)));
}

void ComWidget::slOpenSerialPort()
{
    m_ui->btnConnect->setEnabled(true);
    m_ui->btnDisconnect->setEnabled(false);
    if (m_comPort) {
        disconnect(m_comPort, nullptr, nullptr, nullptr);
        delete m_comPort;
    }
    m_comPort = new ComPort::Connect(getSettings(), false);
    if (m_thread) {
        disconnect(m_thread, nullptr, nullptr, nullptr);
        delete m_thread;
    }
    m_thread = new QThread;
    m_comPort->moveToThread(m_thread);
    connect(m_thread, &QThread::started, m_comPort, &ComPort::Connect::start);
//    connect(m_ui->actDisconnect, &QAction::triggered, )
    connect(m_comPort, &ComPort::Connect::rxMsg, m_ui->wMonitor, &MonitorWidget::rxData);
    connect(m_ui->wMonitor, &MonitorWidget::txData, m_comPort, &ComPort::Connect::txMsg);
    connect(m_comPort, &ComPort::Connect::stoped,
            [=] (int i) { qDebug() << "finished:" << i; });
    connect(m_comPort, &ComPort::Connect::stoped,
            m_comPort, &ComPort::Connect::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    m_thread->start();
}

void ComWidget::slCloseSerialPort()
{
    m_comPort->stop();
//    if (m_serial->isOpen())
//        m_serial->close();
    m_ui->btnConnect->setEnabled(true);
    m_ui->btnDisconnect->setEnabled(false);
}

void ComWidget::checkCustomBaudRatePolicy(int idx)
{
    const bool isCustomBaudRate = !m_ui->baudRateBox->itemData(idx).isValid();
    m_ui->baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        m_ui->baudRateBox->clearEditText();
        QLineEdit *edit = m_ui->baudRateBox->lineEdit();
        edit->setValidator(new QIntValidator(0, 4000000, this));
    }
}

void ComWidget::checkCustomDevicePathPolicy(int idx)
{
    const bool isCustomPath = !m_ui->serialPortInfoListBox->itemData(idx).isValid();
    m_ui->serialPortInfoListBox->setEditable(isCustomPath);
    if (isCustomPath)
        m_ui->serialPortInfoListBox->clearEditText();
}

void ComWidget::fillPortsParameters()
{
    m_ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    m_ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    m_ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    m_ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    m_ui->baudRateBox->addItem(tr("Custom"));

    m_ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    m_ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    m_ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    m_ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    m_ui->dataBitsBox->setCurrentIndex(3);

    m_ui->parityBox->addItem(tr("None"), QSerialPort::NoParity);
    m_ui->parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    m_ui->parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    m_ui->parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    m_ui->parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);

    m_ui->stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    m_ui->stopBitsBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    m_ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    m_ui->flowControlBox->addItem(tr("None"), QSerialPort::NoFlowControl);
    m_ui->flowControlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    m_ui->flowControlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
}

void ComWidget::fillPortsInfo()
{
    m_ui->serialPortInfoListBox->clear();
    QString description;
    QString manufacturer;
    QString serialNumber;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        m_ui->serialPortInfoListBox->addItem(list.first(), list);
    }
//    m_ui->serialPortInfoListBox->addItem(tr("Custom"));
}

ComPort::Settings ComWidget::getSettings()
{
    ComPort::Settings settings;
    settings.name = m_ui->serialPortInfoListBox->currentText();

    if (m_ui->baudRateBox->currentIndex() == 4) {
        settings.baudRate = m_ui->baudRateBox->currentText().toInt();
    } else {
        settings.baudRate = static_cast<QSerialPort::BaudRate>(
                    m_ui->baudRateBox->itemData(m_ui->baudRateBox->currentIndex()).toInt());
    }
    settings.stringBaudRate = QString::number(settings.baudRate);

    settings.dataBits = static_cast<QSerialPort::DataBits>(
                m_ui->dataBitsBox->itemData(m_ui->dataBitsBox->currentIndex()).toInt());
    settings.stringDataBits = m_ui->dataBitsBox->currentText();

    settings.parity = static_cast<QSerialPort::Parity>(
                m_ui->parityBox->itemData(m_ui->parityBox->currentIndex()).toInt());
    settings.stringParity = m_ui->parityBox->currentText();

    settings.stopBits = static_cast<QSerialPort::StopBits>(
                m_ui->stopBitsBox->itemData(m_ui->stopBitsBox->currentIndex()).toInt());
    settings.stringStopBits = m_ui->stopBitsBox->currentText();

    settings.flowControl = static_cast<QSerialPort::FlowControl>(
                m_ui->flowControlBox->itemData(m_ui->flowControlBox->currentIndex()).toInt());
    settings.stringFlowControl = m_ui->flowControlBox->currentText();
    return settings;
}
