#include "traywidget.h"

TrayIcon::TrayIcon(QWidget *parent)
  : QSystemTrayIcon(parent)
{
    setIcon(QIcon::fromTheme("wvdialer", QIcon(":/wvdialer.png")));
    setToolTip("WvDialer");
    hideAction = new QAction(QString("Down"), this);
    hideAction->setIcon (QIcon::fromTheme("down"));
    reloadAction = new QAction(QString("Reload connection"), this);
    reloadAction->setIcon (QIcon::fromTheme("view-refresh"));
    killAction = new QAction(QString("Kill connection"), this);
    killAction->setIcon (QIcon::fromTheme("dialog-close"));
    closeAction = new QAction(QString("Exit"), this);
    closeAction->setIcon (QIcon::fromTheme("application-exit"));

    trayIconMenu = new QMenu(parent);
    trayIconMenu->addAction(hideAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(reloadAction);
    trayIconMenu->addAction(killAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(closeAction);

    setContextMenu(trayIconMenu);
    setVisible(true);
}
