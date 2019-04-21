#include "monitorwidget.h"
#include "ui_monitorwidget.h"

MonitorWidget::MonitorWidget(bool isHex, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MonitorWidget), m_isHex(isHex),
    m_actSend("send msg")
{
    ui->setupUi(this);
    connect(ui->btnMonitorClear, &QPushButton::clicked,
            ui->pteMonitor, &QPlainTextEdit::clear);
    connect(&m_actSend, &QAction::triggered, this, &MonitorWidget::send);
    connect(ui->btnSend, &QPushButton::clicked, &m_actSend, &QAction::trigger);
    connect(ui->leSend, &QLineEdit::textChanged, this, &MonitorWidget::slSendComandChange);
    m_msgTimer.restart();
}

MonitorWidget::~MonitorWidget()
{
    delete ui;
}

void MonitorWidget::rxData(QByteArray rxMsg)
{
    auto time = m_msgTimer.restart();
    HistoryStruct item { false, rxMsg, time };
    printMsg(item);

    m_historyRxTx.append(item);
}

void MonitorWidget::send()
{
    auto time = m_msgTimer.restart();
    QByteArray data = convertToSend(ui->leSend->text(), m_isHex);
    HistoryStruct item { true, data, time };
    printMsg(item);
    m_historyRxTx.append(item);
    m_historyTx.removeOne(data);
    m_historyTx.append(data);
    ui->leSend->clear();
}

QByteArray MonitorWidget::convertToSend(QString txMsg, bool isHex)
{
    if (isHex) {
        return QByteArray::fromHex(txMsg.toLatin1());
    } else {
        txMsg.replace("\\r", "\r");
        return txMsg.toLatin1();
    }
}

void MonitorWidget::printMsg(bool isTx, QByteArray data, qint64 time)
{
    if (isTx) {
        ui->pteMonitor->appendHtml(QString("<div style='color:black;'>Tx (size = %1, time = %3ms):<pre>%2</pre></div>")
                          .arg(data.size())
                          .arg(convertToPrint(data, m_isHex))
                          .arg(time));
    } else {
        ui->pteMonitor->appendHtml(QString("<div style='color:green;'>Rx (size = %1, time = %3ms):<pre>%2</pre></div>")
                          .arg(data.size())
                          .arg(convertToPrint(data, m_isHex))
                          .arg(time));
    }
}

void MonitorWidget::printMsg(HistoryStruct histItem)
{
    printMsg(histItem.isTx, histItem.data, histItem.time);
}

QString MonitorWidget::convertToPrint(QByteArray hex, bool isHex)
{
    if (isHex) {
        return hex.toHex(' ').toUpper().data();
    } else {
        return QString(hex).replace("\r", "\\r");
    }
}

void MonitorWidget::keyPressEvent(QKeyEvent *event)
{
    static QByteArray textLineEditUp;
    if (!event->text().isEmpty() && ui->leSend->isEnabled()) {
        if (event->key() == Qt::Key_Enter
                || event->key() == Qt::Key_Return) {
            send();
        } else if (event->key() == Qt::Key_Escape) {
            ui->pteMonitor->clear();
        } else {
            auto hexKey = ((Qt::Key_0 <= event->key() && event->key() <= Qt::Key_9)
                           || (Qt::Key_A <= event->key() && event->key() <= Qt::Key_F));
            auto textKey = ((Qt::Key_Plus <= event->key() && event->key() <= Qt::Key_9)
                            || (Qt::Key_A <= event->key() && event->key() <= Qt::Key_Z));
            if ((m_isHex && hexKey) || (!m_isHex && textKey)) {
                ui->leSend->setFocus();
                ui->leSend->setText(ui->leSend->text().append(event->text()));
            }
        }
    } else if (event->key() == Qt::Key_Up) {
        if (m_indexHistory == 0) {
            textLineEditUp = convertToSend(ui->leSend->text(), m_isHex);
        }
        if (m_historyTx.size() > m_indexHistory) {
            ++m_indexHistory;
            ui->leSend->setText(convertToPrint(m_historyTx.at(m_historyTx.size() - m_indexHistory), m_isHex));
        }
    } else if (event->key() == Qt::Key_Down) {
        if (m_indexHistory > 0) {
            --m_indexHistory;
            if (m_indexHistory == 0) {
                ui->leSend->setText(textLineEditUp);
            } else {
                ui->leSend->setText(convertToPrint(m_historyTx.at(m_historyTx.size() - m_indexHistory), m_isHex));
            }
        }
    } else {
        QWidget::keyPressEvent(event);
    }
}

void MonitorWidget::slSendComandChange(const QString &newText) {
    auto le = dynamic_cast<QLineEdit *>(sender());
    Q_ASSERT(le);
    if (m_isHex) {  // особые правила только для hex
        int countSpaceLast = newText.count(" ");
        int cursPos = le->cursorPosition();
        QString t = newText;
        t.remove(QRegExp("[^a-fA-F0-9]*"));
        int countSpace = 0;
        int len = t.length();
        while ((len - 1) / 2 > countSpace ) {
            t.insert(len - 2 * ++countSpace, ' ');
        }
        t = t.toUpper();
        if (newText != t) {
            le->setText(t);
            le->setCursorPosition(cursPos + countSpace - countSpaceLast);
        }
    }
}
