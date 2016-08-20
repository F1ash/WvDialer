#include "mainwindow.h"

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
    baseLayout = nullptr;
    baseWdg = nullptr;
    scrolled = nullptr;
    initTrayIcon();
    watcher = new QFileSystemWatcher(this);
    watcher->addPath("/dev");
    connect(watcher, SIGNAL(directoryChanged(QString)),
            this, SLOT(directoryChanged(QString)));
    connect(this, SIGNAL(killed()),
            this, SLOT(startWvDialProcess()));
    restoreGeometry(settings.value("Geometry").toByteArray());
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
        trayIcon->hideAction->setIcon (QIcon::fromTheme("up"));
    } else {
        this->show();
        trayIcon->hideAction->setText (QString("Down"));
        trayIcon->hideAction->setIcon (QIcon::fromTheme("down"));
    };
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason r)
{
    if (r==QSystemTrayIcon::Trigger) changeVisibility();
}

bool MainWindow::getDeviceState() const
{
    // WARNING: used ttyUSB0, because
    // in wvdial.conf used this device name
    QFile f;
    f.setFileName("/dev/ttyUSB0");
    return f.exists();
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
void MainWindow::directoryChanged(QString dir)
{
    Q_UNUSED(dir);
    bool newDeviceState = getDeviceState();
    if ( deviceExist!=newDeviceState ) {
        deviceExist = newDeviceState;
        QString msg = QString("Device in %1 is %2connected.")
                .arg("/dev/ttyUSB0")
                .arg((deviceExist)? "":"dis");
        trayIcon->showMessage("WvDialer", msg);
        reloadFlag = true;
        startWvDialProcess();
    };
}
void MainWindow::reloadConnection()
{
    deviceExist = getDeviceState();
    if ( deviceExist ) {
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
    };
}
void MainWindow::killConnection()
{
    // if PID>1, then kill wvdial session
    if ( PID>1 ) {
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
            PID = -1;
            emit killed();
        } else {
            KNotification::event(
                        KNotification::Notification,
                        "WvDialer",
                        QString("ERROR: %1\n%2")
                        .arg(job->error()).arg(job->errorText()),
                        this);
        };
    };
}
void MainWindow::startWvDialProcess()
{
    if ( !reloadFlag ) return;
    // if device was connected, then run wvdial_helper
    if ( deviceExist ) {
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
        } else {
            KNotification::event(
                        KNotification::Notification,
                        "WvDialer",
                        QString("ERROR: %1\n%2")
                        .arg(job->error()).arg(job->errorText()),
                        this);
            PID = -1;
        };
    };
    reloadFlag = false;
}
