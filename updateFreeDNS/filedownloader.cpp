#include "filedownloader.h"
#include <QNetworkRequest>

FileDownloader::FileDownloader(QUrl url, const bool verbose, QObject *parent) : QObject(parent),
    verbose(verbose)
{
    this->out = new Output(tr("File Downloader: %1").arg(url.toString()), this);

    connect(&(this->networkAccessManager), &QNetworkAccessManager::finished, this, &FileDownloader::onFileDownloaded);

    QNetworkRequest request(url);
    networkAccessManager.get(request);

    if (this->verbose) this->out->writeOut(tr("Download started."));
}

FileDownloader::~FileDownloader()
{

}

void FileDownloader::onFileDownloaded(QNetworkReply* reply)
{
    this->downloadedData = reply->readAll();
    reply->deleteLater();

    if (this->verbose)
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            this->out->writeOut(tr("Download finished."));
        }
        else
        {
            this->out->writeOut(tr("Error downloading."));
        }
    }

    emit downloaded(reply->error());
}

QByteArray FileDownloader::getDownloadedData() const
{
    return this->downloadedData;
}
