#include "updater.h"
#include <QHostInfo>
#include <QMetaEnum>
#include "filedownloader.h"

Updater::Updater(const QString name, const bool enabled, const bool verbose, const QString protocol, const QString updateURL, const QString addressSource, const QString domain) :
    name{name},
    enabled{enabled},
    verbose{verbose},
    updateURL{updateURL},
    addressSource{addressSource},
    domain{domain},
    currentAddress{},
    lastAddress{}
{
    this->out = new Output(tr("Updater %1").arg(this->name), this);
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
        this->out->writeErr(tr("Invalid Protocol."));
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
        this->out->writeErr(tr("Invalid UpdateURL."));
    }

    // Address Source
    this->addressSource = QUrl(addressSource, QUrl::StrictMode);
    if (!this->addressSource.isValid())
    {
        this->valid = false;
        this->out->writeErr(tr("Invalid Source. %1").arg(this->addressSource.errorString()));
    }

    // Domain
    if (this->domain.count() < 1)
    {
        this->valid = false;
        this->out->writeErr(tr("Invalid Domain."));
    }

    if (this->valid)
    {
        this->out->writeOut(tr("Successfully initialized."));
    }
    else
    {
        this->out->writeErr(tr("Marked as invalid."));
    }

    if (!this->enabled)
    {
        this->out->writeOut(tr("Disabling."));
    }
}

void Updater::update()
{
    if (this->valid && this->enabled)
    {
        if (this->updateInProgress.tryLock())
        {
            this->download = new FileDownloader(this->addressSource, this->verbose);
            connect(this->download, &FileDownloader::downloaded, this, &Updater::onCurrentAddressDownloaded);
        }
        else
        {
            this->out->writeErr(tr("Update already in progress, skipping. Consider checking the interval settings."));
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

        if (this->verbose) this->out->writeOut(tr("Current address is %1.").arg(this->currentAddress.toString()));

        // Count the matches between current address and addresses found in DNS
        for (const QHostAddress addr : dnsAddresses)
        {
            if (addr == this->currentAddress)
            {
                if (this->verbose) this->out->writeOut(tr("Current address matches DNS entry %1.").arg(addr.toString()));
                matchesFound++;
            }
        }

        // Update if no matches were found
        if (!matchesFound)
        {
            if (this->currentAddress != this->lastAddress)
            {
                if (this->protocol == this->currentAddress.protocol())
                {
                    this->out->writeOut(tr("Updating to %1.").arg(this->currentAddress.toString()));

                    this->download = new FileDownloader(QUrl(this->updateURL.arg(this->currentAddress.toString())), this->verbose);
                    connect(this->download, &FileDownloader::downloaded, this, &Updater::onDNSUpdated);
                }
                else
                {
                    this->out->writeOut(tr("Mismatch between address %1 and desired protocol.").arg(this->currentAddress.toString()));
                    this->updateInProgress.unlock();
                }
            }
            else
            {
                this->out->writeOut(tr("Not updating, address did not change (%1).").arg(this->currentAddress.toString()));
                this->updateInProgress.unlock();
            }
        }
        else
        {
            this->updateInProgress.unlock();
        }
    }
    else
    {
        this->out->writeErr(tr("Error retrieving current IP."));
        this->updateInProgress.unlock();
    }
}

void Updater::onDNSUpdated(QNetworkReply::NetworkError error)
{
    disconnect(this->download, &FileDownloader::downloaded, this, &Updater::onDNSUpdated);
    this->download->deleteLater();

    if (error == QNetworkReply::NoError)
    {
        this->lastAddress = this->currentAddress;
        this->out->writeOut(tr("Update finished."));
    }
    else
    {
        this->out->writeErr(tr("Error updating DNS entry."));
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
                if (this->verbose) this->out->writeOut(tr("Found DNS entry %1.").arg(addr.toString()));
                addresses << addr;
            }
        }
    }

    return addresses;
}
