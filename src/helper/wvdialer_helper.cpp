#include <QProcess>
#include "wvdialer_helper.h"

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

ActionReply WvDialHelper::run(QVariantMap args)
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

ActionReply WvDialHelper::kill(QVariantMap args)
{
    ActionReply reply;

    QString act = get_key_varmap(args, "action");
    if ( act!="kill" ) {
        QVariantMap err;
        err["result"] = QString::number(-1);
        reply.setData(err);
        return reply;
    };

    QString pid = get_key_varmap(args, "PID");;
    int code = QProcess::execute(
                "/usr/bin/kill",
                QStringList()<<"-2"<<pid);

    QVariantMap retdata;
    retdata["code"] = QString::number(code);

    reply.setData(retdata);
    return reply;
}

KAUTH_HELPER_MAIN("pro.russianfedora.wvdialer", WvDialHelper)
