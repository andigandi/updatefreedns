#include "filedownloader.h"
#include <QNetworkRequest>

FileDownloader::FileDownloader(QUrl url, QObject *parent) :
    QObject(parent)
{
    connect(&(this->networkAccessManager), &QNetworkAccessManager::finished, this, &FileDownloader::onFileDownloaded);

    QNetworkRequest request(url);
    networkAccessManager.get(request);
}

FileDownloader::~FileDownloader()
{

}

void FileDownloader::onFileDownloaded(QNetworkReply* reply)
{
    this->downloadedData = reply->readAll();
    reply->deleteLater();
    emit downloaded(reply->error());
}

QByteArray FileDownloader::getDownloadedData() const
{
    return this->downloadedData;
}
