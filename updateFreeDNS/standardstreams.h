#ifndef STANDARDSTREAMS_H
#define STANDARDSTREAMS_H

#include <QObject>
#include <QMutex>
#include <QTextStream>

class StandardStreams : public QObject
{
    Q_OBJECT
public:
    static void lock();
    static void unlock();

    static QTextStream out;
    static QTextStream err;

private:
    explicit StandardStreams(QObject *parent = nullptr);
    static QMutex mutex;
};

#endif // STANDARDSTREAMS_H
