#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemWatcher>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QTimerEvent>
#include <QSettings>
#include <QtDBus/QDBusInterface>
#include "tray/traywidget.h"
#include <kauth.h>
#include <knotification.h>
using namespace KAuth;

enum SRV_STATUS {
    INACTIVE = -1,
    ACTIVE,
    FAILED,
    DEACTIVATING
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

signals:
    void                killed();

private:
    bool                deviceExist, reloadFlag;
    int                 PID, timerID;
    SRV_STATUS          srvStatus;
    QFileSystemWatcher *watcher;
    TrayIcon           *trayIcon;
    QVBoxLayout        *baseLayout;
    QWidget            *baseWdg;
    QScrollArea        *scrolled;
    QSettings           settings;
    bool                getFileExistanceState(const QString, const QString) const;
    bool                getDirExistanceState(const QString, const QString) const;

    QDBusInterface     *wvdialerUnit;
    bool                connected;

private slots:
    void                initTrayIcon();
    void                changeVisibility();
    void                trayIconActivated(QSystemTrayIcon::ActivationReason);
    void                createWvDialerAccessor();
    void                wvdialerUnitStatusReceiver(QDBusMessage);
    void                closeEvent(QCloseEvent*);
    void                timerEvent(QTimerEvent*);
    void                directoryChanged(QString);
    void                reloadConnection();
    void                killConnection();
    void                stopWvDialProcess();
    void                startWvDialProcess();
    SRV_STATUS          getServiceStatus();
};

#endif // MAINWINDOW_H
