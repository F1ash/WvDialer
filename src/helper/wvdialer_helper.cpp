#include <QProcess>
#include "wvdialer_helper.h"
#include <signal.h>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>

#define WVDIALER QString("wvdialer")

WvDialerHelper::WvDialerHelper(QObject *parent) :
    QObject(parent)
{
}

QString WvDialerHelper::get_key_varmap(const QVariantMap &args, const QString& key) const
{
    QString value;
    if (args.keys().contains(key)) {
        value = args[key].toString();
    } else {
        value = QString();
    };
    return value;
}

ActionReply WvDialerHelper::run(const QVariantMap args) const
{
    ActionReply reply;
 
    const QString act = get_key_varmap(args, "action");
    if ( act!="run" ) {
        QVariantMap err;
        err["result"] = QString::number(-1);
        reply.setData(err);
        return reply;
    };
 
    qint64 pid;
    bool started = QProcess::startDetached(
                "/usr/bin/wvdial",
                QStringList(),
                "",
                &pid);
 
    QVariantMap retdata;
    retdata["result"] = (started)? "ok":"fail";
    retdata["PID"]    = QString::number(pid);
 
    reply.setData(retdata);
    return reply;
}

ActionReply WvDialerHelper::kill(const QVariantMap args) const
{
    ActionReply reply;

    const QString act = get_key_varmap(args, "action");
    if ( act!="kill" ) {
        QVariantMap err;
        err["result"] = QString::number(-1);
        reply.setData(err);
        return reply;
    };

    QString pid = get_key_varmap(args, "PID");
    bool ok;
    int _pid = pid.toInt(&ok);
    //int code = QProcess::execute(
    //            "/usr/bin/kill",
    //            QStringList()<<"-2"<<pid);
    if ( !ok ) {
        QVariantMap err;
        err["result"] = QString::number(-1);
        reply.setData(err);
        return reply;
    };
    int code = ::kill(_pid, SIGKILL);

    QVariantMap retdata;
    retdata["code"] = QString::number(code);

    reply.setData(retdata);
    return reply;
}

ActionReply WvDialerHelper::create(const QVariantMap args) const
{
    // Unable to find method StartTransientUnit
    // on path /org/freedesktop/systemd1 in interface
    // org.freedesktop.systemd1.Manager
    ActionReply reply;

    const QString act = get_key_varmap(args, "action");
    if ( act!="create" ) {
        QVariantMap err;
        err["result"] = QString::number(-1);
        reply.setData(err);
        return reply;
    };

    QVariantMap retdata;
    // Start new transient service unit: wvdialer.service
    QProcess proc;
    proc.setProgram("/usr/bin/systemd-run");
    proc.setArguments(QStringList()
                      <<"--unit"<<WVDIALER
                      <<"--service-type"<<"simple"
                      <<"/usr/bin/wvdial");
    proc.start(QIODevice::ReadOnly);
    if ( proc.waitForStarted() && proc.waitForFinished() ) {
        retdata["code"]     = QString::number(proc.exitCode());
    } else {
        retdata["code"]     = QString::number(-1);
    };

    reply.setData(retdata);
    return reply;
}

ActionReply WvDialerHelper::start(const QVariantMap args) const
{
    ActionReply reply;

    const QString act = get_key_varmap(args, "action");
    if ( act!="start" ) {
        QVariantMap err;
        err["result"] = QString::number(-1);
        reply.setData(err);
        return reply;
    };

    QVariantMap retdata;
    /*
    QProcess proc;
    proc.setProgram("/usr/bin/systemctl");
    proc.setArguments(QStringList()<<act<<WVDIALER);
    proc.start(QIODevice::ReadOnly);
    if ( proc.waitForStarted() && proc.waitForFinished() ) {
        retdata["code"]     = QString::number(proc.exitCode());
    } else {
        retdata["code"]     = QString::number(-1);
    };
    */

    QDBusMessage msg = QDBusMessage::createMethodCall(
                "org.freedesktop.systemd1",
                "/org/freedesktop/systemd1",
                "org.freedesktop.systemd1.Manager",
                "StartUnit");
    QList<QVariant> _args;
    _args<<QString("%1.service").arg(WVDIALER)<<"fail";
    msg.setArguments(_args);
    QDBusMessage res = QDBusConnection::systemBus()
            .call(msg, QDBus::Block);
    switch (res.type()) {
    case QDBusMessage::ReplyMessage:
        retdata["code"]     = QString::number(0);
        break;
    default:
        retdata["code"]     = QString::number(1);
        break;
    };

    reply.setData(retdata);
    return reply;
}

// Unused method
ActionReply WvDialerHelper::status(const QVariantMap args) const
{
    ActionReply reply;

    const QString act = get_key_varmap(args, "action");
    if ( act!="is-active" ) {
        QVariantMap err;
        err["result"] = QString::number(-1);
        reply.setData(err);
        return reply;
    };

    QVariantMap retdata;
    QByteArray result;
    QProcess proc;
    proc.setProgram("/usr/bin/systemctl");
    proc.setArguments(QStringList()<<act<<WVDIALER);
    proc.start(QIODevice::ReadOnly);
    if ( proc.waitForStarted() && proc.waitForFinished() ) {
        // 'inactive'       -- service not exist
        // 'active'         -- service exist and running
        // 'failed'         -- service exist and stopped
        // 'deactivating'   -- service exist and deactivating
        result = proc.readAll();
        retdata["result"]   = QString::fromUtf8(
                    result.data(), result.size()-1);
        retdata["code"]     = QString::number(proc.exitCode());
    } else {
        retdata["result"]   = "failed";
        retdata["code"]     = QString::number(-1);
    };

    reply.setData(retdata);
    return reply;
}

ActionReply WvDialerHelper::stop(const QVariantMap args) const
{
    ActionReply reply;

    const QString act = get_key_varmap(args, "action");
    if ( act!="stop" ) {
        QVariantMap err;
        err["result"] = QString::number(-1);
        reply.setData(err);
        return reply;
    };

    QVariantMap retdata;
    /*
    QProcess proc;
    proc.setProgram("/usr/bin/systemctl");
    proc.setArguments(QStringList()<<act<<WVDIALER);
    proc.start(QIODevice::ReadOnly);
    if ( proc.waitForStarted() && proc.waitForFinished() ) {
        retdata["code"]     = QString::number(proc.exitCode());
    } else {
        retdata["code"]     = QString::number(-1);
    };
    */

    QDBusMessage msg = QDBusMessage::createMethodCall(
                "org.freedesktop.systemd1",
                "/org/freedesktop/systemd1",
                "org.freedesktop.systemd1.Manager",
                "StopUnit");
    QList<QVariant> _args;
    _args<<QString("%1.service").arg(WVDIALER)<<"fail";
    msg.setArguments(_args);
    QDBusMessage res = QDBusConnection::systemBus()
            .call(msg, QDBus::Block);
    switch (res.type()) {
    case QDBusMessage::ReplyMessage:
        retdata["code"]     = QString::number(0);
        break;
    default:
        retdata["code"]     = QString::number(1);
        break;
    };

    reply.setData(retdata);
    return reply;
}

KAUTH_HELPER_MAIN("pro.russianfedora.wvdialer", WvDialerHelper)
