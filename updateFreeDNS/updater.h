#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QUrl>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QNetworkReply>
#include <QMutex>
#include "filedownloader.h"
#include "output.h"

class Updater : public QObject
{
    Q_OBJECT
    Q_ENUMS(Protocol)
public:
    enum class Protocol {IPv4, IPv6, None = -1};
    //Q_ENUM(Protocol)

    explicit Updater(const QString name, const bool enabled, const bool verbose, const QString protocol, const QString updateURL, const QString addressSource, const QString domain);

signals:

public slots:
    void update();

    void onCurrentAddressDownloaded(QNetworkReply::NetworkError error);
    void onDNSUpdated(QNetworkReply::NetworkError error);

private:
    QString name;
    bool enabled;
    bool verbose;
    QString updateURL;
    QUrl addressSource;
    QString domain;
    QAbstractSocket::NetworkLayerProtocol protocol;

    Output *out;

    bool valid{true};
    QMutex updateInProgress;

    FileDownloader *download{nullptr};

    QHostAddress currentAddress;
    QHostAddress lastAddress;

    QList<QHostAddress> getDNSAddresses();
};

#endif // UPDATER_H
