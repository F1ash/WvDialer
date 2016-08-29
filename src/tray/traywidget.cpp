#include "traywidget.h"

TrayIcon::TrayIcon(QWidget *parent)
  : QSystemTrayIcon(parent)
{
    setIcon(QIcon::fromTheme("wvdialer", QIcon(":/wvdialer.png")));
    setToolTip("WvDialer");
    hideAction = new QAction(QString("Up"), this);
    hideAction->setIcon (
                QIcon::fromTheme("up", QIcon(":/up.png")));
    startAction = new QAction(QString("Start connection"), this);
    startAction->setIcon (
                QIcon::fromTheme("view-refresh", QIcon(":/view-refresh.png")));
    stopAction = new QAction(QString("Stop connection"), this);
    stopAction->setIcon (
                QIcon::fromTheme("delete", QIcon(":/delete.png")));
    closeAction = new QAction(QString("Exit"), this);
    closeAction->setIcon (
                QIcon::fromTheme("exit", QIcon(":/exit.png")));

    trayIconMenu = new QMenu(parent);
    trayIconMenu->addAction(hideAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(startAction);
    trayIconMenu->addAction(stopAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(closeAction);

    setContextMenu(trayIconMenu);
    setVisible(true);
}
