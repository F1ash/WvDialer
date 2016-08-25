#ifndef TRAYWIDGET_H
#define TRAYWIDGET_H

#include <QSystemTrayIcon>
#include <QMenu>
#include <QIcon>
#include <QDebug>

class TrayIcon : public QSystemTrayIcon
{
  Q_OBJECT
public :
  explicit TrayIcon(QWidget *parent = nullptr);

  QAction   *hideAction;
  QAction   *reloadAction;
  QAction   *killAction;
  QAction   *closeAction;
  void       setActionState(bool);

private :
  QMenu     *trayIconMenu;
};

#endif
