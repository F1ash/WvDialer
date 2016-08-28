#include "mainwindow.h"
#include <QDir>
#include <QTimer>
#include <private/qdbusutil_p.h>

#define DEV_DIR  QString("/dev")
#define PROC_DIR QString("/proc")

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    objectPathRegExp(QLatin1String("\"ActiveState\" = \\[Variant(QString): \"*\"\\]"))
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
    connected = false;;
    PID = -1;
    timerID = 0;
    srvStatus = INACTIVE;
    wvdialerUnit = nullptr;
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
void MainWindow::createWvDialerAccessor()
{
    if ( connected ) return;
    /*
    wvdialerUnit = new QDBusInterface(
                "org.freedesktop.systemd1",
                "/org/freedesktop/systemd1/unit/wvdialer_2eservice",
                "org.freedesktop.DBus.Properties",
                QDBusConnection::systemBus(),
                this);
    if ( wvdialerUnit->isValid() ) {
        connect(wvdialerUnit,
                "PropertiesChanged",
                this, SLOT(wvdialerUnitStatusReceiver(
                               const QString&, const QVariantMap&, const QStringList&)));
        qDebug()<<"dbus interface created and connected";
    } else {
        QTimer::singleShot(1000, this, SLOT(createWvDialerAccessor()));
    };
    */
    connected = QDBusConnection::systemBus()
            .connect(
                "org.freedesktop.systemd1",
                "/org/freedesktop/systemd1/unit/wvdialer_2eservice",
                "org.freedesktop.DBus.Properties",
                "PropertiesChanged",
                this,
                SLOT(wvdialerUnitStatusReceiver(QDBusMessage)));
    if ( !connected ) {
        KNotification::event(
                    KNotification::Notification,
                    "WvDialer",
                    "Not connected to org.freedesktop.systemd1",
                    this);
        QTimer::singleShot(1000, this, SLOT(createWvDialerAccessor()));
    } else {
        KNotification::event(
                    KNotification::Notification,
                    "WvDialer",
                    "Connected to org.freedesktop.systemd1",
                    this);
    };
}
void MainWindow::wvdialerUnitStatusReceiver(QDBusMessage message)
{
    QList<QVariant> args = message.arguments();
    if ( args.first().toString()!=
         "org.freedesktop.systemd1.Unit" ) return;
    QString out = QLatin1String("Received ");
    switch (message.type()) {
    case QDBusMessage::SignalMessage:
        out += QLatin1String("signal ");
        break;
    case QDBusMessage::ErrorMessage:
        out += QLatin1String("error message ");
        break;
    case QDBusMessage::ReplyMessage:
        out += QLatin1String("reply ");
        break;
    default:
        out += QLatin1String("message ");
        break;
    };
    out += QLatin1String("from ");
    out += message.service();
    if (!message.path().isEmpty())
        out += QLatin1String(", path ") +
                message.path();
    if (!message.interface().isEmpty())
        out += QLatin1String(", interface <i>") +
                message.interface() + QLatin1String("</i>");
    if (!message.member().isEmpty())
        out += QLatin1String(", member ") +
                message.member();
    out += QLatin1String("<br>");
    if (args.isEmpty()) {
        out += QLatin1String("<br><br>(no arguments)");
    } else {
        foreach (QVariant arg, args) {
            QString str = QDBusUtil::argumentToString(arg).toHtmlEscaped();
            // turn object paths into clickable links
            str.replace(
                        objectPathRegExp,
                        QLatin1String(
                            "[ObjectPath: <a href=\"qdbus://bus\\1\">\\1</a>]"));
            // convert new lines from command to proper HTML line breaks
            str.replace(QStringLiteral("\n"), QStringLiteral("<br/>"));
            out += str;
            out += QLatin1String(", ");
        }
        out.chop(2);
        /*
        out += QLatin1String("<br><br>Arguments: ");
        foreach (QVariant arg, args) {
            QMap<QString, QVariant> m;
            QList<QVariant> l;
            switch ( arg.type() ) {
            case QVariant::String :
                out += arg.toString() + " ";
                break;
            case QVariant::StringList :
                foreach (QString s, arg.toStringList()) {
                    out += s + " ";
                };
                break;
            case QVariant::Map :
                m = arg.toMap();
                foreach (QString key, m.keys()) {
                    out += m.value(key).toString() + " ";
                };
                break;
            case QVariant::Bool :
                out += arg.toString() + " ";
                break;
            case QVariant::ULongLong :
                out += arg.toString() + " ";
                break;
            case QVariant::UInt :
                out += arg.toString() + " ";
                break;
            case QVariant::Int :
                out += arg.toString() + " ";
                break;
            case QVariant::List :
                l = arg.toList();
                foreach (QVariant v, l) {
                    out += v.toString() + " ";
                };
                break;
            case QVariant::ByteArray :
                out += arg.toByteArray().data();
                break;
            default:
                break;
            };
        };
        */
    };
    KNotification::event(
                KNotification::Notification,
                "WvDialer",
                message.signature(),
                this);
}
void MainWindow::closeEvent(QCloseEvent *ev)
{
    if ( ev->type()==QEvent::Close ) {
        killTimer(timerID);
        timerID = 0;
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
        /*
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
        */
        srvStatus = getServiceStatus();

        switch ( srvStatus ) {
        case FAILED:
            if ( !reloadFlag )
                trayIcon->setIcon(
                            QIcon::fromTheme("wvdialer_close",
                                             QIcon(":/wvdialer_close.png")));
            break;
        case ACTIVE:
            trayIcon->setIcon(
                        QIcon::fromTheme("wvdialer_open",
                                         QIcon(":/wvdialer_open.png")));
            break;
        case DEACTIVATING:
            trayIcon->setIcon(
                        QIcon::fromTheme("wvdialer_reload",
                                         QIcon(":/wvdialer_reload.png")));
            break;
        default:
            break;
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
            reloadFlag = deviceExist;
            stopWvDialProcess();
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
        //trayIcon->setActionState(false);
        reloadFlag = true;
        //if ( PID>1 ) {
        srvStatus = getServiceStatus();
        if ( srvStatus==ACTIVE ) {
            stopWvDialProcess();
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
    reloadFlag = false;
    stopWvDialProcess();
}
void MainWindow::stopWvDialProcess()
{
    // if PID>1, then kill wvdial session
    /*
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
    */

    //trayIcon->setActionState(false);
    QVariantMap args;
    args["action"] = "stop";
    Action act("pro.russianfedora.wvdialer.stop");
    act.setHelperId("pro.russianfedora.wvdialer");
    act.setArguments(args);
    ExecuteJob *job = act.execute();
    job->setAutoDelete(true);
    if (job->exec()) {
        QString code = job->data().value("code").toString();
        KNotification::event(
                    KNotification::Notification,
                    "WvDialer",
                    QString("Wvdial session closed with exit code: %1")
                    .arg(code),
                    this);
        if ( !reloadFlag ) {
            trayIcon->setIcon(
                        QIcon::fromTheme("wvdialer_close",
                                         QIcon(":/wvdialer_close.png")));
        };
    } else {
        KNotification::event(
                    KNotification::Notification,
                    "WvDialer",
                    QString("ERROR: %1\n%2")
                    .arg(job->error()).arg(job->errorText()),
                    this);
    };
    //trayIcon->setActionState(true);
    emit killed();
}
void MainWindow::startWvDialProcess()
{
    if ( !reloadFlag ) {
        //trayIcon->setActionState(true);
        return;
    };
    // if device was connected, then run wvdial_helper
    if ( deviceExist ) {
        createWvDialerAccessor();
        trayIcon->setIcon(
                    QIcon::fromTheme("wvdialer_reload",
                                     QIcon(":/wvdialer_reload.png")));
        QVariantMap args;
        Action act;
        switch (srvStatus) {
        case INACTIVE:
            args["action"] = "create";
            act.setName("pro.russianfedora.wvdialer.create");
            break;
        case FAILED:
            args["action"] = "start";
            act.setName("pro.russianfedora.wvdialer.start");
            break;
        default:
            QTimer::singleShot(1000, this, SLOT(reloadConnection()));
            return;
        };
        //args["action"] = "run";
        //Action act("pro.russianfedora.wvdialer.run");
        act.setHelperId("pro.russianfedora.wvdialer");
        act.setArguments(args);
        ExecuteJob *job = act.execute();
        job->setAutoDelete(true);
        if (job->exec()) {
            /*
            QString state = job->data().value("result").toString();
            PID = job->data().value("PID").toInt();
            KNotification::event(
                        KNotification::Notification,
                        "WvDialer",
                        QString("Device is %1 (PID: %2).")
                        .arg(state).arg(PID),
                        this);
             */
            QString code = job->data().value("code").toString();
            KNotification::event(
                        KNotification::Notification,
                        "WvDialer",
                        QString("Wvdial session started with exit code: %1.")
                        .arg(code),
                        this);
            trayIcon->setIcon(
                        QIcon::fromTheme("wvdialer_open",
                                         QIcon(":/wvdialer_open.png")));
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
    if ( timerID==0 ) timerID = startTimer(1000);
    reloadFlag = false;
    //trayIcon->setActionState(true);
}
SRV_STATUS MainWindow::getServiceStatus()
{
    SRV_STATUS res = DEACTIVATING;
    QVariantMap args;
    args["action"] = "is-active";
    Action act("pro.russianfedora.wvdialer.status");
    act.setHelperId("pro.russianfedora.wvdialer");
    act.setArguments(args);
    ExecuteJob *job = act.execute();
    job->setAutoDelete(true);
    if (job->exec()) {
        QString status = job->data().value("result").toString();
        if ( status=="inactive" ) {
            res = INACTIVE;
        } else if ( status=="active" ) {
            res = ACTIVE;
        } else if ( status=="failed" ) {
            res = FAILED;
        } else if ( status=="deactivating" ) {
            res = DEACTIVATING;
        };
        //KNotification::event(
        //            KNotification::Notification,
        //            "WvDialer",
        //            QString("Wvdial session status: %1 (%2)")
        //            .arg(status).arg(res),
        //            this);
    } else {
        //KNotification::event(
        //            KNotification::Notification,
        //            "WvDialer",
        //            QString("ERROR: %1\n%2")
        //            .arg(job->error()).arg(job->errorText()),
        //            this);
    };
    return res;
}
