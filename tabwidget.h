#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QObject>
#include <QTabWidget>
#include "comwidget.h"

class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    TabWidget(QWidget *parent = nullptr);
    void addTabCustom();
    ComWidget *comWidget(int index);
private:
    QVector<ComWidget *> m_tabsComWidget;
    QWidget *m_addTab;
protected:
    void tabRemoved(int index) override;
};

#endif // TABWIDGET_H
