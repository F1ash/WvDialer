#include <QProcess>
#include "wvdialer_helper.h"
#include <signal.h>

WvDialHelper::WvDialHelper(QObject *parent) :
    QObject(parent)
{
}

QString WvDialHelper::get_key_varmap(const QVariantMap &args, const QString& key)
{
    QString value;
    if (args.keys().contains(key)) {
        value = args[key].toString();
    } else {
        value = QString();
    };
    return value;
}

ActionReply WvDialHelper::run(const QVariantMap args)
{
    ActionReply reply;
 
    QString act = get_key_varmap(args, "action");
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

ActionReply WvDialHelper::kill(const QVariantMap args)
{
    ActionReply reply;

    QString act = get_key_varmap(args, "action");
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

KAUTH_HELPER_MAIN("pro.russianfedora.wvdialer", WvDialHelper)
