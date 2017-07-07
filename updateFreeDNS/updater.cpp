#include "updater.h"
#include <QHostInfo>
#include <QMetaEnum>
#include "standardstreams.h"
#include "filedownloader.h"

Updater::Updater(const QString name, const bool enabled, const QString protocol, const QString updateURL, const QString addressSource, const QString domain) :
    name(name),
    enabled(enabled),
    updateURL(updateURL),
    addressSource(addressSource),
    domain(domain)
{
    this->prefix = tr("Updater %1: ").arg(this->name);
    bool ok = true;
    const char * cProtocol = reinterpret_cast<const char *>(protocol.toLatin1().data()); // This might fuck the encoding but should work for ASCII

    // Prepare QMetaEnum to map the text to the actual enum
    const QMetaObject &mo = Updater::staticMetaObject;
    const int index = mo.indexOfEnumerator("Protocol");
    QMetaEnum metaEnum = mo.enumerator(index);

    // Map protocol text to enum
    Protocol settingsProtocol = static_cast<Protocol>(metaEnum.keyToValue(cProtocol, &ok));
    if ((!ok) || (settingsProtocol == Protocol::None))
    {
        settingsProtocol = Protocol::None;
        this->valid = false;
        StandardStreams::lock();
        StandardStreams::err << this->prefix << tr("Invalid Protocol!") << endl;
        StandardStreams::unlock();
    }
    switch (settingsProtocol)
    {
    case Protocol::IPv4:
        this->protocol = QAbstractSocket::IPv4Protocol;
        break;
    case Protocol::IPv6:
        this->protocol = QAbstractSocket::IPv6Protocol;
        break;
    default:
        this->protocol = QAbstractSocket::UnknownNetworkLayerProtocol;
    }

    // Basic check for URL
    if (this->updateURL.count() < 1)
    {
        this->valid = false;
        StandardStreams::lock();
        StandardStreams::err << this->prefix << tr("Invalid UpdateURL!") << endl;
        StandardStreams::unlock();
    }

    // Address Source
    this->addressSource = QUrl(addressSource, QUrl::StrictMode);
    if (!this->addressSource.isValid())
    {
        this->valid = false;
        StandardStreams::lock();
        StandardStreams::err << this->prefix << tr("Invalid Source! %1").arg(this->addressSource.errorString()) << endl;
        StandardStreams::unlock();
    }

    // Domain
    if (this->domain.count() < 1)
    {
        this->valid = false;
        StandardStreams::lock();
        StandardStreams::err << this->prefix << tr("Invalid Domain!") << endl;
        StandardStreams::unlock();
    }

    StandardStreams::lock();
    if (this->valid)
    {
        StandardStreams::out << this->prefix << tr("Successfully initialized!") << endl;
    }
    else
    {
        StandardStreams::out << this->prefix << tr("Marked as invalid.") << endl;
    }
    StandardStreams::unlock();
}

void Updater::update()
{
    if (this->valid)
    {
        if (this->updateInProgress.tryLock())
        {
            this->download = new FileDownloader(this->addressSource);
            connect(this->download, &FileDownloader::downloaded, this, &Updater::onCurrentAddressDownloaded);
        }
        else
        {
            StandardStreams::lock();
            StandardStreams::err << this->prefix << tr("Update already in progress, skipping. Consider checking the interval settings.") << endl;
            StandardStreams::unlock();
        }
    }
}

void Updater::onCurrentAddressDownloaded(QNetworkReply::NetworkError error)
{
    disconnect(this->download, &FileDownloader::downloaded, this, &Updater::onCurrentAddressDownloaded);
    this->download->deleteLater();

    if (error == QNetworkReply::NoError)
    {
        int matchesFound = 0;
        const QList<QHostAddress> dnsAddresses = this->getDNSAddresses();
        this->currentAddress = QHostAddress(QString::fromLatin1(this->download->getDownloadedData()));

        // Count the matches between current address and addresses found in DNS
        for (const QHostAddress addr : dnsAddresses)
        {
            if (addr == this->currentAddress)
            {
                matchesFound++;
            }
        }

        // Update if no matches were found
        if (!matchesFound)
        {
            StandardStreams::lock();
            StandardStreams::out << this->prefix << tr("Updating to %1.").arg(this->currentAddress.toString()) << endl;
            StandardStreams::unlock();

            this->download = new FileDownloader(QUrl(this->updateURL.arg(this->currentAddress.toString())));
            connect(this->download, &FileDownloader::downloaded, this, &Updater::onDNSUpdated);
        }
        else
        {
            this->updateInProgress.unlock();
        }
    }
    else
    {
        StandardStreams::lock();
        StandardStreams::err << this->prefix << tr("Error retrieving current IP.") << endl;
        StandardStreams::unlock();
        this->updateInProgress.unlock();
    }
}

void Updater::onDNSUpdated(QNetworkReply::NetworkError error)
{
    disconnect(this->download, &FileDownloader::downloaded, this, &Updater::onDNSUpdated);
    this->download->deleteLater();

    if (error == QNetworkReply::NoError)
    {
            StandardStreams::lock();
            StandardStreams::out << this->prefix << tr("Update finished.") << endl;
            StandardStreams::unlock();
    }
    else
    {
        StandardStreams::lock();
        StandardStreams::err << this->prefix << tr("Error updating DNS entry.") << endl;
        StandardStreams::unlock();
    };
    this->updateInProgress.unlock();
}

QList<QHostAddress> Updater::getDNSAddresses()
{
    QList<QHostAddress> addresses;
    QHostInfo info = QHostInfo::fromName(this->domain);

    if (this->valid)
    {
        for (const QHostAddress addr : info.addresses())
        {
            if (addr.protocol() == this->protocol)
            {
                addresses << addr;
            }
        }
    }

    return addresses;
}
