#include "tabwidget.h"

#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QToolButton>
#include <QTabBar>
#include <QPushButton>

TabWidget::TabWidget(QWidget *parent)
    : QTabWidget(parent), m_addTab(new QWidget)
{
    addTab(m_addTab, "add");
    addTabCustom();
    connect(this, &QTabWidget::currentChanged, [this] (int index) {
        if (index == count() - 1) {
            addTabCustom();
        }
    });
}

void TabWidget::addTabCustom() {
    auto tab = new QWidget;
    auto gridTab = new QGridLayout(tab);
    auto scrollArea = new QScrollArea(tab);
    scrollArea->setWidgetResizable(true);
    auto scrollAreaWidgetContents = new QWidget();
    auto gridScroll = new QGridLayout(scrollAreaWidgetContents);
    auto comWidget = new ComWidget(scrollAreaWidgetContents);
    gridScroll->addWidget(comWidget, 0, 0, 1, 1);
    scrollArea->setWidget(scrollAreaWidgetContents);
    gridTab->addWidget(scrollArea, 0, 0, 1, 1);
    m_tabsComWidget.append(comWidget);
    insertTab(m_tabsComWidget.size() - 1, tab, "off");
    QToolButton *tb = new QToolButton(tabBar());
    tb->setText("✖");
    tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectLeftTab);
    tabBar()->setTabButton(m_tabsComWidget.size() - 1,
                           QTabBar::RightSide, tb);
    connect(tb, &QToolButton::clicked, [=] () {
        if (count() > 2) {
            removeTab(indexOf(tab));
        }
    });
    setCurrentWidget(tab);
}

ComWidget * TabWidget::comWidget(int index) {
    return m_tabsComWidget.at(index);
}

void TabWidget::tabRemoved(int index) {
    m_tabsComWidget.removeAt(index);
    QTabWidget::tabRemoved(index);
}

