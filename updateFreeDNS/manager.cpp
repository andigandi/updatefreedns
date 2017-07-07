#include "manager.h"

#include "standardstreams.h"
#include <QDir>

const QString Manager::prefix = QStringLiteral("Manager: ");
const QString Manager::name = QStringLiteral(TARGET);

const QString Manager::fileName = QString(Manager::name + QStringLiteral(".conf"));
const QString Manager::directory = QStringLiteral("/etc/");

const QString Manager::keyTimer = QStringLiteral("IntervalMinutes");
const QString Manager::keyTimerFirst = QStringLiteral("IntervalMinutesFirst");

const QString Manager::updaterKeyEnabled = QStringLiteral("Enabled");
const QString Manager::updaterKeyProtocol = QStringLiteral("Protocol");
const QString Manager::updaterKeyUpdateURL = QStringLiteral("UpdateURL");
const QString Manager::updaterKeyAddressSource = QStringLiteral("AddressSource");
const QString Manager::updaterKeyDomain = QStringLiteral("Domain");

Manager::Manager(QObject *parent) : QObject(parent)
{
    bool ok = false;
    //this->settings = new QSettings(QSettings::SystemScope, Settings::name); // This leads to ugly /etc/xdg/*.conf path...
    this->settings = new QSettings(QDir(Manager::directory).filePath(Manager::fileName), QSettings::IniFormat, this); // TODO: Support other OSes
    this->settings->setFallbacksEnabled(false); // No fallbacks, we want the specified file (might be unneccessary but does not hurt)

    StandardStreams::lock();
    StandardStreams::out << Manager::prefix << tr("Configuration file: %1").arg(this->settings->fileName()) << endl;
    StandardStreams::unlock();

    this->timerIntervalMinutes = this->settings->value(Manager::keyTimer, Manager::keyTimerDefault).toInt(&ok);
    StandardStreams::lock();
    if ((!ok) || (this->timerIntervalMinutes <= 0) || (this->timerIntervalMinutes > 1440)) // Check boundaries. 1440 minutes is arbitrarily chosen as maximum.
    {
        this->timerIntervalMinutes = Manager::keyTimerDefault;
        StandardStreams::err << Manager::prefix << tr("Invalid timer interval. Using default of %1 minutes.").arg(Manager::keyTimerDefault) << endl;
    }
    else
    {
        StandardStreams::out << Manager::prefix << tr("Timer interval: %1 minutes.").arg(this->timerIntervalMinutes) << endl;
    }
    StandardStreams::unlock();

    this->timerIntervalMinutesFirst = this->settings->value(Manager::keyTimerFirst, Manager::keyTimerFirstDefault).toInt(&ok);
    StandardStreams::lock();
    if ((!ok) || (this->timerIntervalMinutesFirst < 0) || (this->timerIntervalMinutesFirst > 1440)) // Check boundaries. 1440 minutes is arbitrarily chosen as maximum.
    {
        this->timerIntervalMinutes = Manager::keyTimerFirstDefault;
        StandardStreams::err << Manager::prefix << tr("Invalid first timer interval. Using default of %1 minutes.").arg(Manager::keyTimerFirstDefault) << endl;
    }
    else
    {
        StandardStreams::out << Manager::prefix << tr("First timer interval: %1 minutes.").arg(this->timerIntervalMinutesFirst) << endl;
    }
    StandardStreams::unlock();
    this->timer.setInterval(this->timerIntervalMinutesFirst * 60 * 1000);

    // Iterate through groups to create updaters
    for (const QString &updaterName : settings->childGroups())
    {
        Updater* updater = nullptr;
        StandardStreams::lock();
        StandardStreams::out << Manager::prefix << tr("Found updater definition: %1").arg(updaterName) << endl;
        StandardStreams::unlock();

        // Read settings
        this->settings->beginGroup(updaterName);
        const bool updaterEnabled = this->settings->value(Manager::updaterKeyEnabled, Manager::updaterKeyEnabledDefault).toBool();
        const QString updaterProtocol = this->settings->value(Manager::updaterKeyProtocol).toString();
        const QString updaterUpdateURL = this->settings->value(Manager::updaterKeyUpdateURL).toString();
        const QString updaterAddressSource = this->settings->value(Manager::updaterKeyAddressSource).toString();
        const QString updaterDomain = this->settings->value(Manager::updaterKeyDomain).toString();
        this->settings->endGroup();

        // Create Updater instance
        updater = new Updater(updaterName, updaterEnabled, updaterProtocol, updaterUpdateURL, updaterAddressSource, updaterDomain);
        connect(&(this->timer), &QTimer::timeout, updater, &Updater::update);
        this->updaters << updater;
    }

    connect(&(this->timer), &QTimer::timeout, this, &Manager::updateTimerInterval);
}

Manager::~Manager()
{
    delete this->settings;

    for (Updater* updater : this->updaters)
    {
        if (updater != nullptr)
        {
            updater->deleteLater();
        }
    }
}

void Manager::start()
{
    this->timer.start();
}

void Manager::updateTimerInterval()
{
    this->timer.setInterval(this->timerIntervalMinutes * 60 * 1000);

    StandardStreams::lock();
    StandardStreams::out << Manager::prefix << tr("Interval is now %1 minutes.").arg(this->timerIntervalMinutes) << endl;
    StandardStreams::unlock();

    disconnect(&(this->timer), &QTimer::timeout, this, &Manager::updateTimerInterval);
}
