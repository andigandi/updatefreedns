#ifndef MANAGER_H
#define MANAGER_H

#include <QObject>
#include <QSettings>
#include <QTimer>
#include "updater.h"

class Manager : public QObject
{
    Q_OBJECT
public:
    explicit Manager(QObject *parent = nullptr);
    ~Manager();

    void start();

signals:

public slots:
    void updateTimerInterval();

private:
    Output *out;
    QSettings *settings;
    QTimer timer;

    QList<Updater*> updaters;

    static const QString fileName;
    static const QString directory;

    static const QString name;

    static const int keyTimerDefault = 30;
    static const QString keyTimer;

    static const int keyTimerFirstDefault = 2;
    static const QString keyTimerFirst;


    static const QString updaterKeyEnabled;
    static const bool updaterKeyEnabledDefault = false;
    static const QString updaterKeyVerbose;
    static const bool updaterKeyVerboseDefault = false;
    static const QString updaterKeyProtocol;
    static const QString updaterKeyUpdateURL;
    static const QString updaterKeyAddressSource;
    static const QString updaterKeyDomain;

    int timerIntervalMinutes;
    int timerIntervalMinutesFirst;
};

#endif // MANAGER_H
