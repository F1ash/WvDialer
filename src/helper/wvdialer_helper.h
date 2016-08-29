#include <kauth.h>
using namespace KAuth;

class WvDialerHelper : public QObject
{
    Q_OBJECT
public:
    explicit WvDialerHelper(QObject *parent = nullptr);

public slots:
    ActionReply     run(const QVariantMap args) const;
    ActionReply     kill(const QVariantMap args) const;

    ActionReply     create(const QVariantMap args) const;
    ActionReply     start(const QVariantMap args) const;
    ActionReply     status(const QVariantMap args) const;
    ActionReply     stop(const QVariantMap args) const;

private:
    QString         get_key_varmap(const QVariantMap &args, const QString& key) const;
};
