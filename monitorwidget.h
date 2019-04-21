#ifndef MONITORWIDGET_H
#define MONITORWIDGET_H

#include <QAction>
#include <QElapsedTimer>
#include <QWidget>

namespace Ui {
class MonitorWidget;
}

class MonitorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MonitorWidget(bool isHex = false, QWidget *parent = nullptr);
    ~MonitorWidget() override;
    struct HistoryStruct {
        bool isTx;
        QByteArray data;
        qint64 time;
    };
    bool isHexFormat() const { return m_isHex; }
public slots:
    void rxData(QByteArray rxMsg);
    void changeFormat() { m_isHex = !m_isHex; }
    void setFormat(bool isHex) { m_isHex = isHex; }
signals:
    void txData(QByteArray txMsg);

protected:
    static QByteArray convertToSend(QString txMsg, bool isHex);
    static QString convertToPrint(QByteArray hex, bool isHex);
    void keyPressEvent(QKeyEvent *event) override;
    void printMsg(bool isTx, QByteArray data, qint64 time);
    void printMsg(HistoryStruct histItem);
protected slots:
    void slSendComandChange(const QString &newText); // необходимо переделать в фильтр
    void send();
private:
    Ui::MonitorWidget *ui;
    bool m_isHex;
    QAction m_actSend;
    int m_indexHistory = 0;
    QVector<HistoryStruct> m_historyRxTx;
    QVector<QByteArray> m_historyTx;
    QElapsedTimer m_msgTimer;
};

#endif // MONITORWIDGET_H
