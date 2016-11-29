#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemWatcher>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QSettings>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include "tray/traywidget.h"
#include <kauth.h>
#include <knotification.h>
using namespace KAuth;

// ActiveState contains a state value
// that reflects whether the unit is currently active or not.
enum SRV_STATUS {
    INACTIVE = -1,
    ACTIVE,
    FAILED,
    ACTIVATING,
    DEACTIVATING,
    RELOADING
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    bool                deviceExist, startFlag;
    SRV_STATUS          srvStatus;
    QFileSystemWatcher *watcher;
    TrayIcon           *trayIcon;
    QVBoxLayout        *baseLayout;
    QWidget            *baseWdg;
    QScrollArea        *scrolled;
    QSettings           settings;
    QDBusConnection     connection;
    QString             currSrvName;

    void                initTrayIcon();
    bool                getFileExistanceState(const QString, const QString) const;
    void                connectToWvDialerService();
    bool                checkServiceStatus();
    void                serviceStatusChanged();
    void                startWvDialProcess();
    void                stopWvDialProcess();

private slots:
    void                changeVisibility();
    void                trayIconActivated(QSystemTrayIcon::ActivationReason);
    void                servicePropertyChanged(QDBusMessage);
    void                closeEvent(QCloseEvent*);
    void                directoryChanged(QString);
    void                startConnection();
    void                stopConnection();
    void                receiveServiceStatus(QDBusMessage);
};

#endif // MAINWINDOW_H
