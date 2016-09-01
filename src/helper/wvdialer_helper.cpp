//#include <QProcess>
#include "wvdialer_helper.h"
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <private/qdbusmetatype_p.h>
#include <private/qdbusutil_p.h>

#define WVDIALER QString("WvDialer")

struct PrimitivePair
{
    QString         name;
    QVariant        value;
};
Q_DECLARE_METATYPE(PrimitivePair)
typedef QList<PrimitivePair>    SrvParameters;
Q_DECLARE_METATYPE(SrvParameters)

struct ComplexPair
{
    QString         name;
    SrvParameters   value;
};
Q_DECLARE_METATYPE(ComplexPair)
typedef QList<ComplexPair>      AuxParameters;
Q_DECLARE_METATYPE(AuxParameters)

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

QDBusArgument &operator<<(QDBusArgument &argument, const PrimitivePair &pair)
{
    argument.beginStructure();
    argument << pair.name << QDBusVariant(pair.value);
    argument.endStructure();
    return argument;
}
const QDBusArgument &operator>>(const QDBusArgument &argument, PrimitivePair &pair)
{
    argument.beginStructure();
    argument >> pair.name >> pair.value;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const ComplexPair &pair)
{
    argument.beginStructure();
    argument << pair.name << pair.value;
    argument.endStructure();
    return argument;
}
const QDBusArgument &operator>>(const QDBusArgument &argument, ComplexPair &pair)
{
    argument.beginStructure();
    argument >> pair.name >> pair.value;
    argument.endStructure();
    return argument;
}

QVariantList &operator<<(QVariantList &list, const QDBusArgument &argument)
{
    argument.beginArray();
    while (!argument.atEnd()) {
        QDBusVariant item;
        argument >> item;
        list.append(item.variant());
    }
    argument.endArray();

    return list;
}

// Not implemented;
// used persistent wvdialer.service systemd unit
ActionReply WvDialerHelper::create(const QVariantMap args) const
{
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

    /*
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
    */

    qDBusRegisterMetaType<PrimitivePair>();
    qDBusRegisterMetaType<SrvParameters>();
    qDBusRegisterMetaType<ComplexPair>();
    qDBusRegisterMetaType<AuxParameters>();

    QDBusMessage msg = QDBusMessage::createMethodCall(
                "org.freedesktop.systemd1",
                "/org/freedesktop/systemd1",
                "org.freedesktop.systemd1.Manager",
                "StartTransientUnit");
    // Expecting 'ssa(sv)a(sa(sv))'
    QVariantList  _args;

    SrvParameters _props;
    PrimitivePair srvType, execStart, execStop;
    srvType.name    = "Type";
    srvType.value   = "simple";
    execStart.name  = "ExecStart";
    execStart.value = QLatin1String("/usr/bin/wvdial");
    execStop.name   = "ExecStop";
    execStop.value  = QLatin1String("/bin/kill -INT ${MAINPID}");
    _props.append(srvType);
    _props.append(execStart);
    _props.append(execStop);

    AuxParameters _aux;
    // aux is currently unused and should be passed as empty array.
    // ComplexPair   srvAuxData;
    //_aux.append(srvAuxData);

    QDBusArgument PROPS, AUX;
    PROPS<<_props;
    AUX<<_aux;

    _args
        << QString("%1.service").arg(WVDIALER)
        << "replace";
    _args<< PROPS;
    _args<< AUX;
    msg.setArguments(_args);
    QDBusMessage res = QDBusConnection::systemBus()
            .call(msg, QDBus::Block);
    QString str;
    foreach (QVariant arg, res.arguments()) {
        str.append(QDBusUtil::argumentToString(arg));
        str.append("\n");
    };
    retdata["msg"]          = str;
    switch (res.type()) {
    case QDBusMessage::ReplyMessage:
        retdata["code"]     = QString::number(0);
        break;
    case QDBusMessage::ErrorMessage:
        retdata["code"]     = QString::number(1);
        retdata["err"]      = res.errorMessage();
        break;
    default:
        retdata["code"]     = QString::number(1);
        break;
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
    QString str;
    foreach (QVariant arg, res.arguments()) {
        str.append(QDBusUtil::argumentToString(arg));
        str.append("\n");
    };
    retdata["msg"]          = str;
    switch (res.type()) {
    case QDBusMessage::ReplyMessage:
        retdata["code"]     = QString::number(0);
        break;
    case QDBusMessage::ErrorMessage:
        retdata["code"]     = QString::number(1);
        retdata["err"]      = res.errorMessage();
        break;
    default:
        retdata["code"]     = QString::number(1);
        break;
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
    QString str;
    foreach (QVariant arg, res.arguments()) {
        str.append(QDBusUtil::argumentToString(arg));
        str.append("\n");
    };
    retdata["msg"]          = str;
    switch (res.type()) {
    case QDBusMessage::ReplyMessage:
        retdata["code"]     = QString::number(0);
        break;
    case QDBusMessage::ErrorMessage:
        retdata["code"]     = QString::number(1);
        retdata["err"]      = res.errorMessage();
        break;
    default:
        retdata["code"]     = QString::number(1);
        break;
    };

    reply.setData(retdata);
    return reply;
}

KAUTH_HELPER_MAIN("pro.russianfedora.wvdialer", WvDialerHelper)
