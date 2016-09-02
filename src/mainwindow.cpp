#include "mainwindow.h"
#include <QDir>
#include <private/qdbusutil_p.h>

#define DEV_DIR  QString("/dev")

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    connection(QDBusConnection::systemBus())
{
    setSizePolicy(
                QSizePolicy(
                    QSizePolicy::MinimumExpanding,
                    QSizePolicy::MinimumExpanding));
    setWindowTitle("WvDialer");
    QIcon::setThemeName("WvDialer");
    setWindowIcon(QIcon::fromTheme(
                      "wvdialer",
                      QIcon(":/wvdialer.png")));
    deviceExist = false;
    startFlag = true; // for first auto start
    srvStatus = INACTIVE;
    baseLayout = nullptr;
    baseWdg = nullptr;
    scrolled = nullptr;
    initTrayIcon();
    restoreGeometry(settings.value("Geometry").toByteArray());
    hide();
    connectToWvDialerService();
    watcher = new QFileSystemWatcher(this);
    connect(watcher, SIGNAL(directoryChanged(QString)),
            this, SLOT(directoryChanged(QString)));
    watcher->addPath(DEV_DIR);
}

void MainWindow::initTrayIcon()
{
    trayIcon = new TrayIcon(this);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
    connect(trayIcon->hideAction, SIGNAL(triggered(bool)),
            this, SLOT(changeVisibility()));
    connect(trayIcon->startAction, SIGNAL(triggered(bool)),
            this, SLOT(startConnection()));
    connect(trayIcon->stopAction, SIGNAL(triggered(bool)),
            this, SLOT(stopConnection()));
    connect(trayIcon->closeAction, SIGNAL(triggered(bool)),
            this, SLOT(close()));
    trayIcon->show();
}
void MainWindow::changeVisibility()
{
    if (this->isVisible()) {
        this->hide();
        trayIcon->hideAction->setText (QString("Up"));
        trayIcon->hideAction->setIcon (
                    QIcon::fromTheme("up", QIcon(":/up.png")));
    } else {
        this->show();
        trayIcon->hideAction->setText (QString("Down"));
        trayIcon->hideAction->setIcon (
                    QIcon::fromTheme("down", QIcon(":/down.png")));
    };
}
void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason r)
{
    if (r==QSystemTrayIcon::Trigger) changeVisibility();
}
bool MainWindow::getFileExistanceState(const QString _dir, const QString _file) const
{
    QString _filePath = QString("%1%2%3")
            .arg(_dir).arg(QDir::separator()).arg(_file);
    QFile f;
    f.setFileName(_filePath);
    return f.exists();
}
void MainWindow::connectToWvDialerService()
{
    if ( !connection.isConnected() ) return;
    connection = QDBusConnection::systemBus();
    bool connected = connection.connect(
                "org.freedesktop.systemd1",
                "/org/freedesktop/systemd1/unit/WvDialer_2eservice",
                "org.freedesktop.DBus.Properties",
                "PropertiesChanged",
                this,
                SLOT(servicePropertyChanged(QDBusMessage)));
    const QString _state = ( connected )? "Connected" : "Not connected";
    KNotification::event(
                KNotification::Notification,
                "WvDialer",
                QString("%1 to org.freedesktop.systemd1").arg(_state));
}
void MainWindow::servicePropertyChanged(QDBusMessage message)
{
    QList<QVariant> args = message.arguments();
    if ( args.first().toString()!=
         "org.freedesktop.systemd1.Unit" ) return;
    checkServiceStatus();
}
void MainWindow::closeEvent(QCloseEvent *ev)
{
    if ( ev->type()==QEvent::Close ) {
        stopConnection();
        settings.setValue("Geometry", saveGeometry());
        trayIcon->hide();
        ev->accept();
    };
}
void MainWindow::directoryChanged(QString dir)
{
    if ( dir==DEV_DIR ) {
        // WARNING: used ttyUSB0, because
        // in wvdial.conf used this device name
        bool newDeviceState = getFileExistanceState(dir, "ttyUSB0");
        if ( deviceExist!=newDeviceState ) {
            deviceExist = newDeviceState;
            QString msg = QString("Device in %1 is %2connected.")
                    .arg("/dev/ttyUSB0")
                    .arg((deviceExist)? "":"dis");
            KNotification::event(
                       KNotification::Notification,
                       "WvDialer",
                       msg);
            startFlag = deviceExist;
            if ( startFlag )
                checkServiceStatus();
        };
    };
}
void MainWindow::startConnection()
{
    deviceExist = getFileExistanceState(DEV_DIR, "ttyUSB0");
    if ( deviceExist ) {
        trayIcon->setIcon(
                    QIcon::fromTheme("wvdialer_reload",
                                     QIcon(":/wvdialer_reload.png")));
        startFlag = true;
        checkServiceStatus();
    } else {
        KNotification::event(
                    KNotification::Notification,
                    "WvDialer",
                    QString("Device %1 is not connected.")
                    .arg("/dev/ttyUSB0"));
    };
}
void MainWindow::stopConnection()
{
    startFlag = false;
    stopWvDialProcess();
}
void MainWindow::stopWvDialProcess()
{
    QVariantMap args;
    args["action"] = "stop";
    Action act("pro.russianfedora.wvdialer.stop");
    act.setHelperId("pro.russianfedora.wvdialer");
    act.setArguments(args);
    ExecuteJob *job = act.execute();
    job->setAutoDelete(true);
    if (job->exec()) {
        QString code = job->data().value("code").toString();
        QString msg  = job->data().value("msg").toString();
        QString err  = job->data().value("err").toString();
        KNotification::event(
                   KNotification::Notification,
                   "WvDialer",
                   QString("Wvdial session closed with exit code: %1\nMSG: %2\nERR: %3")
                   .arg(code).arg(msg).arg(err));
    } else {
        KNotification::event(
                   KNotification::Notification,
                   "WvDialer",
                   QString("ERROR: %1\n%2")
                   .arg(job->error()).arg(job->errorText()));
        trayIcon->setIcon(
                    QIcon::fromTheme("wvdialer_close",
                                     QIcon(":/wvdialer_close.png")));
    };
}
void MainWindow::startWvDialProcess()
{
    // if device was connected, then run wvdialer_helper
    if ( deviceExist ) {
        connectToWvDialerService();
        trayIcon->setIcon(
                    QIcon::fromTheme("wvdialer_reload",
                                     QIcon(":/wvdialer_reload.png")));
        QVariantMap args;
        Action act;
        switch (srvStatus) {
        //case INACTIVE:
        //    args["action"] = "create";
        //    act.setName("pro.russianfedora.wvdialer.create");
        //    break;
        case INACTIVE:
        case FAILED:
            args["action"] = "start";
            act.setName("pro.russianfedora.wvdialer.start");
            break;
        default:
            return;
        };
        act.setHelperId("pro.russianfedora.wvdialer");
        act.setArguments(args);
        ExecuteJob *job = act.execute();
        job->setAutoDelete(true);
        if (job->exec()) {
            QString code = job->data().value("code").toString();
            QString msg  = job->data().value("msg").toString();
            QString err  = job->data().value("err").toString();
            KNotification::event(
                       KNotification::Notification,
                       "WvDialer",
                       QString("Wvdial session open with exit code: %1\nMSG: %2\nERR: %3")
                       .arg(code).arg(msg).arg(err));
        } else {
            KNotification::event(
                       KNotification::Notification,
                       "WvDialer",
                       QString("ERROR: %1\n%2")
                       .arg(job->error()).arg(job->errorText()));
            trayIcon->setIcon(
                        QIcon::fromTheme("wvdialer_close",
                                         QIcon(":/wvdialer_close.png")));
        };
    };
}
bool MainWindow::checkServiceStatus()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
                "org.freedesktop.systemd1",
                "/org/freedesktop/systemd1/unit/WvDialer_2eservice",
                "org.freedesktop.DBus.Properties",
                "Get");
    QList<QVariant> _args;
    _args<<"org.freedesktop.systemd1.Unit"<<"ActiveState";
    msg.setArguments(_args);
    bool sent = connection.callWithCallback(
                msg, this, SLOT(receiveServiceStatus(QDBusMessage)));
    return sent;
}
void MainWindow::receiveServiceStatus(QDBusMessage _msg)
{
    QList<QVariant> args = _msg.arguments();
    if ( args.length()!=1 ) return;
    QVariant arg = _msg.arguments().first();
    QString str = QDBusUtil::argumentToString(arg);
    QStringList l = str.split('"');
    if ( l.length()<3 ) return;
    QString status = l.at(1);
    KNotification::event(
                KNotification::Notification,
                "WvDialer",
                QString("WvDialer is %1.").arg(status));
    if        ( status=="inactive" ) {
        srvStatus = INACTIVE;
    } else if ( status=="active" ) {
        srvStatus = ACTIVE;
    } else if ( status=="failed" ) {
        srvStatus = FAILED;
    } else if ( status=="activating" ) {
        srvStatus = ACTIVATING;
    } else if ( status=="deactivating" ) {
        srvStatus = DEACTIVATING;
    } else {
        srvStatus = INACTIVE;
    };
    serviceStatusChanged();
}
void MainWindow::serviceStatusChanged()
{
    switch ( srvStatus ) {
    case INACTIVE:
    case FAILED:
        if ( startFlag ) {
            trayIcon->setIcon(
                        QIcon::fromTheme("wvdialer_reload",
                                         QIcon(":/wvdialer_reload.png")));
            startWvDialProcess();
        } else {
            trayIcon->setIcon(
                        QIcon::fromTheme("wvdialer_close",
                                         QIcon(":/wvdialer_close.png")));
        };
        trayIcon->stopAction->setEnabled(false);
        trayIcon->startAction->setEnabled(true);
        break;
    case ACTIVE:
        trayIcon->setIcon(
                    QIcon::fromTheme("wvdialer_open",
                                     QIcon(":/wvdialer_open.png")));
        trayIcon->startAction->setEnabled(false);
        trayIcon->stopAction->setEnabled(true);
        startFlag = false;
        break;
    case DEACTIVATING:
    case   ACTIVATING:
        trayIcon->setIcon(
                    QIcon::fromTheme("wvdialer_reload",
                                     QIcon(":/wvdialer_reload.png")));
        trayIcon->stopAction->setEnabled(false);
        trayIcon->startAction->setEnabled(false);
        break;
    default:
        break;
    };
}
