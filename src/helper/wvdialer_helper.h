#include <kauth.h>
using namespace KAuth;

class WvDialHelper : public QObject
{
    Q_OBJECT
public:
    explicit WvDialHelper(QObject *parent = nullptr);

public slots:
    ActionReply     run(const QVariantMap args);
    ActionReply     kill(const QVariantMap args);

private:
    QString         get_key_varmap(const QVariantMap &args, const QString& key);
};
