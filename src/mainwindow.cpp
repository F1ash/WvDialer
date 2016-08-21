#include "mainwindow.h"
#include <QDir>
#define DEV_DIR QString("/dev")
#define PROC_DIR QString("/proc")

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setSizePolicy(
                QSizePolicy(
                    QSizePolicy::MinimumExpanding,
                    QSizePolicy::MinimumExpanding));
    setWindowTitle("WvDialer");
    QIcon::setThemeName("WvDialer");
    setWindowIcon(QIcon::fromTheme("wvdialer", QIcon(":/wvdialer.png")));
    deviceExist = false;
    reloadFlag = false;
    PID = -1;
    timerID = 0;
    baseLayout = nullptr;
    baseWdg = nullptr;
    scrolled = nullptr;
    initTrayIcon();
    watcher = new QFileSystemWatcher(this);
    watcher->addPath(DEV_DIR);
    connect(watcher, SIGNAL(directoryChanged(QString)),
            this, SLOT(directoryChanged(QString)));
    connect(this, SIGNAL(killed()),
            this, SLOT(startWvDialProcess()));
    restoreGeometry(settings.value("Geometry").toByteArray());
    hide();
}

void MainWindow::initTrayIcon()
{
    trayIcon = new TrayIcon(this);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
    connect(trayIcon->hideAction, SIGNAL(triggered(bool)),
            this, SLOT(changeVisibility()));
    connect(trayIcon->reloadAction, SIGNAL(triggered(bool)),
            this, SLOT(reloadConnection()));
    connect(trayIcon->killAction, SIGNAL(triggered(bool)),
            this, SLOT(killConnection()));
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
bool MainWindow::getDirExistanceState(const QString _dir1, const QString _dir2) const
{
    QString _dirPath = QString("%1%2%3")
            .arg(_dir1).arg(QDir::separator()).arg(_dir2);
    QDir d;
    d.setPath(_dirPath);
    return d.exists();
}
void MainWindow::closeEvent(QCloseEvent *ev)
{
    if ( ev->type()==QEvent::Close ) {
        killConnection();
        settings.setValue("Geometry", saveGeometry());
        trayIcon->hide();
        ev->accept();
    };
}
void MainWindow::timerEvent(QTimerEvent *ev)
{
    ev->accept();
    if ( ev->timerId()==timerID ) {
        if ( PID>1 ) {
            if ( !getDirExistanceState(PROC_DIR, QString::number(PID)) ) {
                KNotification::event(
                            KNotification::Notification,
                            "WvDialer",
                            QString("Connection process closed (PID: %1).")
                            .arg(PID),
                            this);
                if ( !reloadFlag )
                    trayIcon->setIcon(
                                QIcon::fromTheme("wvdialer_close",
                                                 QIcon(":/wvdialer_close.png")));
                PID = -1;
                emit killed();
            };
        } else {
            killTimer(timerID);
            timerID = 0;
        };
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
            trayIcon->showMessage("WvDialer", msg);
            reloadFlag = true;
            if (deviceExist) {
                startWvDialProcess();
            } else {
                killConnection();
            };
        };
    };
}
void MainWindow::reloadConnection()
{
    deviceExist = getFileExistanceState(DEV_DIR, "ttyUSB0");
    if ( deviceExist ) {
        trayIcon->setIcon(
                    QIcon::fromTheme("wvdialer_reload",
                                     QIcon(":/wvdialer_reload.png")));
        reloadFlag = true;
        if ( PID>1 ) {
            killConnection();
        } else {
            startWvDialProcess();
        };
    } else {
        KNotification::event(
                    KNotification::Notification,
                    "WvDialer",
                    QString("Device %1 is not connected. Reconnect him.")
                    .arg("/dev/ttyUSB0"),
                    this);
        trayIcon->setIcon(
                    QIcon::fromTheme("wvdialer_close",
                                     QIcon(":/wvdialer_close.png")));
    };
}
void MainWindow::killConnection()
{
    // if PID>1, then kill wvdial session
    if ( PID>1 ) {
        if ( getDirExistanceState(PROC_DIR, QString::number(PID)) ) {
            QFile f;
            f.setFileName(
                        QString("%1%2%3%4%5")
                        .arg(PROC_DIR)
                        .arg(QDir::separator())
                        .arg(QString::number(PID))
                        .arg(QDir::separator())
                        .arg("comm"));
            if ( !f.exists() ) {
                KNotification::event(
                            KNotification::Notification,
                            "WvDialer",
                            "PID command file not exist.",
                            this);
                trayIcon->setIcon(
                            QIcon::fromTheme("wvdialer_close",
                                             QIcon(":/wvdialer_close.png")));
                return;
            };
            if ( f.open(QIODevice::ReadOnly) ) {
                QByteArray text = f.readAll();
                f.close();
                if ( !QString::fromUtf8(text.data()).contains("wvdial") ) {
                    KNotification::event(
                                KNotification::Notification,
                                "WvDialer",
                                "Current PID process not wvdial. Kill not possible.",
                                this);
                    trayIcon->setIcon(
                                QIcon::fromTheme("wvdialer_close",
                                                 QIcon(":/wvdialer_close.png")));
                    return;
                };
            } else {
                KNotification::event(
                            KNotification::Notification,
                            "WvDialer",
                            "Can't open PID command file.",
                            this);
                trayIcon->setIcon(
                            QIcon::fromTheme("wvdialer_close",
                                             QIcon(":/wvdialer_close.png")));
                return;
            };
        } else {
            KNotification::event(
                        KNotification::Notification,
                        "WvDialer",
                        "PID process not exist in system. Kill not possible.",
                        this);
            trayIcon->setIcon(
                        QIcon::fromTheme("wvdialer_close",
                                         QIcon(":/wvdialer_close.png")));
            return;
        };
        QVariantMap args;
        args["action"] = "kill";
        args["PID"] = QString::number(PID);
        Action act("pro.russianfedora.wvdialer.kill");
        act.setHelperId("pro.russianfedora.wvdialer");
        act.setArguments(args);
        ExecuteJob *job = act.execute();
        job->setAutoDelete(true);
        if (job->exec()) {
            int exitCode = job->data().value("code").toInt();
            KNotification::event(
                        KNotification::Notification,
                        "WvDialer",
                        QString("Wvdial session exit code: %1")
                        .arg(exitCode),
                        this);
        } else {
            KNotification::event(
                        KNotification::Notification,
                        "WvDialer",
                        QString("ERROR: %1\n%2")
                        .arg(job->error()).arg(job->errorText()),
                        this);
        };
    } else {
        KNotification::event(
                    KNotification::Notification,
                    "WvDialer",
                    "Connection not exist",
                    this);
    };
    if ( !reloadFlag )
        trayIcon->setIcon(
                    QIcon::fromTheme("wvdialer_close",
                                     QIcon(":/wvdialer_close.png")));
}
void MainWindow::startWvDialProcess()
{
    if ( !reloadFlag ) return;
    // if device was connected, then run wvdial_helper
    if ( deviceExist ) {
        trayIcon->setIcon(
                    QIcon::fromTheme("wvdialer_reload",
                                     QIcon(":/wvdialer_reload.png")));
        QVariantMap args;
        args["action"] = "run";
        Action act("pro.russianfedora.wvdialer.run");
        act.setHelperId("pro.russianfedora.wvdialer");
        act.setArguments(args);
        ExecuteJob *job = act.execute();
        job->setAutoDelete(true);
        if (job->exec()) {
            QString state = job->data().value("result").toString();
            PID = job->data().value("PID").toInt();
            KNotification::event(
                        KNotification::Notification,
                        "WvDialer",
                        QString("Device is %1 (PID: %2).")
                        .arg(state).arg(PID),
                        this);
            trayIcon->setIcon(
                        QIcon::fromTheme("wvdialer_open",
                                         QIcon(":/wvdialer_open.png")));
            if ( timerID==0 ) timerID = startTimer(1000);
        } else {
            KNotification::event(
                        KNotification::Notification,
                        "WvDialer",
                        QString("ERROR: %1\n%2")
                        .arg(job->error()).arg(job->errorText()),
                        this);
            trayIcon->setIcon(
                        QIcon::fromTheme("wvdialer_close",
                                         QIcon(":/wvdialer_close.png")));
        };
    };
    reloadFlag = false;
}
