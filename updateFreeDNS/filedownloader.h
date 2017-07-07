#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class FileDownloader : public QObject
{
    Q_OBJECT
    public:
        explicit FileDownloader(QUrl url, QObject *parent = 0);
        virtual ~FileDownloader();
        QByteArray getDownloadedData() const;

    signals:
        void downloaded(QNetworkReply::NetworkError error);

    private slots:
        void onFileDownloaded(QNetworkReply* reply);

    private:
        QNetworkAccessManager networkAccessManager;
        QByteArray downloadedData;
};

#endif // FILEDOWNLOADER_H
