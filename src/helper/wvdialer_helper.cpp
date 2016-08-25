#include <QProcess>
#include "wvdialer_helper.h"
#include <signal.h>

#define WVDIALER QString("wvdialer")

WvDialHelper::WvDialHelper(QObject *parent) :
    QObject(parent)
{
}

QString WvDialHelper::get_key_varmap(const QVariantMap &args, const QString& key) const
{
    QString value;
    if (args.keys().contains(key)) {
        value = args[key].toString();
    } else {
        value = QString();
    };
    return value;
}

ActionReply WvDialHelper::run(const QVariantMap args) const
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

ActionReply WvDialHelper::kill(const QVariantMap args) const
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

ActionReply WvDialHelper::create(const QVariantMap args) const
{
    ActionReply reply;

    const QString act = get_key_varmap(args, "action");
    if ( act!="create" ) {
        QVariantMap err;
        err["result"] = QString::number(-1);
        reply.setData(err);
        return reply;
    };

    // Start new transient service unit: wvdialer.service
    QProcess proc;
    proc.setProgram("/usr/bin/systemd-run");
    proc.setArguments(QStringList()
                      <<"--unit"<<WVDIALER
                      <<"--service-type"<<"simple"
                      <<"/usr/bin/wvdial");
    proc.start();
    proc.waitForFinished();

    QVariantMap retdata;
    retdata["code"]     = QString::number(proc.exitCode());

    reply.setData(retdata);
    return reply;
}

ActionReply WvDialHelper::start(const QVariantMap args) const
{
    ActionReply reply;

    const QString act = get_key_varmap(args, "action");
    if ( act!="start" ) {
        QVariantMap err;
        err["result"] = QString::number(-1);
        reply.setData(err);
        return reply;
    };

    QProcess proc;
    proc.setProgram("/usr/bin/systemctl");
    proc.setArguments(QStringList()<<act<<WVDIALER);
    proc.start();
    proc.waitForFinished();

    QVariantMap retdata;
    retdata["code"]     = QString::number(proc.exitCode());

    reply.setData(retdata);
    return reply;
}

ActionReply WvDialHelper::status(const QVariantMap args) const
{
    ActionReply reply;

    const QString act = get_key_varmap(args, "action");
    if ( act!="is-active" ) {
        QVariantMap err;
        err["result"] = QString::number(-1);
        reply.setData(err);
        return reply;
    };

    QProcess proc;
    proc.setProgram("/usr/bin/systemctl");
    proc.setArguments(QStringList()<<act<<WVDIALER);
    proc.start();
    proc.waitForReadyRead();
    // 'inactive'       -- service not exist
    // 'active'         -- service exist and running
    // 'failed'         -- service exist and stopped
    // 'deactivating'   -- service exist and deactivating
    QByteArray result = proc.readAll();
    proc.waitForFinished();

    QVariantMap retdata;
    retdata["code"]     = QString::number(proc.exitCode());
    retdata["result"]   = QString::fromUtf8(
                result.data(), result.size()-1);

    reply.setData(retdata);
    return reply;
}

ActionReply WvDialHelper::stop(const QVariantMap args) const
{
    ActionReply reply;

    const QString act = get_key_varmap(args, "action");
    if ( act!="stop" ) {
        QVariantMap err;
        err["result"] = QString::number(-1);
        reply.setData(err);
        return reply;
    };

    QProcess proc;
    proc.setProgram("/usr/bin/systemctl");
    proc.setArguments(QStringList()<<act<<WVDIALER);
    proc.start();
    proc.waitForFinished();

    QVariantMap retdata;
    retdata["code"]     = QString::number(proc.exitCode());

    reply.setData(retdata);
    return reply;
}

KAUTH_HELPER_MAIN("pro.russianfedora.wvdialer", WvDialHelper)
